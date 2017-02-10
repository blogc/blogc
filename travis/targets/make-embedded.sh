build() {
    pushd build > /dev/null
    ../configure \
        CFLAGS="-Wall -g -O0" \
        --enable-ronn \
        --disable-silent-rules \
        --enable-tests \
        --enable-valgrind \
        --enable-git-receiver \
        --enable-make-embedded \
        --enable-runserver
    popd > /dev/null

    make -C build check
}
