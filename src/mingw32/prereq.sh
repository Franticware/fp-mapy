#!/bin/bash

CURLVER=8.2.1_11
rm -Rf curl
rm -f curl-$CURLVER-win32-mingw.zip
wget https://curl.se/windows/dl-$CURLVER/curl-$CURLVER-win32-mingw.zip
unzip curl-$CURLVER-win32-mingw.zip
mv curl-$CURLVER-win32-mingw curl

SDL2VER=2.26.3
rm -Rf SDL2
rm -f SDL2-devel-$SDL2VER-mingw.tar.gz SDL2.dll
wget https://libsdl.org/release/SDL2-devel-$SDL2VER-mingw.tar.gz
tar -xvf SDL2-devel-$SDL2VER-mingw.tar.gz
mv SDL2-$SDL2VER SDL2

rm -Rf libjpeg-turbo
git clone https://github.com/libjpeg-turbo/libjpeg-turbo.git
cd libjpeg-turbo
cmake -DCMAKE_TOOLCHAIN_FILE="../mingw32-toolchain.cmake" \
-S. -B. \
-DCMAKE_GNUtoMS:BOOL="0" -DCMAKE_BUILD_TYPE:STRING="MinSizeRel" -DCMAKE_INSTALL_PREFIX:PATH="." -DCMAKE_SYSTEM_PROCESSOR:STRING="" -DWITH_ARITH_DEC:BOOL="0" -DCMAKE_INSTALL_LIBDIR:PATH="lib" -DWITH_SIMD:BOOL="1" -DWITH_TURBOJPEG:BOOL="0" -DPKGNAME:STRING="libjpeg-turbo" -DBUILD:STRING="20221228" -DWITH_ARITH_ENC:BOOL="0" -DENABLE_SHARED:BOOL="0" 
make
cd ..

rm -Rf zlib
git clone https://github.com/madler/zlib
cd zlib
cmake -DCMAKE_TOOLCHAIN_FILE="../mingw32-toolchain.cmake" \
-S. -B. \
-DLIBRARY_OUTPUT_PATH:PATH="" -DINSTALL_LIB_DIR:PATH="/usr/local/lib" -DCMAKE_BUILD_TYPE:STRING="MinSizeRel" -DCMAKE_INSTALL_PREFIX:PATH="/usr/local" -DINSTALL_MAN_DIR:PATH="/usr/local/share/man" -DINSTALL_PKGCONFIG_DIR:PATH="/usr/local/share/pkgconfig" -DEXECUTABLE_OUTPUT_PATH:PATH="" 
make
cd ..
