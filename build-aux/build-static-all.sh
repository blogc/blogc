#!/bin/bash

set -ex

${MAKE_CMD:-make} LDFLAGS="-all-static"
${MAKE_CMD:-make} DESTDIR="${PWD}/root" install
install -m 644 ../LICENSE root/LICENSE
echo "${PV}" > root/VERSION

pushd root > /dev/null
tar -cvJf "../${PN}-${TARGET}-amd64-${PV}.tar.xz" *
popd > /dev/null
