#!/bin/bash

set -ex

if [[ "x${TARGET}" = "xblogc-github-lambda" ]]; then
    build-aux/travis-build-github-lambda.sh
    exit $?
fi

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

make -C build "${TARGET}"
