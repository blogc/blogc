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

    local pn="$(grep PACKAGE_TARNAME config.h | cut -d\" -f2)"
    local pv="$(grep PACKAGE_VERSION config.h | cut -d\" -f2)"
    local dest_dir="${pn}-${pv}-${TARGET}"

    rm -rf "${dest_dir}"
    mkdir -p "${dest_dir}"

    cp .libs/blogc.exe "${dest_dir}/"
    cp ../LICENSE "${dest_dir}/"
    cp ../README.md "${dest_dir}/"

    zip "${dest_dir}.zip" "${dest_dir}"/*
}

deploy() {
    FILES=( *.zip )
    [[ ${RV} -eq 0 ]] && [[ "x${CC}" = "xgcc" ]]
}
