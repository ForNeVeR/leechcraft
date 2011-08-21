Pseudopodia
===========

Licensing
=========
Pseudopodia is distributed under terms of MIT license.

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

Description
===========
Pseudopodia is an instant messaging library for OSCAR protocol. Developed with
Qt.

Building
========
### Prerequisites
1. Qt developer version.
2. cmake for managing building process.
3. C++ compiler (Visual Studio 2010 recommended for Windows, g++ for others).

### Windows
1. Rename file build.cmd.sample to build.cmd.
2. Set variables in corresponding section of build.cmd to your system's paths:

    set QTDIR="C:\Programs\Qt\4.7.1"
    set CMAKE_BIN="cmake"

3. Run build.cmd.
4. Open build/pseudopodia_project.sln file with Visual Studio IDE, issue a build
command.

### Others
Still not tested:

    $ md build && cd build && cmake --build ../src

Testing
=======
There is a test source in src/test directory. It is compiled with library by
default.

### Running test
For running test program on Windows you may use special script. As usual, copy
test.cmd.sample file with test.cmd name and change (yet single) configuration
variable:

    set QT_LIBS=C:\Programs\Qt\4.7.1\lib
    
Then run test.cmd file.

### Using test
Test is simple console shell for direct interacting with library functions. See
online help system.
