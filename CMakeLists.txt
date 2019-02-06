cmake_minimum_required(VERSION 3.11)
project(CoreJSON)

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
    YAJL/yajl/src/yajl_version.c)

add_library(CoreJSON ${COREJSON_SOURCE_FILES} CJExport.h)

set_target_properties(CoreJSON PROPERTIES DEFINE_SYMBOL "CJ_BUILDING_CJ")

target_link_libraries(CoreJSON
      PUBLIC
	${CoreFoundation}
	PRIVATE
	${version_rc}
)

target_include_directories(CoreJSON PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/YAJL/include)
