build() {
    unset CC
    pushd build > /dev/null
    ../configure \
        CFLAGS="-O2 -g -pipe -Wall -Wp,-D_FORTIFY_SOURCE=2 -fexceptions --param=ssp-buffer-size=4" \
        --host=i686-w64-mingw32 \
        --target=i686-w64-mingw32 \
        --enable-ronn \
        --disable-silent-rules \
        --disable-tests \
        --disable-valgrind \
        --disable-git-receiver \
        --disable-make-embedded \
        --disable-runserver
    popd > /dev/null

    make -C build blogc.exe

    PN="$(grep PACKAGE_TARNAME build/config.h | cut -d\" -f2)"
    PV="$(grep PACKAGE_VERSION build/config.h | cut -d\" -f2)"
    DEST_DIR="${PN}-${PV}-${TARGET}"

    rm -rf "${DEST_DIR}"
    mkdir -p "${DEST_DIR}"

    cp build/.libs/blogc.exe "${DEST_DIR}/"
    cp LICENSE "${DEST_DIR}/"
    cp README.md "${DEST_DIR}/"

    zip "build/${DEST_DIR}.zip" "${DEST_DIR}"/*
}

deploy_cond() {
    [[ "x${CC}" = "xgcc" ]]
}

deploy() {
    FILES=( build/*.zip )
}
