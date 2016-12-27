#!/bin/bash

set -ex

MAKE_CONFIGURE="--enable-make"
if [[ "x${TARGET}" = "xblogc-make-embedded" ]]; then
    MAKE_CONFIGURE="--enable-make-embedded"
    TARGET="check"
fi
if [[ "x${TARGET}" = "xblogc-github-lambda" ]]; then
    MAKE_CONFIGURE="--enable-make-embedded"
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

if [[ "x${TARGET}" = "xblogc-github-lambda" ]]; then
    make -C build LDFLAGS="-all-static" blogc

    rm -rf root
    mkdir -p root

    PV="$(grep PACKAGE_VERSION build/config.h | cut -d\" -f2)"

    install -m 755 build/blogc root/blogc
    install -m 644 src/blogc-github-lambda/lambda_function.py root/lambda_function.py
    install -m 644 LICENSE root/LICENSE
    strip root/blogc

    pushd root/ > /dev/null
    zip --symlinks -rq "../blogc-github-lambda-${PV}.zip" *
    popd > /dev/null
else
    make -C build "${TARGET}"
fi
