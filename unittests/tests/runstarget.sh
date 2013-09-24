#!/bin/sh

echo "running ${1}..."

FILE=${1##*/}  

cd /opt/tests/buteo-syncfw
export LD_LIBRARY_PATH=/opt/tests/buteo-syncfw:$LD_LIBRARY_PATH

${1} -maxwarnings 0 1>/tmp/$FILE.out 2>&1
RESULT=$?

echo "$RESULT is return value of executing ${1}" >> /tmp/$FILE.out

grep "Totals:" /tmp/$FILE.out >/tmp/$FILE.cmp

# Exit with the same code as the test binary
exit $RESULT
# Exit always with zero until problems in CI environment are resolved
#exit 0
