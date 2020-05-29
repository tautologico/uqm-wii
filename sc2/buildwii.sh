#!/bin/sh

export BUILD_HOST=WII
export BUILD_HOST_ENDIAN=big

/bin/sh build/unix/build.sh "$@"
