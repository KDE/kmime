#!/bin/bash -eu
#
# SPDX-FileCopyrightText: 2025 Azhar Momin <azhar.momin@kdemail.net>
# SPDX-License-Identifier: LGPL-2.0-or-later

cd $SRC/extra-cmake-modules
cmake . -G Ninja \
  -DBUILD_TESTING=OFF
ninja install -j$(nproc)

cd $SRC/qtbase
./configure -platform linux-clang-libc++ -prefix /usr -static -opensource -confirm-license -debug \
    -qt-pcre -qt-zlib \
    -no-glib -no-icu -no-feature-gui -no-feature-sql -no-feature-network -no-feature-xml \
    -no-feature-dbus -no-feature-printsupport
ninja install -j$(nproc)

cd $SRC/kcodecs
rm -rf poqm
cmake . -G Ninja \
-DBUILD_SHARED_LIBS=OFF \
-DBUILD_TESTING=OFF
ninja install -j$(nproc)

cd $SRC/kmime
rm -rf poqm
cmake . -G Ninja \
-DBUILD_SHARED_LIBS=OFF \
-DBUILD_TESTING=OFF
ninja install -j$(nproc)

$CXX $CXXFLAGS -std=c++20 -fPIC autotests/ossfuzz/kmime_fuzzer.cc -o $OUT/kmime_fuzzer \
    -I /usr/include/QtCore/ \
    -I /usr/local/include/KPim6/KMime -I /usr/local/include/KPim6/KMime/kmime \
    -lKPim6Mime -lKF6Codecs \
    -lQt6Core -lQt6BundledPcre2 -lQt6BundledZLIB \
    -lm -ldl -lpthread \
    $LIB_FUZZING_ENGINE

find . -name "*.mbox" | zip -q $OUT/kmime_fuzzer_seed_corpus.zip -@

cp autotests/ossfuzz/kmime_fuzzer.dict $OUT/kmime_fuzzer.dict
