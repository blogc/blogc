#!/bin/bash

set -ex

P="${PN}-clang-analyzer-${PV}"

set +e
scan-build \
    --use-cc="${CC:-clang}" \
    -o reports \
    make
RV=$?
set -e

NUM_REPORTS=$(ls -1 reports | wc -l)
[[ ${NUM_REPORTS} -eq 0 ]] && exit ${RV}
[[ ${NUM_REPORTS} -eq 1 ]]

REPORTS="reports/$(ls -1 reports)"
if [[ -d "${REPORTS}" ]]; then
    mv "${REPORTS}" clang-analyzer
    tar \
        -cvJf "${P}.tar.xz" \
        clang-analyzer
    RV=1
fi

exit ${RV}
