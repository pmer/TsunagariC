#!/bin/sh
here="$PWD/$(dirname $0)/.."
root="$here/.."
rm -rf "$root/Tsungari.app"
cp -a "$root/build-mac-debug/Tsunagari.app" "$root"
mkdir -p "$root/Tsunagari.app/Contents/Resources"
cp "$root/bin/client.json" \
   "$root/bin/testing.world" \
   "$here/data/Tsunagari.icns" \
   "$root/Tsunagari.app/Contents/Resources"
cp "$here/data/Info.plist" \
   "$root/Tsunagari.app/Contents"
touch "$root/Tsunagari.app"
