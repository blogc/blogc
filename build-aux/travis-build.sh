#!/bin/bash

set -ex

rm -rf build
mkdir -p build

pushd build > /dev/null

../configure \
    CFLAGS="-Wall -g -O0" \
    --enable-ronn \
    --disable-silent-rules \
    --enable-tests \
    --enable-valgrind \
    --enable-git-receiver \
    --enable-runserver

popd > /dev/null

if [[ "x${TARGET}" = "xblogc-github-lambda" ]]; then
    make -C build LDFLAGS="-all-static" "${TARGET}"
else
    make -C build "${TARGET}"
fi
