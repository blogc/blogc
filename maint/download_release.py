#!/usr/bin/env python3
# coding: utf-8

import os
import re
import requests
import shutil
import subprocess
import sys

re_version = re.compile(r'blogc-([^\'"]+)\.tar\.gz')
re_distfile = re.compile(r'(blogc[^\'"]+)\.sha512')
base_url = 'https://distfiles.rgm.io/blogc'
cwd = os.path.dirname(os.path.abspath(__file__))
gpg_key = '0xE00C52C92FEBED9B'


def download_release(version):
    if version is None:
        release_url = '%s/LATEST' % (base_url,)
        r = requests.get(release_url)
        r.raise_for_status()
        match = re_version.search(r.text)
        if match is None:
            raise RuntimeError('Could not guess version')
        version = match.group(1)
    else:
        release_url = '%s/blogc-%s' % (base_url, version)
        r = requests.get(release_url)
        r.raise_for_status()

    dest_path = os.path.join(cwd, 'releases', version)
    if os.path.exists(dest_path):
        shutil.rmtree(dest_path)
    os.makedirs(dest_path)

    for distfile in set(re_distfile.findall(r.text)):
        file_url = '%s/%s' % (release_url, distfile)
        subprocess.check_call(['wget', '-P', dest_path, file_url,
                               '%s.sha512' % file_url])
        subprocess.check_call(['sha512sum', '-c', '%s.sha512' % distfile],
                              cwd=dest_path)
        subprocess.check_call(['gpg', '--local-user', gpg_key, '--detach-sign',
                               '--armor', distfile],
                              cwd=dest_path)


if __name__ == '__main__':
    download_release(sys.argv[1] if len(sys.argv) > 1 else None)
