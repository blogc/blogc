#!/bin/bash

set -ex

DESTDIR="${PN}-${TARGET}-${PV}"

${MAKE_CMD:-make}

rm -rf "${DESTDIR}"
mkdir -p "${DESTDIR}"

cp .libs/blogc.exe "${DESTDIR}/"
cp ../LICENSE "${DESTDIR}/"
cp ../README.md "${DESTDIR}/"

zip "${DESTDIR}.zip" "${DESTDIR}"/*
