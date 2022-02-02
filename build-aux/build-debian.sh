#!/bin/bash

set -exo pipefail

export DEBEMAIL="rafael+deb@rafaelmartins.eng.br"
export DEBFULLNAME="Automatic Builder (github-actions)"
export DEB_BUILD_OPTIONS="noddebs"

export DIST="$(echo "${TARGET}" | cut -d- -f2)"

ARCH="$(echo "${TARGET}" | cut -d- -f3)"

REV=
case ${DIST} in
    bullseye)
        REV="1~11bullseye"
        ;;
    bookworm)
        REV="1~12bookworm"
        ;;
    sid)
        REV="1~sid"
        ;;
    focal)
        REV="1~11.0focal"
        ;;
    impish)
        REV="1~11.3impish"
        ;;
    *)
        echo "error: unsupported dist: ${DIST}"
        exit 1
        ;;
esac

download_pbuilder_chroot() {
    local index="$(wget -q -O- https://distfiles.rgm.io/pbuilder-chroots/LATEST/)"
    local archive="$(echo "${index}" | sed -n "s/.*\(pbuilder-chroot-${DIST}-${ARCH}-.*\)\.sha512.*/\1/p")"
    local p="$(echo "${index}" | sed -n "s/.*pbuilder-chroot-${DIST}-${ARCH}-\(.*\)\.tar.*\.sha512.*/pbuilder-chroots-\1/p")"

    pushd "${SRCDIR}" > /dev/null

    wget -c "https://distfiles.rgm.io/pbuilder-chroots/${p}/${archive}"{,.sha512}
    sha512sum --check --status "${archive}.sha512"

    sudo rm -rf /tmp/pbuilder
    mkdir /tmp/pbuilder
    fakeroot tar --checkpoint=1000 -xf "${archive}" -C /tmp/pbuilder

    popd > /dev/null
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

${MAKE_CMD:-make} dist-xz

MY_P="${PN}_${PV}"

mv ${P}.tar.xz "${BUILDDIR}/${MY_P}.orig.tar.xz"

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

download_pbuilder_chroot

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
