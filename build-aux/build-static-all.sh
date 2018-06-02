#!/bin/bash

set -ex

DESTDIR="${PN}-${TARGET}-amd64-${PV}"

${MAKE_CMD:-make} LDFLAGS="-all-static"
${MAKE_CMD:-make} DESTDIR="${PWD}/root/${DESTDIR}" install
install -m 644 ../LICENSE "root/${DESTDIR}/LICENSE"

pushd root > /dev/null
tar -cvJf "../${DESTDIR}.tar.xz" "${DESTDIR}"
popd > /dev/null
