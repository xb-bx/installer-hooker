mkdir libs
cd libs
    $vswhereOutput = & 'C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe'
    $match = echo $vswhereOutput | Select-String -Pattern "installationPath: (.*)"
    $envVarsPath = $match.Matches[0].Groups[1].Value + "\VC\Auxiliary\Build\vcvars64.bat"
    cmd /c "$envVarsPath & pwsh -c ""Get-ChildItem env: | Export-Clixml ./env-vars.clixml"""
    Import-Clixml ./env-vars.clixml | % { Set-Item "env:$($_.Name)" $_.Value }

    git clone https://github.com/DaveGamble/cJSON
    cd cJSON
        cmake -A x64 -B build64 -DBUILD_SHARED_AND_STATIC_LIBS=On
        cmake -A Win32 -B build32 -DBUILD_SHARED_AND_STATIC_LIBS=On
        cmake --build build64 --config Release
        cmake --build build32 --config Release
    cd ..

    iwr https://curl.se/download/curl-8.7.1.zip -OutFile curl.zip
    Expand-Archive -Path curl.zip -DestinationPath .
    cd curl-8.7.1
        cmake -A x64 -B build64 
        cmake -A Win32 -B build32
        cmake --build build64 --config Release
        cmake --build build32 --config Release
    cd ..

    git clone https://github.com/likle/cwalk
    cd cwalk
        cmake -A x64 -B build64 -DBUILD_SHARED_AND_STATIC_LIBS=On
        cmake -A Win32 -B build32 -DBUILD_SHARED_AND_STATIC_LIBS=On
        cmake --build build64 --config Release
        cmake --build build32 --config Release
    cd ..

    git clone https://github.com/Davipb/utf8-utf16-converter
    cd utf8-utf16-converter
        cmake -A x64 -B build64 -DBUILD_SHARED_AND_STATIC_LIBS=On
        cmake -A Win32 -B build32 -DBUILD_SHARED_AND_STATIC_LIBS=On
        cmake --build build64 --config Release
        cmake --build build32 --config Release
    cd ..

cd ..
