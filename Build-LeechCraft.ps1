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
. '.\Register-PathFunctions.ps1'
. '.\Register-CheckFunctions.ps1'
. '.\Register-BuildFunctions.ps1'

function Show-Banner
{
    Write-Host "LeechCraft build system version '$version'."
}

function Load-Config
{
    $config = .\config.ps1
    Write-Host 'Config loaded:'
    Out-Host -InputObject $config
    return $config
}

function Check-Components($config)
{
    $components = $config.Components
    Write-Host 'Started checking following components:'
    Out-Host -InputObject $components

    if (!(Check-cmake $components.cmake))
    {
        Write-Host 'cmake checking failed!'
        return $false
    }
    
    Write-Host 'Component checking finished.'
    return $true
}

function Do-Build($config)
{
    $pluginStates = @{}
    Push-Location
    
    try
    {
        $buildPath = Append-Path . 'build'
        $plugins = $config.Plugins
        Write-Host "Building to '$buildPath'..."
        
        Write-Host 'Plugin list:'
        $format = 
        (
            @{ Name = 'Name'; Expression = { $_.Name } },
            @{ Name = 'Path'; Expression = { $_.Path } }
        )
        $plugins | Format-Table $format | Out-Host
        
        if (Test-Path $buildPath)
        {
            Remove-Item $buildPath -Recurse
        }
        New-Item $buildPath -Type directory | Out-Null
        
        $repoPath = $config.RepositoryPath
        $cmake = $config.Components.cmake
        
        foreach ($plugin in $plugins)
        {
            Set-Location $buildPath
            $pluginPath = Append-Path $repoPath $plugin.Path
            Write-Host "Configuring plugin in '$pluginPath'..."
            $outPath = Append-Path $buildPath $plugin.Name
            New-Item $outPath -Type directory | Out-Null
            Write-Host "Configuring into '$outPath'..."
            
            Set-Location $outPath            
            $pluginStates[$plugin.Name] = Execute-cmake $cmake $pluginPath
        }
    }
    finally
    {
        Write-Host 'Build finished.'
        Write-Host 'Plugin states:'
        $format =
        (
            @{ Name = 'Name'; Expression = { $_.Name } },
            @{ Name = 'Status'; Expression = { $_.Value.Status } },
            @{ Name = 'Output'; Expression = { $_.Value.Output } }
        )
        $pluginStates | Format-Table $format | Out-Host
        Pop-Location
    }
    
    return $pluginStates
}

# Main script:
Show-Banner
$config = Load-Config
if (Check-Components $config)
{
    Write-Host 'Component check succeed.'
    $report = Do-Build $config
    Write-Host ('Building finished. Check returned variable or plugin ' +
        'directories for logs.')
    return $report
}
else
{
    Write-Host 'Component check failed. Cannot build.'
}
