#!/bin/bash

set -ex

PV_DEPS=2

PN_DEPS="blogc-lambda-deps"
P_DEPS="${PN_DEPS}-${PV_DEPS}"
A_DEPS="${P_DEPS}.tar.xz"
SRC_DEPS="https://travis-distfiles.rgm.io/${PN_DEPS}/${P_DEPS}/${P_DEPS}.tar.xz"

rm -rf root build
mkdir -p root build

wget -c "${SRC_DEPS}" "${SRC_DEPS}.sha512"
sha512sum -c "${A_DEPS}.sha512"
tar -xvf "${A_DEPS}" -C root/

pushd build > /dev/null
../configure \
    CFLAGS="-Wall -g -O0" \
    --disable-ronn \
    --disable-silent-rules \
    --disable-tests \
    --disable-valgrind \
    --disable-git-receiver \
    --disable-runserver
popd > /dev/null

make -C build LDFLAGS="-all-static" blogc

PV="$(grep PACKAGE_VERSION build/config.h | cut -d\" -f2)"

install -m 755 build/blogc root/bin/blogc
install -m 644 src/blogc-github-lambda/lambda_function.py root/lambda_function.py
install -m 644 LICENSE root/licenses/blogc
strip root/bin/blogc

pushd root/ > /dev/null
zip -rq "../blogc-github-lambda-${PV}.zip" *
popd > /dev/null
