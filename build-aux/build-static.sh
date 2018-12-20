#!/bin/bash

set -ex

${MAKE_CMD:-make} LDFLAGS="-all-static" blogc

xz -zc blogc > "blogc-static-amd64-${PV}.xz"
