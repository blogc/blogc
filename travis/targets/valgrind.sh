build() {
    pushd build > /dev/null
    ../configure \
        CFLAGS="-Wall -g -O0" \
        --disable-silent-rules \
        --enable-ronn \
        --enable-tests \
        --enable-valgrind \
        --enable-git-receiver \
        --enable-make \
        --enable-runserver
    popd > /dev/null

    make -C build valgrind
}
