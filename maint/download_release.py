#!/usr/bin/env python2
# coding: utf-8

import os
import re
import requests
import subprocess
import sys

re_distfile = re.compile(r'(blogc[^\'"]+)\.sha512')
base_url = 'https://distfiles.rgm.io/blogc'
cwd = os.path.dirname(os.path.abspath(__file__))


def download_release(version):
    release_url = '%s/blogc-%s' % (base_url, version)
    r = requests.get('%s/' % release_url)
    r.raise_for_status()

    for distfile in set(re_distfile.findall(r.content)):
        file_url = '%s/%s' % (release_url, distfile)
        dest_path = os.path.join(cwd, 'releases', version)
        subprocess.check_call(['wget', '-c', '-P', dest_path, file_url,
                               '%s.sha512' % file_url])
        subprocess.check_call(['sha512sum', '-c', '%s.sha512' % distfile],
                              cwd=dest_path)


if __name__ == '__main__':
    download_release(sys.argv[1])
