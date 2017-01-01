param (
    $SourceRoot = "$PSScriptRoot/../../",
    $ConanUserHome = "$PSScriptRoot/conan/",
    $conan = 'conan'
)

$ErrorActionPreference = 'Stop'

function exec($command) {
    Write-Host "[build exec] $command $args" -ForegroundColor White
    & $command $args
    if (!$?) {
        throw "[build error] $command $args = $LASTEXITCODE"
    }
}

$env:CONAN_USER_HOME = $ConanUserHome

Push-Location $SourceRoot
try {
    exec $conan install -s arch="x86_64" -s build_type="Debug" -s compiler="Visual Studio" -s compiler.runtime="MDd" -s compiler.version="14" -s os="Windows"
} finally {
    Pop-Location
}
