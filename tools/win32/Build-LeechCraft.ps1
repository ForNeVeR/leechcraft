$buildDirectory = '.\build32'

if (Test-Path $buildDirectory) {
	Remove-Item -Recurse -Force $buildDirectory
}

.\build32.bat
