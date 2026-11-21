#!/usr/bin/env bash
# Fetches the SDL2 source the Android build compiles (unmodified release tarball, pinned and checksummed) into
# app/jni/SDL2, and copies SDL's Java glue (org.libsdl.app) into the app. Run it once before building; CI does.
set -euo pipefail

SDL_VERSION=2.32.10
SDL_SHA256=5f5993c530f084535c65a6879e9b26ad441169b3e25d789d83287040a9ca5165
SDL_URL="https://github.com/libsdl-org/SDL/releases/download/release-${SDL_VERSION}/SDL2-${SDL_VERSION}.tar.gz"

here="$(cd "$(dirname "$0")" && pwd)"
cache="${SDL_CACHE_DIR:-$here/.cache}"
tarball="$cache/SDL2-${SDL_VERSION}.tar.gz"
mkdir -p "$cache"

if [ ! -f "$tarball" ]; then
  echo "Downloading SDL2 ${SDL_VERSION}"
  curl -fsSL --retry 3 -o "$tarball" "$SDL_URL"
fi
echo "${SDL_SHA256}  ${tarball}" | sha256sum -c -

rm -rf "$here/app/jni/SDL2"
mkdir -p "$here/app/jni/SDL2"
tar -xzf "$tarball" -C "$here/app/jni/SDL2" --strip-components=1

java_dst="$here/app/src/main/java/org/libsdl/app"
rm -rf "$java_dst"
mkdir -p "$java_dst"
cp "$here"/app/jni/SDL2/android-project/app/src/main/java/org/libsdl/app/*.java "$java_dst/"
# SDL's sample android-project ships its own (old) Gradle wrapper jar. The build never uses it, and it fails
# gradle/actions wrapper validation, so drop it.
find "$here/app/jni/SDL2" -name gradle-wrapper.jar -delete
echo "SDL2 ${SDL_VERSION} ready."
