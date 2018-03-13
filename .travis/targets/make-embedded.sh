build() {
    default_configure \
        --enable-make-embedded
    make check
}
