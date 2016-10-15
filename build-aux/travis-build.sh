#!/bin/bash

set -x

rm -rf build
mkdir -p build

git --no-pager log -1 --pretty=format:x . 2>&1
git describe --abbrev=4 --match="$prefix*" HEAD
git describe --abbrev=4 HEAD 2>/dev/null
git rev-list "$vtag"..HEAD 2>/dev/null
git --version >/dev/null 2>&1
build-aux/git-version-gen .tarball-version

pushd build > /dev/null

../configure \
    CFLAGS="-Wall -g" \
    --enable-ronn \
    --disable-silent-rules \
    --enable-tests \
    --enable-valgrind \
    --enable-git-receiver \
    --enable-runserver

popd > /dev/null

make -C build "${TARGET}"
