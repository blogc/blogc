#!/bin/bash

set -exo pipefail

export DEBEMAIL="rafael+deb@rafaelmartins.eng.br"
export DEBFULLNAME="Automatic Builder (github-actions)"
export DEB_BUILD_OPTIONS="noddebs"

PBUILDER_CHROOTS_VERSION="202303052042"
PBUILDER_CHROOTS_BASE_URL="https://github.com/rafaelmartins/pbuilder-chroots/releases/download/pbuilder-chroots-amd64-${PBUILDER_CHROOTS_VERSION}"

MY_P="${PN}_${PV}"
DIST="$(echo "${TARGET}" | cut -d- -f2)"
ARCH="$(echo "${TARGET}" | cut -d- -f3)"
DISTRO="$(wget -q -O- "${PBUILDER_CHROOTS_BASE_URL}/DISTROS" | grep "${DIST}")"
DEB_VERSION="$(echo "${DISTRO}" | cut -d_ -f3)"
REV="1~"
if [[ x${DEB_VERSION} != x ]]; then
    REV="${REV}${DEB_VERSION}"
fi
REV="${REV}${DIST}"

case ${DIST} in
    bookworm|sid|kinetic)
        DEB_BUILD_OPTIONS="${DEB_BUILD_OPTIONS} nocheck"
        ;;
esac

download_pbuilder_chroot() {
    pushd "${SRCDIR}" > /dev/null

    wget -c "${PBUILDER_CHROOTS_BASE_URL}/pbuilder-chroot-${DISTRO}-${ARCH}-${PBUILDER_CHROOTS_VERSION}.tar.xz"{,.sha512}
    sha512sum --check --status "pbuilder-chroot-${DISTRO}-${ARCH}-${PBUILDER_CHROOTS_VERSION}.tar.xz.sha512"

    sudo rm -rf /tmp/pbuilder
    mkdir -p "/tmp/pbuilder/${DIST}-${ARCH}"
    fakeroot tar --checkpoint=1000 -xf "pbuilder-chroot-${DISTRO}-${ARCH}-${PBUILDER_CHROOTS_VERSION}.tar.xz" -C "/tmp/pbuilder/${DIST}-${ARCH}"

    popd > /dev/null
}

download_orig() {
    local i=0
    local out=0
    local url="https://distfiles.rgm.io/${PN}/${P}/${P}.tar.xz"

    while [[ $i -lt 20 ]]; do
        set +ex
        ((i++))
        echo "waiting for ${P}.tar.xz: $i/20"
        wget -q --spider --tries 1 "${url}"
        out=$?
        set -ex

        if [[ $out -eq 0 ]]; then
            wget -c "${url}"
            mv "${P}.tar.xz" "${BUILDDIR}/${MY_P}.orig.tar.xz"
            return
        fi

        if [[ $out -ne 8 ]]; then
            exit $out
        fi

        sleep 30
    done

    echo "failed to find orig distfile. please check if that task succeeded."
    exit 1
}

create_reprepro_conf() {
    echo "Origin: blogc"
    echo "Label: blogc"
    echo "Codename: ${DIST}"
    echo "Architectures: source amd64"
    echo "Components: main"
    echo "Description: Apt repository containing blogc snapshots"
    echo
}

download_pbuilder_chroot
download_orig

rm -rf "${BUILDDIR}/${P}"
tar -xf "${BUILDDIR}/${MY_P}.orig.tar.xz" -C "${BUILDDIR}"
cp -r "${SRCDIR}/debian" "${BUILDDIR}/${P}/"

pushd "${BUILDDIR}/${P}" > /dev/null

## skip build silently when new version is older than last changelog version (version bump)
if ! dch \
    --distribution "${DIST}" \
    --newversion "${PV}-${REV}" \
    "Automated build for ${DIST}"
then
    exit 0
fi

sudo cowbuilder \
    --update \
    --basepath "/tmp/pbuilder/${DIST}-${ARCH}/base.cow"

RES="${BUILDDIR}/deb/${DIST}"
mkdir -p "${RES}"

pdebuild \
    --pbuilder cowbuilder \
    --buildresult "${RES}" \
    --debbuildopts -sa \
    -- --basepath "/tmp/pbuilder/${DIST}-${ARCH}/base.cow"

popd > /dev/null

mkdir -p "${BUILDDIR}/deb-repo/conf"
create_reprepro_conf > "${BUILDDIR}/deb-repo/conf/distributions"

pushd "${BUILDDIR}/deb-repo" > /dev/null

for i in "../deb/${DIST}"/*.changes; do
    reprepro include "${DIST}" "${i}"
done

popd > /dev/null

tar \
    -cJf "blogc-deb-repo-${DIST}-${ARCH}-${PV}.tar.xz" \
    --exclude ./deb-repo/conf \
    --exclude ./deb-repo/db \
    ./deb-repo

tar \
    -cJf "blogc-deb-${DIST}-${ARCH}-${PV}.tar.xz" \
    ./deb
