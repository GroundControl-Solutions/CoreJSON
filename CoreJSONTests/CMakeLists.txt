project(CoreJSONTests)

set(COREJSONTESTS_SOURCE_FILES
    ${version_rc}
    CoreJSONTests.c
    CoreJSONTests.h
    CoreJsonTests-Prefix.pch
    main.c)

add_executable(CoreJSONTests ${COREJSONTESTS_SOURCE_FILES})

target_link_libraries(CoreJSONTests CoreJSON CFExtension)
