#!/bin/bash

# This script builds windows binaries, given a source tarball.
# It was designed to work on Fedora, and requires the following packages:
#
# mingw32-gcc mingw64-gcc zip
#
# This script must be called with the xz source tarball and the target version
# as arguments.


set -ex

[[ $# -eq 2 ]]


build() {
    local version=${2}
    local arch=${3}
    local build_dir="/tmp/blogc_build_${version}_${arch}"
    local dest_dir="/tmp/blogc-${version}-w${arch}"

    rm -rf "${build_dir}"
    mkdir -p "${build_dir}"
    tar -xvf "${1}" -C "${build_dir}"

    pushd "${build_dir}/blogc-${version}" &> /dev/null
    "mingw${arch}-configure"
    make
    popd &> /dev/null

    rm -rf "${dest_dir}"
    mkdir -p "${dest_dir}"
    cp "${build_dir}/blogc-${version}/.libs/blogc.exe" "${dest_dir}/"
    cp "${build_dir}/blogc-${version}/LICENSE" "${dest_dir}/"
    cp "${build_dir}/blogc-${version}/README.md" "${dest_dir}/"

    pushd "$(dirname ${dest_dir})" &> /dev/null
    zip "$(basename ${dest_dir}).zip" "$(basename ${dest_dir})"/*
    popd &> /dev/null

    mv "${dest_dir}.zip" .
}


for arch in 32 64; do
    build "$1" "$2" "${arch}"
done
