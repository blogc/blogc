#!/bin/bash

set -ex

DESTDIR="${PN}-${TARGET}-${PV}"

${MAKE_CMD:-make} blogc.exe blogc-runserver.exe

rm -rf "${DESTDIR}"
mkdir -p "${DESTDIR}"

cp .libs/blogc.exe .libs/blogc-runserver.exe "${DESTDIR}/"
cp ../LICENSE "${DESTDIR}/"
cp ../README.md "${DESTDIR}/"

zip "${DESTDIR}.zip" "${DESTDIR}"/*
