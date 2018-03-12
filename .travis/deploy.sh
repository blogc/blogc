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

if [[ "x$(type -t deploy)" != "xfunction" ]] || [[ "x$(type -t deploy_cond)" != "xfunction" ]]; then
    echo "Nothing to deploy. skipping ..."
    exit 1
fi

if ! deploy_cond; then
    echo "Deploy disabled. skipping ..."
    exit 1
fi

deploy

TARNAME="$(grep PACKAGE_TARNAME build/config.h | cut -d\" -f2)"
VERSION="$(grep PACKAGE_VERSION build/config.h | cut -d\" -f2)"

do_sha512() {
    pushd "$(dirname ${1})" > /dev/null
    sha512sum "$(basename ${1})"
    popd > /dev/null
}

do_curl() {
    curl \
        --silent \
        --form "project=${TARNAME}" \
        --form "version=${VERSION}" \
        --form "file=@${1}" \
        --form "sha512=$(do_sha512 ${1})" \
        "${DISTFILES_URL}" \
        &> /dev/null  # make sure that we don't leak tokens
}

echo " * Found files:"
for f in "${FILES[@]}"; do
    echo "   $(basename ${f})"
done
echo

for f in "${FILES[@]}"; do
    echo " * Processing file: $(basename ${f}):"

    echo -n "   Uploading file ... "
    do_curl "${f}"
    echo "done"

    echo
done

# this is a hack to allow failed tests to upload files.
# the target must write status code to build/.test_result, instead of exiting directly
if [[ -f build/.test_result ]]; then
    exit $(cat build/.test_result)
fi
