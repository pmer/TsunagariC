#!/bin/sh
here="$PWD/$(dirname $0)/.."
root="$here/.."
mkdir -p "$root/bin"
cp "$here/data/client.json" "$root/bin"
