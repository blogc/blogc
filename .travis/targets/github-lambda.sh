build() {
    default_configure \
        CFLAGS="-Wall -g -O2" \
        --disable-tests \
        --disable-git-receiver \
        --disable-runserver \
        --enable-make-embedded

    make LDFLAGS="-all-static" blogc

    rm -rf root
    mkdir -p root

    PV="$(grep PACKAGE_VERSION config.h | cut -d\" -f2)"

    install -m 755 blogc root/blogc
    install -m 644 src/blogc-github-lambda/lambda_function.py root/lambda_function.py
    install -m 644 ../LICENSE root/LICENSE
    strip root/blogc

    pushd root > /dev/null
    zip "../blogc-github-lambda-${PV}.zip" *
    popd > /dev/null

    install -m 755 root/blogc "blogc-static-amd64-${PV}"
    xz -z "blogc-static-amd64-${PV}"
}

deploy() {
    FILES=( *.zip blogc-static-*.xz )
    [[ ${RV} -eq 0 ]] && [[ "x${CC}" = "xgcc" ]]
}
