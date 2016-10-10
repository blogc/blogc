#!/bin/bash

set -xe -o pipefail

TEMP="$(mktemp -d)"
[[ -n "${TEMP}" ]]

trap_func() {
    [[ -e "${TEMP}/output.txt" ]] && cat "${TEMP}/output.txt"
    rm -rf "${TEMP}"
}

trap trap_func EXIT

mkdir -p "${TEMP}/repos"
git init --bare "${TEMP}/repos/foo.git" &> /dev/null

ln -s "${PWD}/blogc-git-receiver" "${TEMP}/repos/foo.git/hooks/post-receive"

cat > "${TEMP}/tmp.txt" <<EOF
blob
mark :1
data 4
bar

reset refs/heads/master
commit refs/heads/master
mark :2
author Rafael G. Martins <rafael@rafaelmartins.eng.br> 1476033730 +0200
committer Rafael G. Martins <rafael@rafaelmartins.eng.br> 1476033888 +0200
data 11
testing...
M 100644 :1 foo

EOF

cd "${TEMP}/repos/foo.git"
git fast-import < "${TEMP}/tmp.txt" &> /dev/null

git init --bare "${TEMP}/repos/bar.git" &> /dev/null

HOME="${TEMP}" ${TESTS_ENVIRONMENT} ./hooks/post-receive 2>&1 | tee "${TEMP}/output.txt"
grep "warning: repository mirroring disabled" "${TEMP}/output.txt" &> /dev/null

git config --local remote.mirror.pushurl "${TEMP}/repos/bar.git"
HOME="${TEMP}" ${TESTS_ENVIRONMENT} ./hooks/post-receive 2>&1 | tee "${TEMP}/output.txt"
grep "[new branch] *master" "${TEMP}/output.txt" &> /dev/null

git config --local --unset remote.mirror.pushurl
rm -rf "${TEMP}/repos/bar.git"
git init --bare "${TEMP}/repos/bar.git" &> /dev/null
git config --local remote.mirror.url "${TEMP}/repos/bar.git"
HOME="${TEMP}" ${TESTS_ENVIRONMENT} ./hooks/post-receive 2>&1 | tee "${TEMP}/output.txt"
grep "[new branch] *master" "${TEMP}/output.txt" &> /dev/null

git config --local --unset remote.mirror.url
rm -rf "${TEMP}/repos/bar.git"
cat > "${TEMP}/blogc-git-receiver.ini" <<EOF
[repo:boo.git]
mirror = 123

[repo:foo.git]
mirror = ${TEMP}/repos/bar.git

[repo:bar.git]
mirror = lol
EOF
git init --bare "${TEMP}/repos/bar.git" &> /dev/null
HOME="${TEMP}" ${TESTS_ENVIRONMENT} ./hooks/post-receive 2>&1 | tee "${TEMP}/output.txt"
grep "[new branch] *master" "${TEMP}/output.txt" &> /dev/null

rm -rf "${TEMP}/repos/bar.git"
cat > "${TEMP}/blogc-git-receiver.ini" <<EOF
asd[repo:boo.git]
mirror = 123

[repo:foo.git]
mirror = ${TEMP}/repos/bar.git

[repo:bar.git]
mirror = lol
EOF
git init --bare "${TEMP}/repos/bar.git" &> /dev/null
HOME="${TEMP}" ${TESTS_ENVIRONMENT} ./hooks/post-receive 2>&1 | tee "${TEMP}/output.txt"
grep "warning: failed to parse configuration file " "${TEMP}/output.txt" &> /dev/null

rm "${TEMP}/output.txt"
