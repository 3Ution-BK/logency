#define DOCTEST_CONFIG_IMPLEMENT
#include "include_doctest.hpp"

#include "file_directory.hpp"
#include "utils/file_fixture.hpp"

int main(int argc, char **argv)
{
    doctest::Context context;

    logency::unit_test::utils::file::file_fixture fixture{
        logency::unit_test::test_dir()};

    context.applyCommandLine(argc, argv);

    return context.run();
}
