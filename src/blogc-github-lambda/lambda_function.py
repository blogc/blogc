# coding: utf-8
"""
  blogc: A blog compiler.
  Copyright (C) 2016 Rafael G. Martins <rafael@rafaelmartins.eng.br>

  This program can be distributed under the terms of the BSD License.
  See the file LICENSE.
"""

from contextlib import closing
from StringIO import StringIO

import base64
import boto3
import hashlib
import json
import mimetypes
import os
import subprocess
import sys
import tarfile
import urllib2
import shutil

cwd = os.path.dirname(os.path.abspath(__file__))

GITHUB_AUTH = os.environ.get('GITHUB_AUTH')
if GITHUB_AUTH is not None and ':' not in GITHUB_AUTH:
    GITHUB_AUTH = boto3.client('kms').decrypt(
        CiphertextBlob=base64.b64decode(GITHUB_AUTH))['Plaintext']


def get_tarball(repo_name):
    tarball_url = 'https://api.github.com/repos/%s/tarball/master' % repo_name
    request = urllib2.Request(tarball_url)

    if GITHUB_AUTH is not None:
        auth = base64.b64encode(GITHUB_AUTH)
        request.add_header("Authorization", "Basic %s" % auth)

    with closing(urllib2.urlopen(request)) as fp:
        tarball = fp.read()

    rootdir = None
    with closing(StringIO(tarball)) as fp:
        with tarfile.open(fileobj=fp, mode='r:gz') as tar:
            for f in tar.getnames():
                if '/' not in f:
                    rootdir = f
                    break
            if rootdir is None:
                raise RuntimeError('Failed to find a directory in tarball')
            rootdir = '/tmp/%s' % rootdir

            if os.path.isdir(rootdir):
                shutil.rmtree(rootdir)

            tar.extractall('/tmp/')

    return rootdir


def sync_s3(src, dest):
    s3 = boto3.resource('s3')
    bucket = s3.Bucket(dest)

    remote_files = {}
    for obj in bucket.objects.all():
        if not obj.key.endswith('/'):
            remote_files[obj.key] = obj

    local_files = []
    for root, dirs, files in os.walk(src):
        real_root = root[len(src):].lstrip('/')
        for file in files:
            local_files.append(os.path.join(real_root, file))

    to_upload = []
    for file in local_files:
        if file not in remote_files:
            to_upload.append(file)

    to_delete = []
    for file in remote_files:
        if file in local_files:
            with open(os.path.join(src, file)) as fp:
                l = hashlib.sha1(fp.read())

            with closing(remote_files[file].get()['Body']) as fp:
                r = hashlib.sha1(fp.read())

            if l.hexdigest() != r.hexdigest():
                to_upload.append(file)
        else:
            to_delete.append(file)

    for file in to_upload:
        with open(os.path.join(src, file), 'rb') as fp:
            print 'Uploading file:', file
            mime, _ = mimetypes.guess_type(file)
            if mime is not None:
                bucket.put_object(Key=file, Body=fp, ContentType=mime)
            else:
                bucket.put_object(Key=file, Body=fp)

    for file in to_delete:
        print 'Deleting file:', file
        remote_files[file].delete()


def lambda_handler(event, context):
    message = event['Records'][0]['Sns']['Message']
    payload = json.loads(message)

    debug = 'DEBUG' in os.environ

    if payload['ref'] == 'refs/heads/master':
        repo_name = payload['repository']['name']
        repo_full_name = payload['repository']['full_name']
        rootdir = get_tarball(repo_full_name)
        rv = subprocess.call(['make', '-C', rootdir, 'OUTPUT_DIR=_build',
                              'BLOGC=%s' % os.path.join(cwd, 'blogc')],
                             stdout=None if debug else subprocess.PIPE,
                             stderr=None if debug else subprocess.PIPE)
        if rv != 0:
            raise RuntimeError('Failed to run make')
        sync_s3(os.path.join(rootdir, '_build'), repo_name)
