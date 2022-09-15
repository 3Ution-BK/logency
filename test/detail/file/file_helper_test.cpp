#include "logency/detail/file/file_helper.hpp"

#include "file_directory.hpp"
#include "include_doctest.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

namespace logency::unit_test::detail::file
{

namespace
{

auto test_path(const std::filesystem::path &directory, const char *file)
    -> std::filesystem::path;

inline auto test_path(const std::filesystem::path &directory, const char *file)
    -> std::filesystem::path
{
    std::filesystem::path where{directory};

    where.append(file);

    return where;
}

} // namespace

TEST_SUITE("logency::detail::file")
{
    SCENARIO(
        "void create_necessary_directory(const std::filesystem::path &path)")
    {
        GIVEN("directories: (workspace)/exist_dir")
        {
            const auto workspace{
                test_path(test_dir(), "create_necessary_directory")};
            const auto exist_dir{test_path(workspace, "exist_dir")};

            std::filesystem::create_directories(workspace);
            std::filesystem::create_directory(exist_dir);

            REQUIRE(std::filesystem::exists(workspace));
            REQUIRE(std::filesystem::exists(exist_dir));

            WHEN("called it with no directory")
            {
                const auto file{test_path(workspace, "foo.txt")};

                CHECK_NOTHROW({
                    logency::detail::file::create_necessary_directory(file);
                });

                THEN("expect do nothing")
                {
                    CHECK(!std::filesystem::exists(file));
                }
            }

            WHEN("called it with exist directory only")
            {
                const auto file{test_path(workspace, "exist_dir/foo.txt")};

                CHECK_NOTHROW({
                    logency::detail::file::create_necessary_directory(file);
                });

                THEN("expect do nothing")
                {
                    CHECK(!std::filesystem::exists(file));
                }
            }

            WHEN("called it with one not exist directory")
            {
                const auto not_exist_dir{test_path(workspace, "not_exist_dir")};
                const auto file{test_path(not_exist_dir, "foo.txt")};

                REQUIRE(!std::filesystem::exists(not_exist_dir));

                CHECK_NOTHROW({
                    logency::detail::file::create_necessary_directory(file);
                });

                THEN("expect not exist directory were created")
                {
                    CHECK(std::filesystem::exists(not_exist_dir));
                    CHECK(!std::filesystem::exists(file));
                }
            }

            WHEN("called it with multiple not exist directory")
            {
                const auto level_one_dir{test_path(workspace, "level_one_dir")};
                const auto level_two_dir{
                    test_path(level_one_dir, "level_two_dir")};
                const auto file{test_path(level_two_dir, "foo.txt")};

                REQUIRE(!std::filesystem::exists(level_one_dir));
                REQUIRE(!std::filesystem::exists(level_two_dir));

                CHECK_NOTHROW({
                    logency::detail::file::create_necessary_directory(file);
                });

                THEN("expect not exist directories were created")
                {
                    CHECK(std::filesystem::exists(level_one_dir));
                    CHECK(std::filesystem::exists(level_two_dir));
                    CHECK(!std::filesystem::exists(file));
                }
            }

            WHEN("called it with exist and not exist directory")
            {
                const auto not_exist_dir{test_path(exist_dir, "not_exist_dir")};
                const auto file{test_path(not_exist_dir, "foo.txt")};

                REQUIRE(!std::filesystem::exists(not_exist_dir));

                CHECK_NOTHROW({
                    logency::detail::file::create_necessary_directory(file);
                });

                THEN("expect not exist directories were created")
                {
                    CHECK(std::filesystem::exists(not_exist_dir));
                    CHECK(!std::filesystem::exists(file));
                }
            }
        }
    }

    SCENARIO(
        "auto extract_file_extension(const std::filesystem::path &filename)")
    {
        GIVEN("single_file.extension")
        {
            std::filesystem::path file{"single_file.extension"};

            WHEN("called by this function")
            {
                auto actual{
                    logency::detail::file::extract_file_extension(file)};

                THEN("get correct result")
                {
                    CHECK_EQ(actual,
                             std::make_tuple("single_file", ".extension"));
                }
            }
        }

        GIVEN("directory/file.extension")
        {
            std::filesystem::path file{"directory/file.extension"};

            WHEN("called by this function")
            {
                auto actual{
                    logency::detail::file::extract_file_extension(file)};

                THEN("get correct result")
                {
                    CHECK_EQ(actual,
                             std::make_tuple("directory/file", ".extension"));
                }
            }
        }

        GIVEN("really/long/nested/directory/list/file.extension")
        {
            std::filesystem::path file{
                "really/long/nested/directory/list/file.extension"};

            WHEN("called by this function")
            {
                auto actual{
                    logency::detail::file::extract_file_extension(file)};

                THEN("get correct result")
                {
                    CHECK_EQ(actual,
                             std::make_tuple(
                                 "really/long/nested/directory/list/file",
                                 ".extension"));
                }
            }
        }

        GIVEN("one_dot_file.")
        {
            std::filesystem::path file{"one_dot_file."};

            WHEN("called by this function")
            {
                auto actual{
                    logency::detail::file::extract_file_extension(file)};

                THEN("get correct result")
                {
                    CHECK_EQ(actual, std::make_tuple("one_dot_file", "."));
                }
            }
        }

        GIVEN("directory/one_dot_file.")
        {
            std::filesystem::path file{"directory/one_dot_file."};

            WHEN("called by this function")
            {
                auto actual{
                    logency::detail::file::extract_file_extension(file)};

                THEN("get correct result")
                {
                    CHECK_EQ(actual,
                             std::make_tuple("directory/one_dot_file", "."));
                }
            }
        }

        GIVEN("really/long/nested/directory/one_dot_file.")
        {
            std::filesystem::path file{
                "really/long/nested/directory/one_dot_file."};

            WHEN("called by this function")
            {
                auto actual{
                    logency::detail::file::extract_file_extension(file)};

                THEN("get correct result")
                {
                    CHECK_EQ(
                        actual,
                        std::make_tuple(
                            "really/long/nested/directory/one_dot_file", "."));
                }
            }
        }

        GIVEN("current_directory/.")
        {
            std::filesystem::path file{"current_directory/."};

            WHEN("called by this function")
            {
                auto actual{
                    logency::detail::file::extract_file_extension(file)};

                THEN("get correct result")
                {
                    CHECK_EQ(actual,
                             std::make_tuple("current_directory/.", ""));
                }
            }
        }

        GIVEN("parent_directory/..")
        {
            std::filesystem::path file{"parent_directory/.."};

            WHEN("called by this function")
            {
                auto actual{
                    logency::detail::file::extract_file_extension(file)};

                THEN("get correct result")
                {
                    CHECK_EQ(actual,
                             std::make_tuple("parent_directory/..", ""));
                }
            }
        }

        GIVEN("directory/.hidden_file")
        {
            std::filesystem::path file{"directory/.hidden_file"};

            WHEN("called by this function")
            {
                auto actual{
                    logency::detail::file::extract_file_extension(file)};

                THEN("get correct result")
                {
                    CHECK_EQ(actual,
                             std::make_tuple("directory/.hidden_file", ""));
                }
            }
        }

        GIVEN("directory/.hidden_file.extension")
        {
            std::filesystem::path file{"directory/.hidden_file.extension"};

            WHEN("called by this function")
            {
                auto actual{
                    logency::detail::file::extract_file_extension(file)};

                THEN("get correct result")
                {
                    CHECK_EQ(actual, std::make_tuple("directory/.hidden_file",
                                                     ".extension"));
                }
            }
        }

        GIVEN("only.last.extension.are.extracted")
        {
            std::filesystem::path file{"only.last.extension.are.extracted"};

            WHEN("called by this function")
            {
                auto actual{
                    logency::detail::file::extract_file_extension(file)};

                THEN("get correct result")
                {
                    CHECK_EQ(actual, std::make_tuple("only.last.extension.are",
                                                     ".extracted"));
                }
            }
        }

        GIVEN("directory/only.last.extension.are.extracted")
        {
            std::filesystem::path file{
                "directory/only.last.extension.are.extracted"};

            WHEN("called by this function")
            {
                auto actual{
                    logency::detail::file::extract_file_extension(file)};

                THEN("get correct result")
                {
                    CHECK_EQ(actual, std::make_tuple(
                                         "directory/only.last.extension.are",
                                         ".extracted"));
                }
            }
        }

        GIVEN("only/directory/with/no/files")
        {
            std::filesystem::path file{"only/directory/with/no/files"};

            WHEN("called by this function")
            {
                auto actual{
                    logency::detail::file::extract_file_extension(file)};

                THEN("get correct result")
                {
                    CHECK_EQ(actual, std::make_tuple(
                                         "only/directory/with/no/files", ""));
                }
            }
        }

        GIVEN("directory/this.is.directory/file.extension")
        {
            std::filesystem::path file{
                "directory/this.is.directory/file.extension"};

            WHEN("called by this function")
            {
                auto actual{
                    logency::detail::file::extract_file_extension(file)};

                THEN("get correct result")
                {
                    CHECK_EQ(actual,
                             std::make_tuple("directory/this.is.directory/file",
                                             ".extension"));
                }
            }
        }

        GIVEN("directory/no.extension.are.extracted/directory")
        {
            std::filesystem::path file{
                "directory/no.extension.are.extracted/directory"};

            WHEN("called by this function")
            {
                auto actual{
                    logency::detail::file::extract_file_extension(file)};

                THEN("get correct result")
                {
                    CHECK_EQ(
                        actual,
                        std::make_tuple(
                            "directory/no.extension.are.extracted/directory",
                            ""));
                }
            }
        }
    }
}

} // namespace logency::unit_test::detail::file
