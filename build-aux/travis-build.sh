#!/bin/bash

set -ex

rm -rf build
mkdir -p build

pushd build > /dev/null

export CFLAGS="-Wall -g -O0"

if [[ "x${TARGET}" = "xblogc-github-lambda" ]]; then
    export LDFLAGS="-static"
fi

../configure \
    --enable-ronn \
    --disable-silent-rules \
    --enable-tests \
    --enable-valgrind \
    --enable-git-receiver \
    --enable-runserver

popd > /dev/null

if [[ "x${TARGET}" = "xblogc-github-lambda" ]]; then
    export LDFLAGS="-all-static"
fi

make -C build "${TARGET}"
