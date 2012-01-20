# Copyright (C) 2011 by ForNeVeR
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
project(pseudopodia_project)
cmake_minimum_required(VERSION 2.6)

option(ENABLE_TEST "Enable test." ON)

find_package(Qt4 REQUIRED)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:wchar_t-")
    set(CMAKE_EXE_LINKER_FLAGS "/entry:mainCRTStartup")
elseif(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
endif(MSVC)

find_package(Qt4 REQUIRED COMPONENTS QtCore QtNetwork)

include(${QT_USE_FILE})
include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
)

set(SOURCES
    client.cpp
    flappacket.cpp
    tlvdata.cpp
    utils.cpp
)

set(HEADERS
    client.hpp
    connectionstatus.hpp
    flappacket.hpp
    tlvdata.hpp
    utils.hpp
)

qt4_wrap_cpp(MOC_SOURCES ${HEADERS})

add_library(pseudopodia STATIC
    ${SOURCES}
    ${MOC_SOURCES}
    ${HEADERS}
)

target_link_libraries(pseudopodia
    ${QT_LIBRARIES}
)

if(ENABLE_TEST)
    add_subdirectory(test)
endif(ENABLE_TEST)