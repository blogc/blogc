build() {
    pushd build > /dev/null
    ../configure \
        CFLAGS="-Wall -g -O0" \
        --enable-ronn \
        --disable-silent-rules \
        --disable-tests \
        --disable-valgrind \
        --disable-git-receiver \
        --enable-make-embedded \
        --disable-runserver
    popd > /dev/null

    make -C build LDFLAGS="-all-static" blogc

    rm -rf build/root
    mkdir -p build/root

    PV="$(grep PACKAGE_VERSION build/config.h | cut -d\" -f2)"

    install -m 755 build/blogc build/root/blogc
    install -m 644 build/src/blogc-github-lambda/lambda_function.py build/root/lambda_function.py
    install -m 644 LICENSE build/root/LICENSE
    strip build/root/blogc

    pushd build/root/ > /dev/null
    zip "../blogc-github-lambda-${PV}.zip" *
    popd > /dev/null

    install -m 755 build/root/blogc "build/blogc-static-amd64-${PV}"
    xz -z "build/blogc-static-amd64-${PV}"
}

deploy() {
    FILES=( build/*.zip build/blogc-static-*.xz )
}
