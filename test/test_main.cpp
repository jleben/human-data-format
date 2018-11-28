#include "../test_lib/testing.h"

using namespace Testing;

Test_Set buffered_input_stream_tests();

int main(int argc, char *argv[])
{
    Test_Set tests = {
        { "buffered-input-stream", buffered_input_stream_tests() }
    };

    Testing::run(tests, argc, argv);
}
