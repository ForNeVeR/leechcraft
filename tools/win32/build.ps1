<#
.SYNOPSIS
	Downloads and build the LeechCraft dependencies and build the program.
.PARAMETER Platform
	Target platform triplet in vcpkg notation. Check `vcpkg help triplet` for
	documentation.
.PARAMETER BuildType
	Either Debug or Release.
.PARAMETER BuildDirectory
	Directory where the files build will be placed.
.PARAMETER VcpkgToolchainFile
	Path to vcpkg toolchain file, see
	https://github.com/Microsoft/vcpkg/blob/master/docs/EXAMPLES.md#option-b-cmake-toolchain-file
.PARAMETER cmake
	Path to cmake executable.
.PARAMETER git
	Path to git executable.
.PARAMETER vcpkg
	Path to vcpkg executable.
#>
param (
	$Platform = 'x86-windows',
	$BuildType = 'Debug',

	$BuildDirectory = "$PSScriptRoot/build32",
	$VcpkgToolchainFile = 'D:\Programs\vcpkg\scripts\buildsystems\vcpkg.cmake',

	$cmake = 'cmake',
	$git = 'git',
	$vcpkg = 'vcpkg'
)

$ErrorActionPreference = 'Stop'

function log($text, $header) {
	if (-not $header) {
		$header = 'build'
	}
	Write-Host "[$header] $text" -ForegroundColor White
}

function exec($command) {
	log "$command $args" 'build exec'
	& $command $args
	if (!$?) {
		throw "[build error] $command $args = $LASTEXITCODE"
	}
}

log '=== INSTALLING DEPENDENCIES ==='
$dependencies = @('boost'; 'qt5')
$dependencies | % {
	$dependency = $_
	exec $vcpkg install "$($dependency):$Platform"
}

log '=== BUILDING LEECHCRAFT ==='
$version = exec $git describe
log "LeechCraft version: $version"

if (-not (Test-Path $BuildDirectory -ErrorAction Ignore)) {
	New-Item -ItemType Directory $BuildDirectory | Out-Null
}

$enabledPlugins = @(
	'LACKMAN'
	'SECMAN'
	'AZOTH'
	'SHELLOPEN'
	'GLANCE'
	'TABSLIST'
	'GMAILNOTIFIER'
	'ADVANCEDNOTIFICATIONS'
	'KNOWHOW'
	'LAURE'
	'LIZNOO'
	'SIDEBAR'
)
$disabledPlugins = @('FTP', 'DBUSMANAGER', 'ANHERO', 'LASTFM')

$plugins = ($enabledPlugins | % { "-DENABLE_$_=True" }) + ($disabledPlugins | % { "-DENABLE_$_=False" })

Push-Location $BuildDirectory
try {
	exec $cmake ../../../src `
		"-DCMAKE_BUILD_TYPE=$BuildType" `
		"-DCMAKE_TOOLCHAIN_FILE=$VcpkgToolchainFile" `
		@plugins
} finally {
	Pop-Location
}
