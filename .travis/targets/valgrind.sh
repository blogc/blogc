build() {
    default_configure \
        --enable-valgrind
    make valgrind
}
