cmake_minimum_required(VERSION 3.1)
project(CoreJSONTests)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../Win32\ dependencies/include/ ${CMAKE_CURRENT_SOURCE_DIR}/../../ ${CMAKE_CURRENT_SOURCE_DIR}/../YAJL/include)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../Win32\ dependencies/lib/)

set(COREJSONTESTS_SOURCE_FILES
    CoreJSONTests.c
    CoreJSONTests.h
    CoreJsonTests-Prefix.pch
    main.c)

add_executable(CoreJSONTests ${COREJSONTESTS_SOURCE_FILES})

target_link_libraries(CoreJSONTests CoreJSON CFExtension)
