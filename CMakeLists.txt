cmake_minimum_required(VERSION 3.1)
project(CoreJSON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../Win32\ dependencies/include/ ${CMAKE_CURRENT_SOURCE_DIR}/../ ${CMAKE_CURRENT_SOURCE_DIR}/YAJL/include)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../Win32\ dependencies/lib/)

set(COREJSON_SOURCE_FILES
    CJPlatformCompatibility.h
    CoreJSON.c
    CoreJSON.h
    CoreJSON-Prefix.pch
    YAJL/include/yajl/yajl_common.h
    YAJL/include/yajl/yajl_gen.h
    YAJL/include/yajl/yajl_parse.h
    YAJL/include/yajl/yajl_tree.h
    YAJL/include/yajl/yajl_version.h
    YAJL/yajl/example/parse_config.c
    YAJL/yajl/perf/documents.c
    YAJL/yajl/perf/documents.h
    YAJL/yajl/perf/perftest.c
    YAJL/yajl/reformatter/json_reformat.c
    YAJL/yajl/src/api/yajl_common.h
    YAJL/yajl/src/api/yajl_gen.h
    YAJL/yajl/src/api/yajl_parse.h
    YAJL/yajl/src/api/yajl_tree.h
    YAJL/yajl/src/yajl.c
    YAJL/yajl/src/yajl_alloc.c
    YAJL/yajl/src/yajl_alloc.h
    YAJL/yajl/src/yajl_buf.c
    YAJL/yajl/src/yajl_buf.h
    YAJL/yajl/src/yajl_bytestack.h
    YAJL/yajl/src/yajl_encode.c
    YAJL/yajl/src/yajl_encode.h
    YAJL/yajl/src/yajl_gen.c
    YAJL/yajl/src/yajl_lex.c
    YAJL/yajl/src/yajl_lex.h
    YAJL/yajl/src/yajl_parser.c
    YAJL/yajl/src/yajl_parser.h
    YAJL/yajl/src/yajl_tree.c
    YAJL/yajl/src/yajl_version.c
    YAJL/yajl/test/yajl_test.c
    YAJL/yajl/verify/json_verify.c)

add_library(CoreJSON ${COREJSON_SOURCE_FILES})