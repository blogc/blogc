build() {
    default_configure \
        CC= \
        CFLAGS="-O2 -g -pipe -Wall -Wp,-D_FORTIFY_SOURCE=2 -fexceptions --param=ssp-buffer-size=4" \
        --host=i686-w64-mingw32 \
        --target=i686-w64-mingw32 \
        --disable-tests \
        --disable-git-receiver \
        --disable-make \
        --disable-runserver
    make blogc.exe

    PN="$(grep PACKAGE_TARNAME config.h | cut -d\" -f2)"
    PV="$(grep PACKAGE_VERSION config.h | cut -d\" -f2)"
    DEST_DIR="${PN}-${PV}-${TARGET}"

    rm -rf "${DEST_DIR}"
    mkdir -p "${DEST_DIR}"

    cp .libs/blogc.exe "${DEST_DIR}/"
    cp ../LICENSE "${DEST_DIR}/"
    cp ../README.md "${DEST_DIR}/"

    zip "${DEST_DIR}.zip" "${DEST_DIR}"/*
}

deploy() {
    FILES=( *.zip )
    [[ ${RV} -eq 0 ]] && [[ "x${CC}" = "xgcc" ]]
}
