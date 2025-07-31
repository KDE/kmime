#!/bin/bash -eu
#
# SPDX-FileCopyrightText: 2025 Azhar Momin <azhar.momin@kdemail.net>
# SPDX-License-Identifier: LGPL-2.0-or-later

export PATH="$WORK/bin:$PATH"

cd $SRC/extra-cmake-modules
cmake . -G Ninja \
    -DBUILD_TESTING=OFF \
    -DCMAKE_INSTALL_PREFIX=$WORK
ninja install -j$(nproc)

cd $SRC/qtbase
./configure -platform linux-clang-libc++ -prefix $WORK -static -opensource -confirm-license -debug \
    -qt-pcre -qt-zlib \
    -no-glib -no-icu -no-feature-gui -no-feature-sql -no-feature-network -no-feature-xml \
    -no-feature-dbus -no-feature-printsupport -no-feature-widgets
ninja install -j$(nproc)

cd $SRC/kcodecs
rm -rf poqm
cmake . -G Ninja \
    -DBUILD_SHARED_LIBS=OFF \
    -DBUILD_TESTING=OFF \
    -DCMAKE_INSTALL_PREFIX=$WORK
ninja install -j$(nproc)

cd $SRC/kmime
rm -rf poqm
cmake . -G Ninja \
    -DBUILD_SHARED_LIBS=OFF \
    -DBUILD_TESTING=OFF \
    -DBUILD_FUZZERS=ON \
    -DCMAKE_INSTALL_PREFIX=$WORK
ninja install -j$(nproc)

# Copy the fuzzer
cp bin/fuzzers/kmime_fuzzer $OUT/kmime_fuzzer
# Create a seed corpus
find . -name "*.mbox" | zip -q $OUT/kmime_fuzzer_seed_corpus.zip -@
# Copy the dictionary
cp autotests/ossfuzz/kmime_fuzzer.dict $OUT/kmime_fuzzer.dict
