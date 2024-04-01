mkdir output

$INCLUDES = @("/I", "libs\curl-8.7.1\include\", "/I", "./libs/cJSON/", "/I", "./libs/cwalk/include", "/I", "./libs/utf8-utf16-converter/converter/include/")

$DEFAULTLIBS = @('/DEFAULTLIB:ws2_32.lib', '/DEFAULTLIB:Crypt32.lib', '/DEFAULTLIB:Wldap32.lib', '/DEFAULTLIB:Normaliz.lib', '/DEFAULTLIB:MSVCRT', '/DEFAULTLIB:LIBCMT', '/DEFAULTLIB:Advapi32.lib', '/DEFAULTLIB:User32.lib', '/DEFAULTLIB:Shlwapi.lib')

cl /c .\src\bootstrap.c @INCLUDES


$libcurl = Get-ChildItem ".\libs\curl-8.7.1\build$env:BUILD_ARCH\lib\libcurl_object.dir\Release\" -Filter *.obj
$cjson = "./libs/cJSON/build$env:BUILD_ARCH/Release/libcjson.lib"
$cwalk = "./libs/cwalk/build$env:BUILD_ARCH/Release/cwalk.lib"
$utf8conv = "./libs/utf8-utf16-converter/build$env:BUILD_ARCH/converter/Release/converter.lib"

link  .\bootstrap.obj $libcurl $cjson @DEFAULTLIBS 



mv -Force bootstrap.exe ./output/

cl /c src\hook.c @INCLUDES
link hook.obj $libcurl $cjson $cwalk $utf8conv @DEFAULTLIBS /OUT:"output/hook$env:BUILD_ARCH.dll" /DLL


cl src\hooker.c 
mv -Force hooker.exe "output/hooker$env:BUILD_ARCH.exe"
cp -Force ./src/manifest.xml "output/hooker$env:BUILD_ARCH.exe.manifest"
cp -Force ./src/manifest.xml "output/bootstrap.exe.manifest"

Get-ChildItem -Filter "*.exp" | rm
Get-ChildItem -Filter "*.lib" | rm
Get-ChildItem -Filter "*.obj" | rm
cd output
Get-ChildItem -Filter "*.exp" | rm
Get-ChildItem -Filter "*.lib" | rm
Get-ChildItem -Filter "*.obj" | rm
cd ..

