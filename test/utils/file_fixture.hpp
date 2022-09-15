#ifndef LOGENCY_TEST_UTILS_FILE_FIXTURE_HPP_
#define LOGENCY_TEST_UTILS_FILE_FIXTURE_HPP_

#include <filesystem>
#include <string>

namespace logency::unit_test::utils::file
{
class file_fixture
{
public:
    explicit file_fixture(const char *where);
    explicit file_fixture(std::filesystem::path where);
    ~file_fixture();

    file_fixture(const file_fixture &other) = delete;
    file_fixture(file_fixture &&other) noexcept = delete;
    auto operator=(const file_fixture &other) -> file_fixture & = delete;
    auto operator=(file_fixture &&other) noexcept -> file_fixture & = delete;

private:
    std::filesystem::path where_;
};

} // namespace logency::unit_test::utils::file

#endif // LOGENCY_TEST_UTILS_FILE_FIXTURE_HPP_
