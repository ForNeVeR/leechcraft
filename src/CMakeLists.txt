project(pseudopodia_project)
cmake_minimum_required(VERSION 2.6)

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
)

set(HEADERS
    client.hpp
    connectionstatus.hpp
)

#set(UIS
#
#)

#set(RESOURCES resources.qrc)

qt4_wrap_cpp(MOC_SOURCES ${HEADERS})
#qt4_wrap_ui(UIS_HEADERS ${UIS})
#qt4_add_resources(RCC ${RESOURCES})

add_library(pseudopodia SHARED
    ${SOURCES}
    ${MOC_SOURCES}
    ${HEADERS}
#    ${RCC}
)

target_link_libraries(pseudopodia
    ${QT_LIBRARIES}
)
