#include "file_fixture.hpp"

#include <stdexcept>

namespace logency::unit_test::utils::file
{

file_fixture::file_fixture(const char *where)
    : file_fixture{std::filesystem::path{where}}
{
}

file_fixture::file_fixture(std::filesystem::path where)
    : where_{std::move(where)}
{
    if (!std::filesystem::exists(where_))
    {
        return;
    }

    std::string message{
        "Testing path exist. The test will not be executed. Please clean "
        "up the directory in order to continue. Path name:"};
    message.append(where_.string());

    throw std::runtime_error(message);
}

file_fixture::~file_fixture()
{
    if (std::filesystem::exists(where_))
    {
        std::ignore = std::filesystem::remove_all(where_);
    }
}

} // namespace logency::unit_test::utils::file
