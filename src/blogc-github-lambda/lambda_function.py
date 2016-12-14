# coding: utf-8
#
# blogc: A blog compiler.
# Copyright (C) 2016 Rafael G. Martins <rafael@rafaelmartins.eng.br>
#
# This program can be distributed under the terms of the BSD License.
# See the license for details.
#

from contextlib import closing
from StringIO import StringIO

import base64
import boto3
import hashlib
import json
import mimetypes
import os
import subprocess
import tarfile
import urllib2
import shutil

cwd = os.path.dirname(os.path.abspath(__file__))
bindir = os.path.join(cwd, 'bin')

os.environ['PATH'] = '%s:%s' % (bindir, os.environ.get('PATH', ''))

s3 = boto3.resource('s3')

GITHUB_AUTH = os.environ.get('GITHUB_AUTH')
if GITHUB_AUTH is not None and ':' not in GITHUB_AUTH:
    GITHUB_AUTH = boto3.client('kms').decrypt(
        CiphertextBlob=base64.b64decode(GITHUB_AUTH))['Plaintext']

# this is just a safeguard, just in case lambda stops supporting symlinks
# in zip files
for binary in subprocess.check_output([os.path.join(bindir, 'busybox'),
                                       '--list']).split():
    dst = os.path.join(bindir, binary)
    if not os.path.islink(dst):
        os.symlink('busybox', dst)
    else:
        break  # if one symlink exists, all the others will likely exist


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


def translate_filename(filename):
    f = filename.split('/')
    if len(f) == 0:
        return filename
    basename = f[-1]

    # replace any index.$EXT file with index.html, because s3 only allows
    # users to declare one directory index file name.
    p = basename.split('.')
    if len(p) == 2 and p[0] == 'index':
        f[-1] = 'index.html'
        return '/'.join(f)

    return filename


def sync_s3(src, dest, settings_file):
    if os.path.exists(settings_file):
        with open(settings_file, 'r') as fp:
            settings = json.load(fp)

    content_types = settings.get('content-type', {})
    dest = settings.get('bucket', dest)

    bucket = s3.Bucket(dest)

    remote_files = {}
    for obj in bucket.objects.all():
        if not obj.key.endswith('/'):
            remote_files[obj.key] = obj

    local_files = {}
    for root, dirs, files in os.walk(src):
        real_root = root[len(src):].lstrip('/')
        for file in files:
            f = os.path.join(real_root, file)
            local_files[translate_filename(f)] = f

    to_upload = []
    for file in local_files:
        if file not in remote_files:
            to_upload.append(local_files[file])

    to_delete = []
    for file in remote_files:
        if file in local_files:
            with open(os.path.join(src, local_files[file])) as fp:
                l = hashlib.sha1(fp.read())

            with closing(remote_files[file].get()['Body']) as fp:
                r = hashlib.sha1(fp.read())

            if l.hexdigest() != r.hexdigest():
                to_upload.append(local_files[file])
        else:
            to_delete.append(file)

    for file in to_upload:
        with open(os.path.join(src, file), 'rb') as fp:
            mime = content_types.get(file, mimetypes.guess_type(file)[0])
            file = translate_filename(file)
            print 'Uploading file: %s; content-type: "%s"' % (file, mime)
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

    if payload['ref'] == 'refs/heads/master':
        debug = 'DEBUG' in os.environ
        stream = None if debug else subprocess.PIPE

        rootdir = get_tarball(payload['repository']['full_name'])
        rv = subprocess.call([os.path.join(bindir, 'make'), '-C', rootdir,
                              'BLOGC=%s' % os.path.join(bindir, 'blogc'),
                              'OUTPUT_DIR=_build'],
                             stdout=stream, stderr=stream)
        if rv != 0:
            raise RuntimeError('Failed to run make')

        sync_s3(os.path.join(rootdir, '_build'),
                payload['repository']['name'],
                os.path.join(rootdir, 's3.json'))
