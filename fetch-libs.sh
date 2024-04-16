#!/bin/sh
set -x
export PATH=/opt/msvc/bin/x64:$PATH
mkdir libs
cd libs
    git clone https://github.com/DaveGamble/cJSON
    cd cJSON
        msvc-x64-cmake -B build64 -DENABLE_CJSON_TEST=OFF -DCMAKE_C_FLAGS_DEBUG="/MDd /Ob0 /Od /RTC1" -DBUILD_SHARED_AND_STATIC_LIBS=On
        msvc-x86-cmake -B build32 -DENABLE_CJSON_TEST=OFF -DCMAKE_C_FLAGS_DEBUG="/MDd /Ob0 /Od /RTC1" -DBUILD_SHARED_AND_STATIC_LIBS=On
        msvc-x64-cmake --build build64 
        msvc-x86-cmake --build build32
    cd ..

    wget https://curl.se/download/curl-8.7.1.zip -O curl.zip
    unzip curl.zip
    cd curl-8.7.1
        msvc-x64-cmake -B build64 -DBUILD_STATIC_LIBS=ON -DCMAKE_C_FLAGS_DEBUG="/MDd /Ob0 /Od /RTC1"
        msvc-x86-cmake -B build32 -DBUILD_STATIC_LIBS=ON -DCMAKE_C_FLAGS_DEBUG="/MDd /Ob0 /Od /RTC1"
        msvc-x64-cmake --build build64 
        msvc-x86-cmake --build build32 
    cd ..

    git clone https://github.com/likle/cwalk
    cd cwalk
        msvc-x64-cmake -B build64 -DCMAKE_C_FLAGS_DEBUG="/MDd /Ob0 /Od /RTC1" 
        msvc-x86-cmake -B build32 -DCMAKE_C_FLAGS_DEBUG="/MDd /Ob0 /Od /RTC1"
        msvc-x64-cmake --build build64 
        msvc-x86-cmake --build build32 
    cd ..

    git clone https://github.com/Davipb/utf8-utf16-converter
    cd utf8-utf16-converter
        msvc-x64-cmake -B build64 -DCMAKE_C_FLAGS_DEBUG="/MDd /Ob0 /Od /RTC1" 
        msvc-x86-cmake -B build32 -DCMAKE_C_FLAGS_DEBUG="/MDd /Ob0 /Od /RTC1"
        msvc-x64-cmake --build build64 
        msvc-x86-cmake --build build32 
    cd ..

cd ..
