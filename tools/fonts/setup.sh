#!/bin/sh
# Compila o bdfconv do U8g2 em tools/fonts/.build, numa versão fixa.
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
# O Makefile usa -O4, que o Clang recusa
make -C "$SOURCE/tools/font/bdfconv" CFLAGS=-O2 >/dev/null
cp "$SOURCE/tools/font/bdfconv/bdfconv" "$BUILD/bdfconv"
echo "bdfconv pronto em $BUILD/bdfconv"
