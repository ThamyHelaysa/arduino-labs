#!/bin/sh
# Builds U8g2's bdfconv into tools/fonts/.build, at a pinned version.
set -eu

VERSION=2.37.1
DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
BUILD="$DIR/.build"
SOURCE="$BUILD/u8g2"

mkdir -p "$BUILD"
rm -rf "$SOURCE"
git clone --quiet --depth 1 --branch "$VERSION" --filter=blob:none --sparse \
  https://github.com/olikraus/u8g2.git "$SOURCE"
git -C "$SOURCE" sparse-checkout set tools/font/bdfconv
# The Makefile uses -O4, which Clang rejects
make -C "$SOURCE/tools/font/bdfconv" CFLAGS=-O2 >/dev/null
cp "$SOURCE/tools/font/bdfconv/bdfconv" "$BUILD/bdfconv"
echo "bdfconv ready at $BUILD/bdfconv"
