project(CoreJSONTests)

set(COREJSONTESTS_SOURCE_FILES
    CoreJSONTests.c
    CoreJSONTests.h
    CoreJSONTests-Prefix.pch
    main.c)

add_executable(CoreJSONTests ${COREJSONTESTS_SOURCE_FILES})

target_link_libraries(CoreJSONTests CoreJSON CFExtension)
