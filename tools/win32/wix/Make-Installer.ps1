# Configuration:
$wixPath = 'c:\Program Files\Windows Installer XML v3.5\bin'
$sourcePath = '..\LeechCraft'

# Used tools:
$candle = "$wixPath\candle"
$light = "$wixPath\light"

# Create installer:
& $candle leechcraft.wxs "-dSourcePath=$sourcePath"
& $light leechcraft.wixobj
