#!/bin/sh

if [ $# -lt 1 ]; then
    echo "You need to pass test executable as an argument!"
    exit 1
fi

cd /opt/tests/buteo-syncfw

export LD_LIBRARY_PATH="$(pwd):${LD_LIBRARY_PATH}"

PATH=".:${PATH}"
exec "${@}" -maxwarnings 0
