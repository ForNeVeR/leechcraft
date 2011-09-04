# Configuration:
$wixPath = 'c:\Program Files\Windows Installer XML v3.5\bin'
$sourcePath = '..\LeechCraft'

# Used tools:
$candle = "$wixPath\candle"
$light = "$wixPath\light"

# Create installer:
& $candle leechcraft.wxs *.wixlib "-dSourcePath=`"$sourcePath`""
& $light *.wixobj -o leechcraft.msi
