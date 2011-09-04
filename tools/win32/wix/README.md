LeechCraft Install Package
==========================
In this directory you can find source code for compilng LeechCraft MSI package
for Windows. It is based on open-source WiX toolset.

Prerequisites
=============
For making package, you need to install following tools:

1. WiX toolset (http://wix.sourceforge.net/).
2. Microsoft Visual Studio (only for obtaining Visual Studio 2010 C++
Redistributable merge module).
3. Windows PowerShell for executing scripts.

Also you need compiled version of LeechCraft.

Compiling
=========
1. Open file Make-Installer.ps1, edit variables in `# Configuration` section.
2. Open PowerShell console (there are various standard and 3rd-party PowerShell
hosts; for example, you may use Windows built-in powershell.exe host).
3. Execute Make-Installer.ps1 script:
	PS> .\Make-Installer.ps1
After that, file leechcraft.msi must be compiled into current directory.
