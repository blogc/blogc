build() {
    default_configure
    make dist-srpm
}

deploy() {
    FILES=( *.src.rpm )
    [[ ${RV} -eq 0 ]] && [[ "x${CC}" = "xgcc" ]]
}
