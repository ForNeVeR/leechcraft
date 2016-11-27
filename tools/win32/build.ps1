param (
    $Platform = 'x86-windows',
    $vcpkg = 'vcpkg'
)

$ErrorActionPreference = 'Stop'

function exec($command) {
    Write-Host "[build exec] $command $args" -ForegroundColor White
    & $command $args
    if (!$?) {
        throw "[build error] $command $args = $LASTEXITCODE"
    }
}

exec $vcpkg install qt5:$Platform
