build() {
    pushd build > /dev/null
    ../configure \
        CFLAGS="-Wall -g -O0" \
        --disable-silent-rules \
        --enable-ronn \
        --enable-tests \
        --enable-git-receiver \
        --enable-make \
        --enable-runserver
    popd > /dev/null

    make -C build distcheck
}

deploy() {
    FILES=( build/*.{*.tar.{gz,bz2,xz},zip} )
}
