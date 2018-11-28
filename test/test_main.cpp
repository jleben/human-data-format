#include "../test_lib/testing.h"

using namespace Testing;

namespace human_data {

Test_Set buffered_input_stream_tests();
Test_Set parser_tests();

}

using namespace human_data;

int main(int argc, char *argv[])
{
    Test_Set tests = {
        { "buffered-input-stream", buffered_input_stream_tests() },
        { "parser", parser_tests() }
    };

    Testing::run(tests, argc, argv);
}
