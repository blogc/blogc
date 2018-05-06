#!/bin/bash

set -ex

PN="$(grep PACKAGE_TARNAME config.h | cut -d\" -f2)"
PV="$(grep PACKAGE_VERSION config.h | cut -d\" -f2)"
DEST_DIR="${PN}-${TARGET}-${PV}"

${MAKE_CMD:-make} blogc.exe

rm -rf "${DEST_DIR}"
mkdir -p "${DEST_DIR}"

cp .libs/blogc.exe "${DEST_DIR}/"
cp ../LICENSE "${DEST_DIR}/"
cp ../README.md "${DEST_DIR}/"

zip "${DEST_DIR}.zip" "${DEST_DIR}"/*
