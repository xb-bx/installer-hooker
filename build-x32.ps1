$vswhereOutput = & 'C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe'
$match = echo $vswhereOutput | Select-String -Pattern "installationPath: (.*)"
$envVarsPath = $match.Matches[0].Groups[1].Value + "\VC\Auxiliary\Build\vcvars32.bat"
$env:BUILD_ARCH="32"
cmd /c "$envVarsPath & pwsh build.ps1"
