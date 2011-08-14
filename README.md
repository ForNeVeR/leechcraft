LeechPower
==========

License
=======
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

About
=====
LeechPower is PowerShell based system for building LeechCraft modular
application under Windows environment.

Visit http://leechcraft.org for details about LeechCraft.

Project status
==============
LeechPower system is under development. Expect no functionality at this moment.

Prerequisites
=============
Minimal requirement for using LeechPower is installed version of Windows
PowerShell v2. All commands in this document must be executed inside PowerShell
host (such as standard PowerShell or more modern PowerShell ISE; third-party
hosts as PoshConsole are also supported).

Usage
=====
At first, start PowerShell host and copy file config.example.ps1 to config.ps1:

    Set-Location 'path to LeechPower directory here'
    Copy-Item config.example.ps1 config.ps1

Now, edit file config.ps1. Follow instructions in file comments.

Finally, run main project script and follow on-screen instructions:

    Set-Location 'path to LeechPower directory here'
    .\Build-LeechCraft.ps1
