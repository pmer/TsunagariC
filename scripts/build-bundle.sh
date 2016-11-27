#!/bin/sh
here="$PWD/$(dirname $0)/.."
root="$here/.."
rm -rf "$root/Tsungari.app"
cp -a "$root/build/Tsunagari.app" "$root"
mkdir -p "$root/Tsunagari.app/Contents/Resources"
cp "$root/bin/client.ini" \
   "$root/bin/testing.world" \
   "$here/data/Tsunagari.icns" \
   "$root/Tsunagari.app/Contents/Resources"
