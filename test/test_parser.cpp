#include "../test_lib/testing.h"
#include "parse_test.h"

#include <sstream>
#include <vector>

#include <sys/types.h>
#include <dirent.h>

using namespace Testing;
using namespace std;

namespace human_data {

vector<string> dir_entries(const std::string& dir_path)
{
    vector<string> entries;
    DIR* dirp = opendir(dir_path.c_str());
    struct dirent * dp;
    while ((dp = readdir(dirp)) != nullptr) {
        string name = dp->d_name;
        if (name == "." or name == "..")
            continue;
        entries.push_back(name);
    }
    closedir(dirp);
    return entries;
}

static
bool parse_test_func(const string & file_name)
{
    Parse_Test test;
    test.evaluate_test_file("../test/cases/" + file_name);
    return test.success();
}

Test_Set::Func parse_test(const string & file_name)
{
    return std::bind(&parse_test_func, file_name);
}

Test_Set parser_tests()
{
    vector<string> test_files = dir_entries("../test/cases");

    Test_Set tests;

    for(auto & file : test_files)
        tests.add_test(file, parse_test(file));

    return tests;
}

}
