#!/bin/bash

set -e

if [[ "x${TRAVIS_PULL_REQUEST}" != "xfalse" ]]; then
    echo "This is a pull request. skipping deploy ..."
    exit 0
fi

if [[ "x${TRAVIS_BRANCH}" != "xmaster" ]] && [[ "x${TRAVIS_TAG}" != xv* ]]; then
    echo "This isn't master branch nor a valid tag. skipping deploy ..."
    exit 0
fi

if [[ "x${CC}" != "xgcc" ]] || [[ "x${TARGET}" = "xvalgrind" ]] || [[ "x${TARGET}" = "xmake-embedded" ]]; then
    echo "Invalid target for deploy. skipping ..."
    exit 0
fi

if [[ ! -d build ]]; then
    echo "Build directory not found."
    exit 1
fi

FILES=
if [[ -n "${TARGET}" ]] && [[ -e ".travis/targets/${TARGET}.sh" ]]; then
    source ".travis/targets/${TARGET}.sh"
else
    echo "Target not defined or invalid!"
    exit 1
fi

deploy

TARNAME="$(grep PACKAGE_TARNAME build/config.h | cut -d\" -f2)"
VERSION="$(grep PACKAGE_VERSION build/config.h | cut -d\" -f2)"

do_curl() {
    curl \
        --silent \
        --ftp-create-dirs \
        --upload-file "${1}" \
        --user "${FTP_USER}:${FTP_PASSWORD}" \
        "ftp://${FTP_HOST}/public_html/${TARNAME}/${TARNAME}-${VERSION}/$(basename ${1})"
}

echo " * Found files:"
for f in "${FILES[@]}"; do
    echo "   $(basename ${f})"
done
echo

for f in "${FILES[@]}"; do
    echo " * Processing file: $(basename ${f}):"

    echo -n "   Generating SHA512 checksum ... "
    pushd "$(dirname ${f})" > /dev/null
    sha512sum "$(basename ${f})" > "$(basename ${f}).sha512"
    popd > /dev/null
    echo "done"

    echo -n "   Uploading file ... "
    do_curl "${f}"
    echo "done"

    echo -n "   Uploading SHA512 checksum ... "
    do_curl "${f}.sha512"
    echo "done"

    echo
done
