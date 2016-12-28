#!/bin/sh
here="$PWD/$(dirname $0)/.."
root="$here/.."
mkdir -p "$root/bin"
cd "$root/data"

packtool=$(find "$root" -name pack-tool | head -n 1)

if [ ! -x "$packtool" ]; then
	echo "Cannot find pack-tool"
	exit 1
fi

"$packtool" create ../bin/testing.world *
