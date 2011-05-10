Pseudopodia
===========

Licensing
=========

Pseudopodia licensed under GNU LGPLv3 license. See LICENSE file for details.

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
2. Set variables in corresponding section of build.cmd to your system's paths.
3. Run build.cmd.
4. Open build/pseudopodia_project.sln file with Visual Studio IDE, issue a build
command.

### Others

Still not tested:

    $ md build && cd build && cmake --build ../src

Testing
=======

There is a test program in src/test directory. Useful script for Windows exists
for running this program. As usual, copy test.cmd.sample file with test.cmd
name, check this file for configuration variables and run it.