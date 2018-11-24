cmake_minimum_required(VERSION 3.4)

add_custom_command(
    COMMAND bison ${CMAKE_SOURCE_DIR}/parser.yy -r all -o ${CMAKE_SOURCE_DIR}/parser.cpp --debug
    DEPENDS parser.yy
    OUTPUT ${CMAKE_SOURCE_DIR}/parser.cpp
)

add_executable(human-data
    parser.cpp
    parser_extra.cpp
    parser2.cpp
    scanner.cpp
    main.cpp
)

add_executable(test-human-data
    test/test_buffered_input_stream.cpp
)
