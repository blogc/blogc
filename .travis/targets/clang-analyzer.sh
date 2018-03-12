build() {
    pushd build > /dev/null
    ../configure \
        CFLAGS="-Wall -g -O0" \
        --enable-silent-rules \
        --enable-ronn \
        --enable-tests \
        --disable-valgrind \
        --enable-git-receiver \
        --enable-make \
        --enable-runserver
    popd > /dev/null

    PN="$(grep PACKAGE_TARNAME build/config.h | cut -d\" -f2)"
    PV="$(grep PACKAGE_VERSION build/config.h | cut -d\" -f2)"
    P="${PN}-clang-analyzer-${PV}"

    set +e
    scan-build \
        --status-bugs \
        -o "build/${P}" \
        make -C build
    RV=$?
    set -e

    if [[ ${RV} -ne 0 ]]; then
        tar \
            -cvJf "build/${P}.tar.xz" \
            -C build/${P}/ \
            .
    fi

    echo ${RV} > build/.test_result
}

deploy_cond() {
    [[ "x${CC}" = "xclang" ]] && [[ -f build/*.tar.xz ]]
}

deploy() {
    FILES=( build/*.tar.xz )
}
