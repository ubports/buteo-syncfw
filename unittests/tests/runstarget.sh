#!/bin/sh -e

if [ $# -lt 1 ]; then
    echo "You need to pass test executable as an argument!"
    exit 1
fi

TESTS_DIR="$(dirname "${0}")"
TMP_DIR="/tmp/$(basename "${TESTS_DIR}")"

rm -rf "${TMP_DIR}"
mkdir -p "${TMP_DIR}"

# Copy test data into tmp dir (read/write enabled location)
mkdir -p "${TMP_DIR}/syncprofiletests"
cp -a "${TESTS_DIR}/syncprofiletests/testprofiles" "${TMP_DIR}/syncprofiletests/"

# Test data are searched with paths relative to CWD
cd "${TMP_DIR}"

export LD_LIBRARY_PATH="${TESTS_DIR}:${LD_LIBRARY_PATH}"

# Accept both absolute and relative path to test executable
TESTDIR=$(cd "${TESTS_DIR}"; readlink -f "${1}")
TEST=$(basename ${1})
shift

exec "${TESTDIR}/${TEST}" "${@}" -maxwarnings 0
