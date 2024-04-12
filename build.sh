mkdir output
if [[ $BUILD_ARCH == 64 ]] then
    export PATH=/opt/msvc/bin/x64:$PATH
else 
    export PATH=/opt/msvc/bin/x86:$PATH
fi
INCLUDES="-I ./libs/cJSON/ -I ./libs/curl-8.7.1/include/ -I ./libs/cwalk/include -I ./libs/utf8-utf16-converter/converter/include"
DEFAULTLIBS='/DEFAULTLIB:ws2_32.lib /DEFAULTLIB:Crypt32.lib /DEFAULTLIB:Wldap32.lib /DEFAULTLIB:Normaliz.lib /DEFAULTLIB:MSVCRT /DEFAULTLIB:LIBCMT /DEFAULTLIB:Advapi32.lib /DEFAULTLIB:User32.lib /DEFAULTLIB:Shlwapi.lib'

cl /DWINE /c ./src/bootstrap.c $INCLUDES

libcurl=./libs/curl-8.7.1/build$BUILD_ARCH/lib/libcurl-d.lib
cjson=./libs/cJSON/build$BUILD_ARCH/libcjson.lib
cwalk=./libs/cwalk/build$BUILD_ARCH/cwalk.lib
utf8conv=./libs/utf8-utf16-converter/build$BUILD_ARCH/converter/converter.lib

link ./bootstrap.obj $libcurl $cjson $cwalk $DEFAULTLIBS

mv ./bootstrap.exe ./output/

cl /c /DWINE ./src/hook.c $INCLUDES
link hook.obj $libcurl $cjson $cwalk $utf8conv $DEFAULTLIBS /OUT:./output/hook$BUILD_ARCH.dll /DLL

cl /DWINE ./src/hooker.c
mv hooker.exe ./output/hooker$BUILD_ARCH.exe
cp ./src/manifest.xml ./output/hooker$BUILD_ARCH.exe.manifest
cp ./src/manifest.xml ./output/bootstrap.exe.manifest

rm -f *.exp *.lib *obj
rm -f output/*.exp output/*.lib output/*obj
