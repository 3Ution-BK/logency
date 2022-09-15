#include "logency/detail/file/basic_file.hpp"

#include "file_directory.hpp"
#include "include_doctest.hpp"
#include "utils/check_exception.hpp"
#include "utils/file.hpp"

#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <tuple>
#include <typeinfo>

namespace logency::unit_test::detail::file
{

namespace
{

namespace content
{

template <typename value_type>
auto content() -> std::basic_string<value_type>;

template <typename value_type>
auto empty() -> std::basic_string<value_type>;

template <>
auto content() -> std::basic_string<char>
{
    return "content";
}

template <>
auto content() -> std::basic_string<wchar_t>
{
    return L"content";
}

template <typename value_type>
auto empty() -> std::basic_string<value_type>
{
    return std::basic_string<value_type>{};
}

} // namespace content

} // namespace

TEST_SUITE("logency::detail::file::basic_file")
{
    using path_type = std::filesystem::path;

    template <typename value_type>
    using buffer_type = typename std::basic_string<value_type>;
    template <typename value_type>
    using file_type = logency::detail::file::basic_file<value_type>;

    SCENARIO_TEMPLATE("basic_file<T>::basic_file(const char *, file_open_mode)",
                      T, char, wchar_t)
    {
        GIVEN("file is not existed")
        {
            std::unique_ptr<file_type<T>> file{nullptr};

            std::string name{unique_file_name("basic_file-c_string-new")};

            WHEN("instantiate with file_open_mode::append")
            {
                CHECK_NOTHROW({
                    file = std::make_unique<file_type<T>>(
                        name.c_str(), file_open_mode::append);
                });

                THEN("new file is created and attached the object")
                {
                    file.reset();

                    CHECK(std::filesystem::exists(name));
                    CHECK_EQ(utils::file::get_content<T, char>(name),
                             content::empty<T>());
                }
            }

            WHEN("instantiate with file_open_mode::truncate")
            {
                CHECK_NOTHROW({
                    file = std::make_unique<file_type<T>>(
                        name.c_str(), file_open_mode::truncate);
                });

                THEN("new file is created and attached the object")
                {
                    file.reset();

                    CHECK(std::filesystem::exists(name));
                    CHECK_EQ(utils::file::get_content<T, char>(name),
                             content::empty<T>());
                }
            }
        }

        GIVEN("file is existed")
        {
            std::unique_ptr<file_type<T>> file{nullptr};

            std::string name{unique_file_name("basic_file-c_string-exist")};
            utils::file::set_content(name, content::content<T>());

            WHEN("instantiate with file_open_mode::append")
            {
                CHECK_NOTHROW({
                    file = std::make_unique<file_type<T>>(
                        name.c_str(), file_open_mode::append);
                });

                THEN("exist file is attached the object without clearing its "
                     "content")
                {
                    file.reset();

                    CHECK(std::filesystem::exists(name));
                    CHECK_EQ(utils::file::get_content<T, char>(name),
                             content::content<T>());
                }
            }

            WHEN("instantiate with file_open_mode::truncate")
            {
                CHECK_NOTHROW({
                    file = std::make_unique<file_type<T>>(
                        name.c_str(), file_open_mode::truncate);
                });

                THEN("exist file is attached the object with empty content")
                {
                    file.reset();

                    CHECK(std::filesystem::exists(name));
                    CHECK_EQ(utils::file::get_content<T, char>(name),
                             content::empty<T>());
                }
            }
        }

        GIVEN("incorrect file name")
        {
            constexpr const char *name{""};

            WHEN("instantiate with file_open_mode::append")
            {
                auto act{[&]()
                         {
                             std::ignore = std::make_unique<file_type<T>>(
                                 name, file_open_mode::append);
                         }};

                THEN("throw logency::system_error")
                {
                    utils::check::throw_start_with_as<logency::system_error>(
                        act, "Failed to open file");
                }
            }

            WHEN("instantiate with file_open_mode::truncate")
            {
                auto act{[&]
                         {
                             std::ignore = std::make_unique<file_type<T>>(
                                 name, file_open_mode::truncate);
                         }};

                THEN("throw logency::system_error")
                {
                    utils::check::throw_start_with_as<logency::system_error>(
                        act, "Failed to open file");
                }
            }
        }
    }

    SCENARIO_TEMPLATE(
        "basic_file<T>::basic_file(const std::string &, file_open_mode)", T,
        char, wchar_t)
    {
        GIVEN("file is not existed")
        {
            std::unique_ptr<file_type<T>> file{nullptr};

            std::string name{unique_file_name("basic_file-std_string-new")};

            WHEN("instantiate with file_open_mode::append")
            {
                CHECK_NOTHROW({
                    file = std::make_unique<file_type<T>>(
                        name, file_open_mode::append);
                });

                THEN("new file is created and attached the object")
                {
                    file.reset();

                    CHECK(std::filesystem::exists(name));
                    CHECK_EQ(utils::file::get_content<T, char>(name),
                             content::empty<T>());
                }
            }

            WHEN("instantiate with file_open_mode::truncate")
            {
                CHECK_NOTHROW({
                    file = std::make_unique<file_type<T>>(
                        name, file_open_mode::truncate);
                });

                THEN("new file is created and attached the object")
                {
                    file.reset();

                    CHECK(std::filesystem::exists(name));
                    CHECK_EQ(utils::file::get_content<T, char>(name),
                             content::empty<T>());
                }
            }
        }

        GIVEN("file is existed")
        {
            std::unique_ptr<file_type<T>> file{nullptr};

            std::string name{unique_file_name("basic_file-std_string-exist")};
            utils::file::set_content(name, content::content<T>());

            WHEN("instantiate with file_open_mode::append")
            {
                CHECK_NOTHROW({
                    file = std::make_unique<file_type<T>>(
                        name, file_open_mode::append);
                });

                THEN("exist file is attached the object without clearing its "
                     "content")
                {
                    file.reset();

                    CHECK(std::filesystem::exists(name));
                    CHECK_EQ(utils::file::get_content<T, char>(name),
                             content::content<T>());
                }
            }

            WHEN("instantiate with file_open_mode::truncate")
            {
                CHECK_NOTHROW({
                    file = std::make_unique<file_type<T>>(
                        name, file_open_mode::truncate);
                });

                THEN("exist file is attached the object with empty content")
                {
                    file.reset();

                    CHECK(std::filesystem::exists(name));
                    CHECK_EQ(utils::file::get_content<T, char>(name),
                             content::empty<T>());
                }
            }
        }

        GIVEN("incorrect file name")
        {
            const std::string name;

            WHEN("instantiate with file_open_mode::append")
            {
                auto act{[&]
                         {
                             std::ignore = std::make_unique<file_type<T>>(
                                 name, file_open_mode::append);
                         }};

                THEN("throw logency::system_error")
                {
                    utils::check::throw_start_with_as<logency::system_error>(
                        act, "Failed to open file");
                }
            }

            WHEN("instantiate with file_open_mode::truncate")
            {
                auto act{[&]
                         {
                             std::ignore = std::make_unique<file_type<T>>(
                                 name, file_open_mode::truncate);
                         }};

                THEN("throw logency::system_error")
                {
                    utils::check::throw_start_with_as<logency::system_error>(
                        act, "Failed to open file");
                }
            }
        }
    }

    SCENARIO_TEMPLATE(
        "basic_file<T>::"
        "basic_file(const path_type::value_type *, file_open_mode)",
        T, char, wchar_t)
    {
        if constexpr (!std::is_same<path_type::value_type, char>::value)
        {
            GIVEN("file is not existed")
            {
                std::unique_ptr<file_type<T>> file{nullptr};

                std::string name{
                    unique_file_name("basic_file-path_c_string-new")};

                WHEN("instantiate with file_open_mode::append")
                {
                    CHECK_NOTHROW({
                        file = std::make_unique<file_type<T>>(
                            path_type{name}.c_str(), file_open_mode::append);
                    });

                    THEN("new file is created and attached the object")
                    {
                        file.reset();

                        CHECK(std::filesystem::exists(name));
                        CHECK_EQ(utils::file::get_content<T, char>(name),
                                 content::empty<T>());
                    }
                }

                WHEN("instantiate with file_open_mode::truncate")
                {
                    CHECK_NOTHROW({
                        file = std::make_unique<file_type<T>>(
                            path_type{name}.c_str(), file_open_mode::truncate);
                    });

                    THEN("new file is created and attached the object")
                    {
                        file.reset();

                        CHECK(std::filesystem::exists(name));
                        CHECK_EQ(utils::file::get_content<T, char>(name),
                                 content::empty<T>());
                    }
                }
            }

            GIVEN("file is existed")
            {
                std::unique_ptr<file_type<T>> file{nullptr};

                std::string name{
                    unique_file_name("basic_file-path_c_string-exist")};
                utils::file::set_content(name, content::content<T>());

                WHEN("instantiate with file_open_mode::append")
                {
                    CHECK_NOTHROW({
                        file = std::make_unique<file_type<T>>(
                            path_type{name}.c_str(), file_open_mode::append);
                    });

                    THEN("exist file is attached the object without clearing "
                         "its content")
                    {
                        file.reset();

                        CHECK(std::filesystem::exists(name));
                        CHECK_EQ(utils::file::get_content<T, char>(name),
                                 content::content<T>());
                    }
                }

                WHEN("instantiate with file_open_mode::truncate")
                {
                    CHECK_NOTHROW({
                        file = std::make_unique<file_type<T>>(
                            path_type{name}.c_str(), file_open_mode::truncate);
                    });

                    THEN("exist file is attached the object with empty content")
                    {
                        file.reset();

                        CHECK(std::filesystem::exists(name));
                        CHECK_EQ(utils::file::get_content<T, char>(name),
                                 content::empty<T>());
                    }
                }
            }

            GIVEN("incorrect file name")
            {
                constexpr const char *name{""};

                WHEN("instantiate with file_open_mode::append")
                {
                    auto act{[&]
                             {
                                 std::ignore = std::make_unique<file_type<T>>(
                                     path_type{name}.c_str(),
                                     file_open_mode::append);
                             }};

                    THEN("throw logency::system_error")
                    {
                        utils::check::throw_start_with_as<
                            logency::system_error>(act, "Failed to open file");
                    }
                }

                WHEN("instantiate with file_open_mode::truncate")
                {
                    auto act{[&]
                             {
                                 std::ignore = std::make_unique<file_type<T>>(
                                     path_type{name}.c_str(),
                                     file_open_mode::truncate);
                             }};

                    THEN("throw logency::system_error")
                    {
                        utils::check::throw_start_with_as<
                            logency::system_error>(act, "Failed to open file");
                    }
                }
            }
        }
    }

    SCENARIO_TEMPLATE(
        "basic_file<T>::basic_file("
        "const std::basic_string<path_type::value_type> &, file_open_mode)",
        T, char, wchar_t)
    {
        if constexpr (!std::is_same<path_type::value_type, char>::value)
        {
            GIVEN("file is not existed")
            {
                std::unique_ptr<file_type<T>> file{nullptr};

                std::string name{
                    unique_file_name("basic_file-path_native_string-exist")};

                WHEN("instantiate with file_open_mode::append")
                {
                    CHECK_NOTHROW({
                        file = std::make_unique<file_type<T>>(
                            path_type{name}.native(), file_open_mode::append);
                    });

                    THEN("new file is created and attached the object")
                    {
                        file.reset();

                        CHECK(std::filesystem::exists(name));
                        CHECK_EQ(utils::file::get_content<T, char>(name),
                                 content::empty<T>());
                    }
                }

                WHEN("instantiate with file_open_mode::truncate")
                {
                    CHECK_NOTHROW({
                        file = std::make_unique<file_type<T>>(
                            path_type{name}.native(), file_open_mode::truncate);
                    });

                    THEN("new file is created and attached the object")
                    {
                        file.reset();

                        CHECK(std::filesystem::exists(name));
                        CHECK_EQ(utils::file::get_content<T, char>(name),
                                 content::empty<T>());
                    }
                }
            }

            GIVEN("file is existed")
            {
                std::unique_ptr<file_type<T>> file{nullptr};

                std::string name{
                    unique_file_name("basic_file-path_native_string-new")};
                utils::file::set_content(name, content::content<T>());

                WHEN("instantiate with file_open_mode::append")
                {
                    CHECK_NOTHROW({
                        file = std::make_unique<file_type<T>>(
                            path_type{name}.native(), file_open_mode::append);
                    });

                    THEN("exist file is attached the object without clearing "
                         "its content")
                    {
                        file.reset();

                        CHECK(std::filesystem::exists(name));
                        CHECK_EQ(utils::file::get_content<T, char>(name),
                                 content::content<T>());
                    }
                }

                WHEN("instantiate with file_open_mode::truncate")
                {
                    CHECK_NOTHROW({
                        file = std::make_unique<file_type<T>>(
                            path_type{name}.native(), file_open_mode::truncate);
                    });

                    THEN("exist file is attached the object with empty content")
                    {
                        file.reset();

                        CHECK(std::filesystem::exists(name));
                        CHECK_EQ(utils::file::get_content<T, char>(name),
                                 content::empty<T>());
                    }
                }
            }

            GIVEN("incorrect file name")
            {
                constexpr const char *name{""};

                WHEN("instantiate with file_open_mode::append")
                {
                    auto act{[&]
                             {
                                 std::ignore = std::make_unique<file_type<T>>(
                                     path_type{name}.native(),
                                     file_open_mode::append);
                             }};

                    THEN("throw logency::system_error")
                    {
                        utils::check::throw_start_with_as<
                            logency::system_error>(act, "Failed to open file");
                    }
                }

                WHEN("instantiate with file_open_mode::truncate")
                {
                    auto act{[&]
                             {
                                 std::ignore = std::make_unique<file_type<T>>(
                                     path_type{name}.native(),
                                     file_open_mode::truncate);
                             }};

                    THEN("throw logency::system_error")
                    {
                        utils::check::throw_start_with_as<
                            logency::system_error>(act, "Failed to open file");
                    }
                }
            }
        }
    }

    SCENARIO_TEMPLATE(
        "basic_file<T>::basic_file(const path_type &, file_open_mode mode)", T,
        char, wchar_t)
    {
        GIVEN("file is not existed")
        {
            std::unique_ptr<file_type<T>> file{nullptr};

            std::string name{unique_file_name("basic_file-path-new")};

            WHEN("instantiate with file_open_mode::append")
            {
                CHECK_NOTHROW({
                    file = std::make_unique<file_type<T>>(
                        path_type{name}, file_open_mode::append);
                });

                THEN("new file is created and attached the object")
                {
                    file.reset();

                    CHECK(std::filesystem::exists(name));
                    CHECK_EQ(utils::file::get_content<T, char>(name),
                             content::empty<T>());
                }
            }

            WHEN("instantiate with file_open_mode::truncate")
            {
                CHECK_NOTHROW({
                    file = std::make_unique<file_type<T>>(
                        path_type{name}, file_open_mode::truncate);
                });

                THEN("new file is created and attached the object")
                {
                    file.reset();

                    CHECK(std::filesystem::exists(name));
                    CHECK_EQ(utils::file::get_content<T, char>(name),
                             content::empty<T>());
                }
            }
        }

        GIVEN("file is existed")
        {
            std::unique_ptr<file_type<T>> file{nullptr};

            std::string name{unique_file_name("basic_file-path-exist")};
            utils::file::set_content(name, content::content<T>());

            WHEN("instantiate with file_open_mode::append")
            {
                CHECK_NOTHROW({
                    file = std::make_unique<file_type<T>>(
                        path_type{name}, file_open_mode::append);
                });

                THEN("exist file is attached the object without clearing "
                     "its content")
                {
                    file.reset();

                    CHECK(std::filesystem::exists(name));
                    CHECK_EQ(utils::file::get_content<T, char>(name),
                             content::content<T>());
                }
            }

            WHEN("instantiate with file_open_mode::truncate")
            {
                CHECK_NOTHROW({
                    file = std::make_unique<file_type<T>>(
                        path_type{name}, file_open_mode::truncate);
                });

                THEN("exist file is attached the object with empty content")
                {
                    file.reset();

                    CHECK(std::filesystem::exists(name));
                    CHECK_EQ(utils::file::get_content<T, char>(name),
                             content::empty<T>());
                }
            }
        }

        GIVEN("incorrect file name")
        {
            constexpr const char *name{""};

            WHEN("instantiate with file_open_mode::append")
            {
                auto act{[&]
                         {
                             std::ignore = std::make_unique<file_type<T>>(
                                 path_type{name}, file_open_mode::append);
                         }};

                THEN("throw logency::system_error")
                {
                    utils::check::throw_start_with_as<logency::system_error>(
                        act, "Failed to open file");
                }
            }

            WHEN("instantiate with file_open_mode::truncate")
            {
                auto act{[&]
                         {
                             std::ignore = std::make_unique<file_type<T>>(
                                 path_type{name}, file_open_mode::truncate);
                         }};

                THEN("throw logency::system_error")
                {
                    utils::check::throw_start_with_as<logency::system_error>(
                        act, "Failed to open file");
                }
            }
        }
    }

    SCENARIO_TEMPLATE("basic_file<T>::~basic_file()", T, char, wchar_t)
    {
        GIVEN("instantiated file")
        {
            std::string name{test_dir().append("basic_file-destruct.txt")};

            auto file{
                std::make_unique<file_type<T>>(name, file_open_mode::truncate)};

            WHEN("destruct")
            {
                file.reset();

                THEN("successfully destruct the file object without delete the "
                     "actual file")
                {
                    CHECK(std::filesystem::exists(name));
                }
            }
        }
    }

    SCENARIO_TEMPLATE("void basic_file<T>::write(const buffer_type<T> &)", T,
                      char, wchar_t)
    {
        GIVEN("instantiated file object")
        {
            std::string name{test_dir().append("basic_file-write.txt")};
            auto file{
                std::make_unique<file_type<T>>(name, file_open_mode::truncate)};

            WHEN("write the content into the file")
            {
                CHECK_NOTHROW({ file->write(content::content<T>()); });

                THEN("content is written into the file successfully")
                {
                    file.reset();

                    CHECK_EQ(utils::file::get_content<T, char>(name),
                             content::content<T>());
                }
            }
        }
    }

    SCENARIO_TEMPLATE("void basic_file<T>::flush()", T, char, wchar_t)
    {
        GIVEN("instantiated file object with content written inside")
        {
            std::string name{test_dir().append("basic_file-flush.txt")};
            auto file{
                std::make_unique<file_type<T>>(name, file_open_mode::truncate)};
            file->write(content::content<T>());

            WHEN("flush the file object")
            {
                CHECK_NOTHROW({ file->flush(); });

                THEN("content is flushed into the file successfully")
                {
                    file.reset();

                    CHECK_EQ(utils::file::get_content<T, char>(name),
                             content::content<T>());
                }
            }
        }
    }
}

} // namespace logency::unit_test::detail::file
