

add_executable(run_tests
    ${CMAKE_SOURCE_DIR}/test_lib/testing.cpp
    ${CMAKE_SOURCE_DIR}/event_printing.cpp
    ${CMAKE_SOURCE_DIR}/parser2.cpp
    parse_test.cpp
    test_buffered_input_stream.cpp
    test_parser.cpp
    test_main.cpp
)
