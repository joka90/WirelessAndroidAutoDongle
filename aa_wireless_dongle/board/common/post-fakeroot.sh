#!/bin/bash
echo "args: $@"
set -e

if [[ "${1}" == */f2fs/target ]] ; then
    echo "TODO Have f2fs, configure compression attributes"
    # TODO chattr: Operation not supported while reading flags on
    # configure compression
    echo chattr -R +c $TARGET_DIR/
    echo chattr -R -c $TARGET_DIR/lib/modules/
else
   echo "No compression support for $1"
fi

exit 0

