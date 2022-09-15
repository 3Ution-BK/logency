#include "logency/sink_module/basic_file_module.hpp"

#include "file_directory.hpp"
#include "include_doctest.hpp"
#include "utils/check_exception.hpp"
#include "utils/file.hpp"
#include "utils/test_message.hpp"

#include <memory>
#include <string>
#include <tuple>

namespace logency::unit_test::sink_module
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

TEST_SUITE("logency::sink_module::basic_file_module")
{
    template <typename value_type>
    using buffer_type = typename std::basic_string<value_type>;

    template <typename value_type>
    using module_type =
        logency::sink_module::basic_file_module<utils::message<value_type>,
                                                utils::formatter<value_type>>;

    SCENARIO_TEMPLATE(
        "template <typename CharT> "
        "basic_file_module<MessageType, Formatter>::"
        "basic_file_module(const CharT *, file_open_mode, formatter_type &&)",
        T, char, wchar_t)
    {
        GIVEN("file is not existed")
        {
            std::unique_ptr<module_type<T>> sink_module{nullptr};

            std::string name{
                unique_file_name("basic_file_module-c_string-new")};

            WHEN("instantiate with file_open_mode::append")
            {
                CHECK_NOTHROW({
                    sink_module = std::make_unique<module_type<T>>(
                        name.c_str(), file_open_mode::append,
                        std::make_unique<utils::formatter<T>>());
                });

                THEN("new file is created and attached the object")
                {
                    sink_module.reset();

                    CHECK(std::filesystem::exists(name));
                    CHECK_EQ(utils::file::get_content<T, char>(name),
                             content::empty<T>());
                }
            }

            WHEN("instantiate with file_open_mode::truncate")
            {
                CHECK_NOTHROW({
                    sink_module = std::make_unique<module_type<T>>(
                        name.c_str(), file_open_mode::truncate,
                        std::make_unique<utils::formatter<T>>());
                });

                THEN("new file is created and attached the object")
                {
                    sink_module.reset();

                    CHECK(std::filesystem::exists(name));
                    CHECK_EQ(utils::file::get_content<T, char>(name),
                             content::empty<T>());
                }
            }
        }

        GIVEN("file is existed")
        {
            std::unique_ptr<module_type<T>> sink_module{nullptr};

            std::string name{
                unique_file_name("basic_file_module-c_string-exist")};

            utils::file::set_content(name, content::content<T>());

            WHEN("instantiate with file_open_mode::append")
            {
                CHECK_NOTHROW({
                    sink_module = std::make_unique<module_type<T>>(
                        name.c_str(), file_open_mode::append,
                        std::make_unique<utils::formatter<T>>());
                });

                THEN("exist file is attached the object without clearing its "
                     "content")
                {
                    sink_module.reset();

                    CHECK(std::filesystem::exists(name));
                    CHECK_EQ(utils::file::get_content<T, char>(name),
                             content::content<T>());
                }
            }

            WHEN("instantiate with file_open_mode::truncate")
            {
                CHECK_NOTHROW({
                    sink_module = std::make_unique<module_type<T>>(
                        name.c_str(), file_open_mode::truncate,
                        std::make_unique<utils::formatter<T>>());
                });

                THEN("exist file is attached the object with empty content")
                {
                    sink_module.reset();

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
                             std::ignore = std::make_unique<module_type<T>>(
                                 name, file_open_mode::append,
                                 std::make_unique<utils::formatter<T>>());
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
                             std::ignore = std::make_unique<module_type<T>>(
                                 name, file_open_mode::truncate,
                                 std::make_unique<utils::formatter<T>>());
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
        "template <typename CharT> "
        "basic_file_module<MessageType, Formatter>::"
        "basic_file_module(const std::basic_string<CharT> &, file_open_mode, "
        "formatter_type &&)",
        T, char, wchar_t)
    {
        GIVEN("file is not existed")
        {
            std::unique_ptr<module_type<T>> sink_module{nullptr};

            std::string name{
                unique_file_name("basic_file_module-c_string-new")};

            WHEN("instantiate with file_open_mode::append")
            {
                CHECK_NOTHROW({
                    sink_module = std::make_unique<module_type<T>>(
                        name, file_open_mode::append,
                        std::make_unique<utils::formatter<T>>());
                });

                THEN("new file is created and attached the object")
                {
                    sink_module.reset();

                    CHECK(std::filesystem::exists(name));
                    CHECK_EQ(utils::file::get_content<T, char>(name),
                             content::empty<T>());
                }
            }

            WHEN("instantiate with file_open_mode::truncate")
            {
                CHECK_NOTHROW({
                    sink_module = std::make_unique<module_type<T>>(
                        name, file_open_mode::truncate,
                        std::make_unique<utils::formatter<T>>());
                });

                THEN("new file is created and attached the object")
                {
                    sink_module.reset();

                    CHECK(std::filesystem::exists(name));
                    CHECK_EQ(utils::file::get_content<T, char>(name),
                             content::empty<T>());
                }
            }
        }

        GIVEN("file is existed")
        {
            std::unique_ptr<module_type<T>> sink_module{nullptr};

            std::string name{
                unique_file_name("basic_file_module-c_string-exist")};

            utils::file::set_content(name, content::content<T>());

            WHEN("instantiate with file_open_mode::append")
            {
                CHECK_NOTHROW({
                    sink_module = std::make_unique<module_type<T>>(
                        name, file_open_mode::append,
                        std::make_unique<utils::formatter<T>>());
                });

                THEN("exist file is attached the object without clearing its "
                     "content")
                {
                    sink_module.reset();

                    CHECK(std::filesystem::exists(name));
                    CHECK_EQ(utils::file::get_content<T, char>(name),
                             content::content<T>());
                }
            }

            WHEN("instantiate with file_open_mode::truncate")
            {
                CHECK_NOTHROW({
                    sink_module = std::make_unique<module_type<T>>(
                        name, file_open_mode::truncate,
                        std::make_unique<utils::formatter<T>>());
                });

                THEN("exist file is attached the object with empty content")
                {
                    sink_module.reset();

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
                             std::ignore = std::make_unique<module_type<T>>(
                                 name, file_open_mode::append,
                                 std::make_unique<utils::formatter<T>>());
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
                             std::ignore = std::make_unique<module_type<T>>(
                                 name, file_open_mode::truncate,
                                 std::make_unique<utils::formatter<T>>());
                         }};

                THEN("throw logency::system_error")
                {
                    utils::check::throw_start_with_as<logency::system_error>(
                        act, "Failed to open file");
                }
            }
        }
    }

    SCENARIO_TEMPLATE("basic_file_module<MessageType, LevelType, "
                      "Formatter>::~basic_file_module()",
                      T, char, wchar_t)
    {
        GIVEN("instantiated sink")
        {
            std::string name{unique_file_name("basic_file_module-destruct")};

            auto sink_module{std::make_unique<module_type<T>>(
                name, file_open_mode::truncate,
                std::make_unique<utils::formatter<T>>())};

            WHEN("destruct")
            {
                sink_module.reset();

                THEN("successfully destruct the file object without delete the "
                     "actual file")
                {
                    CHECK(std::filesystem::exists(name));
                }
            }
        }
    }

    SCENARIO_TEMPLATE("void basic_file_module<MessageType, Formatter>::flush()",
                      T, char, wchar_t)
    {
        GIVEN("instantiated sink module with content written inside")
        {
            std::string name{unique_file_name("basic_file_module-flush")};

            auto sink_module{std::make_unique<module_type<T>>(
                name, file_open_mode::truncate,
                std::make_unique<utils::formatter<T>>())};

            sink_module->log_message(utils::not_used<T>(),
                                     utils::message<T>{content::content<T>()});

            WHEN("flush the sink module")
            {
                CHECK_NOTHROW({ sink_module->flush(); });

                THEN("content is flushed into the file successfully")
                {
                    sink_module.reset();

                    CHECK_EQ(utils::file::get_content<T, char>(name),
                             content::content<T>());
                }
            }
        }
    }

    SCENARIO_TEMPLATE("void basic_file_module<MessageType, Formatter>::"
                      "log_message(std::string_view logger, "
                      "const message_type &message)",
                      T, char, wchar_t)
    {
        GIVEN("instantiated sink module")
        {
            std::string name{unique_file_name("basic_file-log_message")};

            auto sink_module{std::make_unique<module_type<T>>(
                name, file_open_mode::truncate,
                std::make_unique<utils::formatter<T>>())};

            WHEN("write the content into the sink module")
            {
                CHECK_NOTHROW({
                    sink_module->log_message(
                        utils::not_used<T>(),
                        utils::message<T>{content::content<T>()});
                });

                THEN("content is written into the sink module successfully")
                {
                    sink_module.reset();

                    CHECK_EQ(utils::file::get_content<T, char>(name),
                             content::content<T>());
                }
            }
        }
    }
}

} // namespace logency::unit_test::sink_module
