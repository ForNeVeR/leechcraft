project(pseudopodia_test)
cmake_minimum_required(VERSION 2.6)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ../
)

set(TEST_SOURCES
    consoleclient.cpp
    main.cpp
)

set(TEST_HEADERS
    consoleclient.hpp
)

qt4_wrap_cpp(TEST_MOC_SOURCES ${TEST_HEADERS})

add_executable(pseudopodia_test
    ${TEST_SOURCES}
    ${TEST_MOC_SOURCES}
    ${TEST_HEADERS}
)

target_link_libraries(pseudopodia_test
    ${QT_LIBRARIES}
    pseudopodia
)
