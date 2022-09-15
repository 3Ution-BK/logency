#ifndef LOGENCY_TEST_FILE_DIRECTORY_HPP_
#define LOGENCY_TEST_FILE_DIRECTORY_HPP_

#include <string>

namespace logency::unit_test
{

auto test_dir() -> std::string;

auto unique_file_name(const char *filename = "") -> std::string;

} // namespace logency::unit_test

#endif // LOGENCY_TEST_FILE_DIRECTORY_HPP_
