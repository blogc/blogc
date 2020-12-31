#!/bin/bash

set -eo pipefail

export DEBEMAIL="noreply@rafaelmartins.eng.br"
export DEBFULLNAME="Snapshot github-actions"

download_pbuilder_chroots() {
    local arch="amd64"
    local os="$(uname -s | tr '[:upper:]' '[:lower:]')"

    local index="$(curl --silent https://distfiles.rgm.io/pbuilder-chroots/LATEST/)"
    local archive="$(echo "${index}" | sed -n "s/.*\(pbuilder-chroots-${os}-${arch}-.*\)\.sha512.*/\1/p")"
    local folder="$(echo "${index}" | sed -n "s/.*\(pbuilder-chroots-${os}-${arch}-.*\)\.tar.*\.sha512.*/\1/p")"
    local p="$(echo "${folder}" | sed "s/pbuilder-chroots-${os}-${arch}-\(.*\)/pbuilder-chroots-\1/")"

    curl --fail --output "${archive}" "https://distfiles.rgm.io/pbuilder-chroots/${p}/${archive}"
    curl --fail --output "${archive}.sha512" "https://distfiles.rgm.io/pbuilder-chroots/${p}/${archive}.sha512"

    sha512sum --check --status "${archive}.sha512"

    sudo rm -rf /tmp/pbuilder
    fakeroot tar -xf "${archive}" -C /tmp
}

create_reprepro_conf() {
    for dist in "$@"; do
        echo "Origin: blogc-snapshot"
        echo "Label: blogc-snapshot"
        echo "Codename: ${dist}"
        echo "Architectures: amd64"
        echo "Components: main"
        echo "Description: Apt repository containing blogc snapshots"
        echo
    done
}

download_pbuilder_chroots

${MAKE_CMD:-make} dist-xz

MY_P="${PN}_${PV}"

mv ${P}.tar.xz "../${MY_P}.orig.tar.xz"

for dir in /tmp/pbuilder/*/base.cow; do
    export DIST="$(basename "$(dirname "${dir}")" | cut -d- -f1)"
    RES="${BUILDDIR}/deb/${DIST}"
    mkdir -p "${RES}"

    rm -rf "../${P}"
    tar -xf "../${MY_P}.orig.tar.xz" -C ..
    cp -r ../debian "../${P}/"

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
            ;;
    esac

    pushd "../${P}" > /dev/null

    # do not mess with changelog for releases, it should be done manually during version bump
    if [[ ${PV} == *-* ]]; then
        dch \
            --distribution "${DIST}" \
            --newversion "${PV}-${REV}" \
            "snapshot"
    fi

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
    for deb in "../deb/${dist}"/*.deb; do
        reprepro includedeb "${dist}" "${deb}"
    done
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
