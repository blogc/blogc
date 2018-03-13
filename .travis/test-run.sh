#!/bin/bash

set -e

REALPATH=grealpath
if ! command -v grealpath >/dev/null 2>&1; then
    REALPATH=realpath
fi

SCRIPT_DIR="$(dirname "$("${REALPATH}" "${BASH_SOURCE[0]}")")"
TARGET_SCRIPT="${SCRIPT_DIR}/targets/${TARGET}.sh"

if [[ -n "${TARGET}" ]] && [[ -e "${TARGET_SCRIPT}" ]]; then
    source "${TARGET_SCRIPT}"
else
    echo "Target not defined or invalid!"
    exit 1
fi

set -x

BUILD=0
[[ "x$(type -t build)" = "xfunction" ]] && BUILD=1

DEPLOY=0
[[ "x$(type -t deploy)" = "xfunction" ]] && DEPLOY=1

EXTRACT=0
[[ "x$(type -t extract)" = "xfunction" ]] && EXTRACT=1

SOURCE_DIR="$(dirname "${SCRIPT_DIR}")"
BUILD_DIR="${SOURCE_DIR}/build"

default_configure() {
    pushd "${BUILD_DIR}" > /dev/null
    "${SOURCE_DIR}/configure" \
        CFLAGS="-Wall -g -O0" \
        --disable-silent-rules \
        --disable-valgrind \
        --enable-ronn \
        --enable-tests \
        --enable-git-receiver \
        --enable-make \
        --enable-runserver \
        "$@"
    popd > /dev/null
}

rm -rf "${BUILD_DIR}"
mkdir -p "${BUILD_DIR}"

RV=0

if [[ ${BUILD} -eq 1 ]]; then
    pushd "${BUILD_DIR}" > /dev/null
    build || RV=1
    popd > /dev/null
fi

[[ ${DEPLOY} -eq 1 ]] || exit ${RV}

[[ "x${TRAVIS_PULL_REQUEST}" != "xfalse" ]] && exit ${RV}
[[ "x${TRAVIS_BRANCH}" != "xmaster" ]] && [[ "x${TRAVIS_TAG}" != xv* ]] && exit ${RV}

[[ -d "${BUILD_DIR}" ]] || exit ${RV}

FILES=
pushd "${BUILD_DIR}" > /dev/null
deploy || exit ${RV}
popd > /dev/null

set +x

PN="$(grep PACKAGE_TARNAME "${BUILD_DIR}/config.h" | cut -d\" -f2)"
PV="$(grep PACKAGE_VERSION "${BUILD_DIR}/config.h" | cut -d\" -f2)"

do_sha512() {
    pushd "$(dirname ${1})" > /dev/null
    sha512sum "$(basename ${1})"
    popd > /dev/null
}

do_extract_flag() {
    [[ ${EXTRACT} -eq 0 ]] && echo false && return 0
    basename "${1}" | extract &> /dev/null && echo true || echo false
}

do_curl() {
    curl \
        --silent \
        --form "project=${PN}" \
        --form "version=${PV}" \
        --form "file=@${1}" \
        --form "sha512=$(do_sha512 ${1})" \
        --form "extract=$(do_extract_flag ${1})" \
        "${DISTFILES_URL}" \
        &> /dev/null  # make sure that we don't leak tokens
}

echo
echo " * Found files:"
for f in "${FILES[@]}"; do
    echo "   ${f}"
done
echo

for f in "${FILES[@]}"; do
    echo " * Processing file: $(basename ${f}):"

    echo -n "   Uploading file ... "
    if do_curl "${BUILD_DIR}/${f}"; then
        echo "done"
    else
        echo "fail"
    fi

    echo
done

exit ${RV}
