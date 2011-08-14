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

$version = '0.1'

# Prepare service functions:
. '.\Register-ComponentChecks.ps1'

function Show-Banner
{
    Write-Host "LeechCraft build system version '$version'."
}

function Load-Config
{
    $config = .\config.ps1
    Write-Host "Config loaded:"
    Out-Host -InputObject $config
    return $config
}

function Check-Components($config)
{
    $components = $config.Components
    Write-Host "Starting checking following components:"
    Out-Host -InputObject $components

    if (!(Check-cmake $components.cmake))
    {
        Write-Host 'cmake checking failed!'
    }
    
    Write-Host 'Component checking finished.'
}

# Main script:
Show-Banner
$config = Load-Config
Check-Components $config
