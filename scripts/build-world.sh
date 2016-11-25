#!/bin/sh
here="$PWD/$(dirname $0)/.."
root="$here/.."
mkdir -p "$root/bin"
cd "$root/data"
echo $root/bin
zip -r --symlinks -0 "$root/bin/testing.world" .
