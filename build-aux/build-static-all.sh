#!/bin/bash

set -ex

DESTDIR="${PN}-${TARGET}-${PV}"

${MAKE_CMD:-make} LDFLAGS="-all-static"
${MAKE_CMD:-make} DESTDIR="${PWD}/root/${DESTDIR}" install
install -m 644 ../LICENSE "root/${DESTDIR}/LICENSE"

pushd root > /dev/null
tar -cvJf "../${DESTDIR}.tar.xz" .
popd > /dev/null
