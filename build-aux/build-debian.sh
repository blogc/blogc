#!/bin/bash

set -exo pipefail

export DEBEMAIL="rafael+deb@rafaelmartins.eng.br"
export DEBFULLNAME="Automatic Builder (github-actions)"
export DEB_BUILD_OPTIONS="noddebs"

download_pbuilder_chroots() {
    local arch="amd64"
    local os="$(uname -s | tr '[:upper:]' '[:lower:]')"

    local index="$(wget -q -O- https://distfiles.rgm.io/pbuilder-chroots/LATEST/)"
    local archive="$(echo "${index}" | sed -n "s/.*\(pbuilder-chroots-${os}-${arch}-.*\)\.sha512.*/\1/p")"
    local folder="$(echo "${index}" | sed -n "s/.*\(pbuilder-chroots-${os}-${arch}-.*\)\.tar.*\.sha512.*/\1/p")"
    local p="$(echo "${folder}" | sed "s/pbuilder-chroots-${os}-${arch}-\(.*\)/pbuilder-chroots-\1/")"

    pushd "${SRCDIR}" > /dev/null

    wget -c "https://distfiles.rgm.io/pbuilder-chroots/${p}/${archive}"{,.sha512}
    sha512sum --check --status "${archive}.sha512"

    sudo rm -rf /tmp/pbuilder
    fakeroot tar --checkpoint=.1000 -xf "${archive}" -C /tmp

    popd > /dev/null
}

create_reprepro_conf() {
    for dist in "$@"; do
        echo "Origin: blogc"
        echo "Label: blogc"
        echo "Codename: ${dist}"
        echo "Architectures: source amd64"
        echo "Components: main"
        echo "Description: Apt repository containing blogc snapshots"
        echo
    done
}

download_pbuilder_chroots

${MAKE_CMD:-make} dist-xz

MY_P="${PN}_${PV}"

mv ${P}.tar.xz "${BUILDDIR}/${MY_P}.orig.tar.xz"

for dir in /tmp/pbuilder/*/base.cow; do
    export DIST="$(basename "$(dirname "${dir}")" | cut -d- -f1)"
    RES="${BUILDDIR}/deb/${DIST}"
    mkdir -p "${RES}"

    rm -rf "${BUILDDIR}/${P}"
    tar -xf "${BUILDDIR}/${MY_P}.orig.tar.xz" -C "${BUILDDIR}"
    cp -r "${SRCDIR}/debian" "${BUILDDIR}/${P}/"

    REV=
    case ${DIST} in
        buster)
            REV="1~10buster"
            ;;
        bullseye)
            REV="1~11bullseye"
            ;;
        sid)
            REV="1~sid"
            ;;
        focal)
            REV="1~11.0focal"
            ;;
        groovy)
            REV="1~11.1groovy"
            ;;
        *)
            echo "error: unsupported dist: ${DIST}"
            exit 1
            ;;
    esac

    pushd "${BUILDDIR}/${P}" > /dev/null

    dch \
        --distribution "${DIST}" \
        --newversion "${PV}-${REV}" \
        "Automated build for ${DIST}"

    pdebuild \
        --pbuilder cowbuilder \
        --buildresult "${RES}" \
        -- --basepath "${dir}"

    popd > /dev/null

done

DISTS="$(ls -1 "${BUILDDIR}/deb")"

mkdir -p "${BUILDDIR}/deb-repo/conf"
create_reprepro_conf ${DISTS} > "${BUILDDIR}/deb-repo/conf/distributions"

pushd "${BUILDDIR}/deb-repo" > /dev/null

for dist in ${DISTS}; do
    reprepro include "${dist}" "../deb/${dist}"/*.changes
done

popd > /dev/null

tar \
    -cJf "blogc-deb-repo-${PV}.tar.xz" \
    --exclude ./deb-repo/conf \
    --exclude ./deb-repo/db \
    ./deb-repo

tar \
    -cJf "blogc-deb-${PV}.tar.xz" \
    ./deb
