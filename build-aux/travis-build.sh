#!/bin/bash

set -ex

rm -rf build
mkdir -p build

MAKE_TARGET="${TARGET}"

if [[ "x${TARGET}" = xw* ]]; then
    unset CC
    export CFLAGS="-O2 -g -pipe -Wall -Wp,-D_FORTIFY_SOURCE=2 -fexceptions --param=ssp-buffer-size=4"
    export CHOST="x86_64-w64-mingw32"
    [[ "x${TARGET}" = "xw32" ]] && export CHOST="i686-w64-mingw32"
    MAKE_TARGET="all"
else
    export CFLAGS="-Wall -g"
    export CONFIGURE_ARGS="--enable-tests --enable-valgrind"
fi

pushd build > /dev/null

../configure \
    ${CHOST:+--host=${CHOST} --target=${CHOST}} \
    --enable-ronn \
    --with-squareball=internal \
    --disable-silent-rules \
    ${CONFIGURE_ARGS}

popd > /dev/null

make -C build "${MAKE_TARGET}"

if [[ "x${TARGET}" = xw* ]]; then
    TARNAME="$(grep PACKAGE_TARNAME build/config.h | cut -d\" -f2)"
    VERSION="$(grep PACKAGE_VERSION build/config.h | cut -d\" -f2)"
    DEST_DIR="${TARNAME}-${VERSION}-${TARGET}"

    rm -rf "${DEST_DIR}"
    mkdir -p "${DEST_DIR}"

    cp build/.libs/blogc.exe "${DEST_DIR}/"
    cp LICENSE "${DEST_DIR}/"
    cp README.md "${DEST_DIR}/"

    zip "build/${DEST_DIR}.zip" "${DEST_DIR}"/*
fi
