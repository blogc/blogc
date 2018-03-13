build() {
    default_configure \
        --enable-silent-rules

    local pn="$(grep PACKAGE_TARNAME config.h | cut -d\" -f2)"
    local pv="$(grep PACKAGE_VERSION config.h | cut -d\" -f2)"
    local p="${pn}-clang-analyzer-${pv}"

    set +e
    scan-build \
        --use-cc="${CC}" \
        -o reports \
        make
    #src/blogc/libblogc_la-debug.lo
    local rv=$?
    set -e

    local num_reports=$(ls -1 reports | wc -l)
    [[ ${num_reports} -eq 0 ]] && return ${rv}
    [[ ${num_reports} -eq 1 ]]

    local reports="reports/$(ls -1 reports)"

    if [[ -d "${reports}" ]]; then
        mv "${reports}" clang-analyzer
        tar \
            -cvJf "${p}.tar.xz" \
            clang-analyzer
        rv=1
    fi

    return ${rv}
}

deploy() {
    FILES=( *.tar.xz )
    [[ ${RV} -ne 0 ]] && [[ "x${CC}" = "xclang" ]]
}

extract() {
    grep .
}
