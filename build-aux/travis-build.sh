#!/bin/bash

set -ex

if [[ "x${TARGET}" = "xblogc-github-lambda" ]]; then
    build-aux/travis-build-github-lambda.sh
    exit $?
fi

MAKE_CONFIGURE="--enable-make"
if [[ "x${TARGET}" = "xblogc-make-embedded" ]]; then
    MAKE_CONFIGURE="--enable-make-embedded"
    TARGET="check"
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
    --enable-runserver \
    ${MAKE_CONFIGURE}
popd > /dev/null

make -C build "${TARGET}"
