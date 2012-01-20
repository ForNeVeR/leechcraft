<#
Copyright (C) 2011 by ForNeVeR

Permission is hereby granted, free of charge, to any person obtaining a copy 
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
#>

# This is example LeechPower configuration file. Copy it with config.ps1 name,
# then change following settings.

$config = @{}

# Path to local LeechCraft git repository clone:
$config.RepositoryPath = 'f:\X-Files\Projects\leechcraft'

$config.Components =
@{
    # cmake executable path:
    cmake = 'c:\Program Files (x86)\CMake 2.8\bin\cmake.exe'
}

# $config.Plugins contains paths to every plugin included in build. Paths are
# relative to $config.RepositoryPath.
$config.Plugins =
(
    @{ Name = 'Main'; Path = 'src' },
    @{ Name = 'xmlsettingsdialog', Path = 'src\xmlsettingsdialog' }
)

# Return $config variable from this script:
$config
