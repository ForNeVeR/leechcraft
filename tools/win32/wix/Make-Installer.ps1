# Configuration:

# Path to WiX executables; your system is likely to accept this setting as is:
$wixPath = "${Env:\WIX}\bin"

# Path to directory with deployed LeechCraft:
$sourcePath = '..\LeechCraft'

# End of configuration.

# Used tools:
$candle = "$wixPath\candle"
$light = "$wixPath\light"

# Create installer:
& $candle leechcraft.wxs *.wixlib "-dSourcePath=`"$sourcePath`""
& $light *.wixobj -o leechcraft.msi
