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

    make -C build LDFLAGS="-all-static" blogc

    rm -rf build/root
    mkdir -p build/root

    PV="$(grep PACKAGE_VERSION build/config.h | cut -d\" -f2)"

    install -m 755 build/blogc build/root/blogc
    install -m 644 src/blogc-github-lambda/lambda_function.py build/root/lambda_function.py
    install -m 644 LICENSE build/root/LICENSE
    strip build/root/blogc

    pushd build/root/ > /dev/null
    zip -rq "../blogc-github-lambda-${PV}.zip" *
    popd > /dev/null
}

deploy() {
    FILES=( build/*.zip )
}
