# Configuration:
$wixDirectory = 'c:\Program Files\Windows Installer XML v3.5\bin'

# Used tools:
$candle = "$wixDirectory\candle"
$light = "$wixDirectory\light"

# Create installer:
& $candle leechcraft.wxs
& $light leechcraft.wixobj