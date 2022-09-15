#include "logency/sink_module/rotation_file_module.hpp"

#include "file_directory.hpp"
#include "include_doctest.hpp"
#include "utils/check_exception.hpp"
#include "utils/file.hpp"
#include "utils/literals.hpp"
#include "utils/test_message.hpp"

#include <cstddef>
#include <filesystem>
#include <memory>
#include <string>

namespace logency::unit_test::sink_module
{

namespace
{

namespace constant
{

// NOLINTNEXTLINE(*-using-namespace) we need it for literals
using namespace unit_test::utils::literals;

constexpr const logency::sink_module::rotation_file::rotate_info rotate_info{
    16_byte, 3};

} // namespace constant

namespace content
{

template <typename value_type>
auto content(std::uintmax_t size = 1) -> std::basic_string<value_type>;

template <typename value_type>
auto empty() -> std::basic_string<value_type>;

template <>
auto content(std::uintmax_t size) -> std::basic_string<char>
{
    // NOLINTNEXTLINE(modernize-return-braced-init-list)
    return std::string(static_cast<typename std::string::size_type>(size), '0');
}

template <>
auto content(std::uintmax_t size) -> std::basic_string<wchar_t>
{
    // NOLINTNEXTLINE(modernize-return-braced-init-list)
    return std::wstring(static_cast<typename std::wstring::size_type>(size),
                        L'0');
}

template <typename value_type>
auto empty() -> std::basic_string<value_type>
{
    return std::basic_string<value_type>{};
}

} // namespace content

namespace file
{

auto archive_name(std::string_view filename, int index) -> std::string;

auto archive_name(std::string_view filename, int index) -> std::string
{
    using path_type = std::filesystem::path;

    const path_type path_name{filename};

    return path_type{path_name}
        .replace_filename(path_name.stem())
        .concat(std::string_view{"-"})
        .concat(std::to_string(index))
        .concat(path_name.extension().native())
        .string();
}

} // namespace file

} // namespace

TEST_SUITE("logency::sink_module::rotation_file_module")
{
    namespace filesystem = std::filesystem;

    template <typename value_type>
    using buffer_type = typename std::basic_string<value_type>;

    template <typename value_type>
    using module_type = logency::sink_module::rotation_file_module<
        utils::message<value_type>, utils::formatter<value_type>>;

    using rotate_info = logency::sink_module::rotation_file::rotate_info;
    using construct_mode = logency::sink_module::rotation_file::construct_mode;
    using path_type = filesystem::path;

    SCENARIO_TEMPLATE(
        "template <typename CharT> "
        "rotation_file_module<MessageType, Formatter>::"
        "rotation_file_module(const CharT *, rotate_info, construct_mode, "
        "formatter_type &&)",
        T, char, wchar_t)
    {
        GIVEN("not existed file")
        {
            std::unique_ptr<module_type<T>> sink_module{nullptr};

            std::string name{
                unique_file_name("rotation_file_module-c_string-new")};

            WHEN("instantiate sink module with construct_mode::append_previous")
            {
                CHECK_NOTHROW({
                    sink_module = std::make_unique<module_type<T>>(
                        name.c_str(), constant::rotate_info,
                        construct_mode::append_previous,
                        std::make_unique<utils::formatter<T>>());
                });

                THEN("new file is created and attached to the sink module "
                     "without making any archive file")
                {
                    sink_module.reset();

                    CHECK(filesystem::exists(name));
                    CHECK_EQ(utils::file::get_content<T, char>(name),
                             content::empty<T>());

                    CHECK(!filesystem::exists(file::archive_name(name, 1)));
                    CHECK(!filesystem::exists(file::archive_name(name, 2)));
                }
            }

            WHEN("instantiate sink module with construct_mode::create_new_file")
            {
                CHECK_NOTHROW({
                    sink_module = std::make_unique<module_type<T>>(
                        name.c_str(), constant::rotate_info,
                        construct_mode::create_new_file,
                        std::make_unique<utils::formatter<T>>());
                });

                THEN("new file is created and attached to the sink module "
                     "without making any archive file")
                {
                    sink_module.reset();

                    CHECK(filesystem::exists(name));
                    CHECK_EQ(utils::file::get_content<T, char>(name),
                             content::empty<T>());

                    CHECK(!filesystem::exists(file::archive_name(name, 1)));
                    CHECK(!filesystem::exists(file::archive_name(name, 2)));
                }
            }
        }

        GIVEN("existed file with not exceeded rotation value")
        {
            std::unique_ptr<module_type<T>> sink_module{nullptr};

            std::string name{unique_file_name(
                "rotation_file_module-c_string-exist_no_exceed")};

            const auto expect{content::content<T>(1U)};
            utils::file::set_content(name, expect);

            WHEN("instantiate sink module with construct_mode::append_previous")
            {
                CHECK_NOTHROW({
                    sink_module = std::make_unique<module_type<T>>(
                        name.c_str(), constant::rotate_info,
                        construct_mode::append_previous,
                        std::make_unique<utils::formatter<T>>());
                });

                THEN("original file is and attached to the sink module without "
                     "losing its content and no archive file is created")
                {
                    sink_module.reset();

                    CHECK(filesystem::exists(name));
                    CHECK_EQ(utils::file::get_content<T, char>(name), expect);

                    CHECK(!filesystem::exists(file::archive_name(name, 1)));
                    CHECK(!filesystem::exists(file::archive_name(name, 2)));
                }
            }

            WHEN("instantiate sink module with construct_mode::create_new_file")
            {
                CHECK_NOTHROW({
                    sink_module = std::make_unique<module_type<T>>(
                        name.c_str(), constant::rotate_info,
                        construct_mode::create_new_file,
                        std::make_unique<utils::formatter<T>>());
                });

                THEN("content of original file is rotated into archive file")
                {
                    sink_module.reset();

                    CHECK(filesystem::exists(name));
                    CHECK_EQ(utils::file::get_content<T, char>(name),
                             content::empty<T>());

                    const auto archive_file{file::archive_name(name, 1)};
                    CHECK(filesystem::exists(archive_file));
                    CHECK_EQ(utils::file::get_content<T, char>(archive_file),
                             expect);

                    CHECK(!filesystem::exists(file::archive_name(name, 2)));
                }
            }
        }

        GIVEN("existed file with exceeded rotation value")
        {
            std::unique_ptr<module_type<T>> sink_module{nullptr};

            std::string name{
                unique_file_name("rotation_file_module-c_string-exist_exceed")};

            const auto expect{
                content::content<T>(constant::rotate_info.file_size + 1)};
            utils::file::set_content(name, expect);

            WHEN("instantiate sink module with construct_mode::append_previous")
            {
                CHECK_NOTHROW({
                    sink_module = std::make_unique<module_type<T>>(
                        name.c_str(), constant::rotate_info,
                        construct_mode::append_previous,
                        std::make_unique<utils::formatter<T>>());
                });

                THEN("content of original file is rotated into archive file")
                {
                    sink_module.reset();

                    CHECK(filesystem::exists(name));
                    CHECK_EQ(utils::file::get_content<T, char>(name),
                             content::empty<T>());

                    const auto archive_file{file::archive_name(name, 1)};
                    CHECK(filesystem::exists(archive_file));
                    CHECK_EQ(utils::file::get_content<T, char>(archive_file),
                             expect);

                    CHECK(!filesystem::exists(file::archive_name(name, 2)));
                }
            }

            WHEN("instantiate sink module with construct_mode::create_new_file")
            {
                CHECK_NOTHROW({
                    sink_module = std::make_unique<module_type<T>>(
                        name.c_str(), constant::rotate_info,
                        construct_mode::create_new_file,
                        std::make_unique<utils::formatter<T>>());
                });

                THEN("content of original file is rotated into archive file")
                {
                    sink_module.reset();

                    CHECK(filesystem::exists(name));
                    CHECK_EQ(utils::file::get_content<T, char>(name),
                             content::empty<T>());

                    const auto archive_file{file::archive_name(name, 1)};
                    CHECK(filesystem::exists(archive_file));
                    CHECK_EQ(utils::file::get_content<T, char>(archive_file),
                             expect);

                    CHECK(!filesystem::exists(file::archive_name(name, 2)));
                }
            }
        }

        GIVEN("incorrect file name")
        {
            constexpr const char *name{""};

            WHEN("instantiate sink module with construct_mode::append_previous")
            {
                auto act{[&]
                         {
                             std::ignore = std::make_unique<module_type<T>>(
                                 name, constant::rotate_info,
                                 construct_mode::append_previous,
                                 std::make_unique<utils::formatter<T>>());
                         }};

                THEN("throw logency::system_error")
                {
                    utils::check::throw_start_with_as<logency::system_error>(
                        act, "Failed to open file");
                }
            }

            WHEN("instantiate sink module with construct_mode::create_new_file")
            {
                auto act{[&]
                         {
                             std::ignore = std::make_unique<module_type<T>>(
                                 name, constant::rotate_info,
                                 construct_mode::create_new_file,
                                 std::make_unique<utils::formatter<T>>());
                         }};

                THEN("throw logency::system_error")
                {
                    utils::check::throw_start_with_as<logency::system_error>(
                        act, "Failed to open file");
                }
            }
        }

        GIVEN("incorrect rotate_info::file_size")
        {
            const std::string name{
                unique_file_name("c_string-incorrect_rotate_info-file_size")};
            constexpr const rotate_info incorrect{0U, 3};

            WHEN("instantiate sink module with construct_mode::append_previous")
            {
                auto act{[&]
                         {
                             std::ignore = std::make_unique<module_type<T>>(
                                 name, incorrect,
                                 construct_mode::append_previous,
                                 std::make_unique<utils::formatter<T>>());
                         }};

                THEN("throw logency::runtime_error without creating file")
                {
                    CHECK_THROWS_WITH_AS(
                        act(), "file size should be smaller than zero.",
                        logency::runtime_error);

                    CHECK(!filesystem::exists(name));
                }
            }

            WHEN("instantiate sink module with construct_mode::create_new_file")
            {
                auto act{[&]
                         {
                             std::ignore = std::make_unique<module_type<T>>(
                                 name, incorrect,
                                 construct_mode::create_new_file,
                                 std::make_unique<utils::formatter<T>>());
                         }};

                THEN("throw logency::runtime_error without creating file")
                {
                    CHECK_THROWS_WITH_AS(
                        act(), "file size should be smaller than zero.",
                        logency::runtime_error);

                    CHECK(!filesystem::exists(name));
                }
            }
        }

        GIVEN("incorrect rotate_info::file_count")
        {
            const std::string name{
                unique_file_name("c_string-incorrect_rotate_info-file_count")};
            constexpr const rotate_info incorrect{16U, 0};

            WHEN("instantiate sink module with construct_mode::append_previous")
            {
                auto act{[&]
                         {
                             std::ignore = std::make_unique<module_type<T>>(
                                 name, incorrect,
                                 construct_mode::append_previous,
                                 std::make_unique<utils::formatter<T>>());
                         }};

                THEN("throw logency::runtime_error without creating file")
                {
                    CHECK_THROWS_WITH_AS(
                        act(), "file count should be positive integer.",
                        logency::runtime_error);

                    CHECK(!filesystem::exists(name));
                }
            }

            WHEN("instantiate sink module with construct_mode::create_new_file")
            {
                auto act{[&]
                         {
                             std::ignore = std::make_unique<module_type<T>>(
                                 name, incorrect,
                                 construct_mode::create_new_file,
                                 std::make_unique<utils::formatter<T>>());
                         }};

                THEN("throw logency::runtime_error without creating file")
                {
                    CHECK_THROWS_WITH_AS(
                        act(), "file count should be positive integer.",
                        logency::runtime_error);

                    CHECK(!filesystem::exists(name));
                }
            }
        }
    }

    SCENARIO_TEMPLATE("template <typename CharT> "
                      "rotation_file_module<MessageType, Formatter>::"
                      "rotation_file_module(const std::basic_string<CharT> &, "
                      "rotate_info, construct_mode, formatter_type &&)",
                      T, char, wchar_t)
    {
        GIVEN("not existed file")
        {
            std::unique_ptr<module_type<T>> sink_module{nullptr};

            std::string name{
                unique_file_name("rotation_file_module-c_string-new")};

            WHEN("instantiate sink module with construct_mode::append_previous")
            {
                CHECK_NOTHROW({
                    sink_module = std::make_unique<module_type<T>>(
                        name, constant::rotate_info,
                        construct_mode::append_previous,
                        std::make_unique<utils::formatter<T>>());
                });

                THEN("new file is created and attached to the sink module")
                {
                    sink_module.reset();

                    CHECK(filesystem::exists(name));
                    CHECK_EQ(utils::file::get_content<T, char>(name),
                             content::empty<T>());
                }
            }

            WHEN("instantiate sink module with construct_mode::create_new_file")
            {
                CHECK_NOTHROW({
                    sink_module = std::make_unique<module_type<T>>(
                        name, constant::rotate_info,
                        construct_mode::create_new_file,
                        std::make_unique<utils::formatter<T>>());
                });

                THEN("new file is created and attached to the sink module")
                {
                    sink_module.reset();

                    CHECK(filesystem::exists(name));
                    CHECK_EQ(utils::file::get_content<T, char>(name),
                             content::empty<T>());
                }
            }
        }

        GIVEN("existed file with not exceeded rotation value")
        {
            std::unique_ptr<module_type<T>> sink_module{nullptr};

            std::string name{unique_file_name(
                "rotation_file_module-c_string-exist_no_exceed")};

            const auto expect{content::content<T>(1U)};
            utils::file::set_content(name, expect);

            WHEN("instantiate sink module with construct_mode::append_previous")
            {
                CHECK_NOTHROW({
                    sink_module = std::make_unique<module_type<T>>(
                        name, constant::rotate_info,
                        construct_mode::append_previous,
                        std::make_unique<utils::formatter<T>>());
                });

                THEN("original file is created and attached to the sink module "
                     "without losing its content")
                {
                    sink_module.reset();

                    CHECK(filesystem::exists(name));
                    CHECK_EQ(utils::file::get_content<T, char>(name), expect);
                }
            }

            WHEN("instantiate sink module with construct_mode::create_new_file")
            {
                CHECK_NOTHROW({
                    sink_module = std::make_unique<module_type<T>>(
                        name, constant::rotate_info,
                        construct_mode::create_new_file,
                        std::make_unique<utils::formatter<T>>());
                });

                THEN("content of original file is rotated into archive file")
                {
                    sink_module.reset();

                    CHECK(filesystem::exists(name));
                    CHECK_EQ(utils::file::get_content<T, char>(name),
                             content::empty<T>());

                    const auto archive_file{file::archive_name(name, 1)};
                    CHECK(filesystem::exists(archive_file));
                    CHECK_EQ(utils::file::get_content<T, char>(archive_file),
                             expect);
                }
            }
        }

        GIVEN("existed file with exceeded rotation value")
        {
            std::unique_ptr<module_type<T>> sink_module{nullptr};

            std::string name{
                unique_file_name("rotation_file_module-c_string-exist_exceed")};

            const auto expect{
                content::content<T>(constant::rotate_info.file_size + 1)};
            utils::file::set_content(name, expect);

            WHEN("instantiate sink module with construct_mode::append_previous")
            {
                CHECK_NOTHROW({
                    sink_module = std::make_unique<module_type<T>>(
                        name, constant::rotate_info,
                        construct_mode::append_previous,
                        std::make_unique<utils::formatter<T>>());
                });

                THEN("content of original file is rotated into archive file")
                {
                    sink_module.reset();

                    CHECK(filesystem::exists(name));
                    CHECK_EQ(utils::file::get_content<T, char>(name),
                             content::empty<T>());

                    const auto archive_file{file::archive_name(name, 1)};
                    CHECK(filesystem::exists(archive_file));
                    CHECK_EQ(utils::file::get_content<T, char>(archive_file),
                             expect);
                }
            }

            WHEN("instantiate sink module with construct_mode::create_new_file")
            {
                CHECK_NOTHROW({
                    sink_module = std::make_unique<module_type<T>>(
                        name, constant::rotate_info,
                        construct_mode::create_new_file,
                        std::make_unique<utils::formatter<T>>());
                });

                THEN("content of original file is rotated into archive file")
                {
                    sink_module.reset();

                    CHECK(filesystem::exists(name));
                    CHECK_EQ(utils::file::get_content<T, char>(name),
                             content::empty<T>());

                    const auto archive_file{file::archive_name(name, 1)};
                    CHECK(filesystem::exists(archive_file));
                    CHECK_EQ(utils::file::get_content<T, char>(archive_file),
                             expect);
                }
            }
        }

        GIVEN("incorrect file name")
        {
            const std::string name;

            WHEN("instantiate sink module with construct_mode::append_previous")
            {
                auto act{[&]
                         {
                             std::ignore = std::make_unique<module_type<T>>(
                                 name, constant::rotate_info,
                                 construct_mode::append_previous,
                                 std::make_unique<utils::formatter<T>>());
                         }};

                THEN("throw logency::system_error")
                {
                    utils::check::throw_start_with_as<logency::system_error>(
                        act, "Failed to open file");
                }
            }

            WHEN("instantiate sink module with construct_mode::create_new_file")
            {
                auto act{[&]
                         {
                             std::ignore = std::make_unique<module_type<T>>(
                                 name, constant::rotate_info,
                                 construct_mode::create_new_file,
                                 std::make_unique<utils::formatter<T>>());
                         }};

                THEN("throw logency::system_error")
                {
                    utils::check::throw_start_with_as<logency::system_error>(
                        act, "Failed to open file");
                }
            }
        }

        GIVEN("incorrect rotate_info::file_size")
        {
            const std::string name{
                unique_file_name("std_string-incorrect_rotate_info-file_size")};
            constexpr const rotate_info incorrect{0U, 3};

            WHEN("instantiate sink module with construct_mode::append_previous")
            {
                auto act{[&]
                         {
                             std::ignore = std::make_unique<module_type<T>>(
                                 name, incorrect,
                                 construct_mode::append_previous,
                                 std::make_unique<utils::formatter<T>>());
                         }};

                THEN("throw logency::runtime_error without creating file")
                {
                    CHECK_THROWS_WITH_AS(
                        act(), "file size should be smaller than zero.",
                        logency::runtime_error);

                    CHECK(!filesystem::exists(name));
                }
            }

            WHEN("instantiate sink module with construct_mode::create_new_file")
            {
                auto act{[&]
                         {
                             std::ignore = std::make_unique<module_type<T>>(
                                 name, incorrect,
                                 construct_mode::create_new_file,
                                 std::make_unique<utils::formatter<T>>());
                         }};

                THEN("throw logency::runtime_error without creating file")
                {
                    CHECK_THROWS_WITH_AS(
                        act(), "file size should be smaller than zero.",
                        logency::runtime_error);

                    CHECK(!filesystem::exists(name));
                }
            }
        }

        GIVEN("incorrect rotate_info::file_count")
        {
            const std::string name{unique_file_name(
                "std_string-incorrect_rotate_info-file_count")};
            constexpr const rotate_info incorrect{16U, 0};

            WHEN("instantiate sink module with construct_mode::append_previous")
            {
                auto act{[&]
                         {
                             std::ignore = std::make_unique<module_type<T>>(
                                 name, incorrect,
                                 construct_mode::append_previous,
                                 std::make_unique<utils::formatter<T>>());
                         }};

                THEN("throw logency::runtime_error without creating file")
                {
                    CHECK_THROWS_WITH_AS(
                        act(), "file count should be positive integer.",
                        logency::runtime_error);

                    CHECK(!filesystem::exists(name));
                }
            }

            WHEN("instantiate sink module with construct_mode::create_new_file")
            {
                auto act{[&]
                         {
                             std::ignore = std::make_unique<module_type<T>>(
                                 name, incorrect,
                                 construct_mode::create_new_file,
                                 std::make_unique<utils::formatter<T>>());
                         }};

                THEN("throw logency::runtime_error without creating file")
                {
                    CHECK_THROWS_WITH_AS(
                        act(), "file count should be positive integer.",
                        logency::runtime_error);

                    CHECK(!filesystem::exists(name));
                }
            }
        }
    }

    SCENARIO_TEMPLATE(
        "void rotation_file_module<MessageType, Formatter>::flush()", T, char,
        wchar_t)
    {
        GIVEN("instantiated sink module with content written inside")
        {
            std::string name{unique_file_name("rotation_file_module-flush")};

            auto sink_module{std::make_unique<module_type<T>>(
                name, constant::rotate_info, construct_mode::create_new_file,
                std::make_unique<utils::formatter<T>>())};

            const auto expect{content::content<T>()};

            sink_module->log_message(
                utils::not_used<T>(),
                utils::message<T>{std::basic_string<T>{expect}});

            WHEN("flush the sink module")
            {
                CHECK_NOTHROW({ sink_module->flush(); });

                THEN("content is flushed into the file successfully")
                {
                    sink_module.reset();

                    CHECK_EQ(utils::file::get_content<T, char>(name), expect);
                }
            }
        }
    }

    SCENARIO_TEMPLATE("void rotation_file_module<MessageType, Formatter>::"
                      "log_message(std::string_view logger,  "
                      "const message_type &message)",
                      T, char, wchar_t)
    {
        constexpr const auto rotate_size{constant::rotate_info.file_size};

        if (rotate_size <= 1)
        {
            FAIL("rotate_size is not enough to proceed the test. "
                 "{rotate_size: ",
                 rotate_size, "}");
        }

        GIVEN("no archive file")
        {
            std::string name{unique_file_name(
                "rotation_file_module-log_message-no_archive_file")};

            constexpr const auto current_log_size{rotate_size / 2};
            const auto current_log{content::content<T>(current_log_size)};
            utils::file::set_content(name, current_log);

            auto sink_module{std::make_unique<module_type<T>>(
                name, constant::rotate_info, construct_mode::append_previous,
                std::make_unique<utils::formatter<T>>())};

            WHEN("log message is not exceed the limit")
            {
                constexpr const auto limit{rotate_size - current_log_size};
                constexpr const auto should_not_rotate{
                    static_cast<decltype(limit)>(1U)};

                if (should_not_rotate >= limit)
                {
                    FAIL("cannot allocate proper message to proceed the test. "
                         "{limit: ",
                         limit, ", actual: ", should_not_rotate, "}");
                }

                const auto message{content::content<T>(should_not_rotate)};
                CHECK_NOTHROW({
                    sink_module->log_message(
                        utils::not_used<T>(),
                        utils::message<T>{std::basic_string<T>{message}});
                });

                THEN("content is recorded into the file")
                {
                    sink_module.reset();

                    CHECK(filesystem::exists(name));
                    CHECK_EQ(utils::file::get_content<T, char>(name),
                             std::basic_string<T>{current_log}.append(message));

                    AND_THEN("without creating any unnecessary archive file")
                    {
                        CHECK(!filesystem::exists(file::archive_name(name, 1)));
                        CHECK(!filesystem::exists(file::archive_name(name, 2)));
                    }
                }
            }

            WHEN("log message is exceed the limit")
            {
                constexpr const auto limit{rotate_size - current_log_size};
                constexpr const auto should_rotate{limit + 1U};

                if (should_rotate >= rotate_size)
                {
                    FAIL("cannot allocate proper message to proceed the test. "
                         "{limit: ",
                         rotate_size, ", actual: ", should_rotate, "}");
                }

                const auto message{content::content<T>(should_rotate)};
                CHECK_NOTHROW({
                    sink_module->log_message(
                        utils::not_used<T>(),
                        utils::message<T>{std::basic_string<T>{message}});
                });

                THEN("new content is recorded into the original file")
                {
                    sink_module.reset();

                    CHECK(filesystem::exists(name));
                    CHECK_EQ(utils::file::get_content<T, char>(name), message);

                    AND_THEN("old content is rotated into the archive file")
                    {
                        const auto archive_file{file::archive_name(name, 1)};
                        CHECK(filesystem::exists(archive_file));
                        CHECK_EQ(
                            utils::file::get_content<T, char>(archive_file),
                            current_log);

                        AND_THEN(
                            "without creating any unnecessary archive file")
                        {
                            CHECK(!filesystem::exists(
                                file::archive_name(name, 2)));
                        }
                    }
                }
            }
        }

        GIVEN("exist archive file")
        {
            std::string name{unique_file_name(
                "rotation_file_module-log_message-exist_archive_file")};

            constexpr const auto current_log_size{rotate_size / 2};
            const auto current_log{content::content<T>(current_log_size)};
            utils::file::set_content(name, current_log);

            const auto archive_content{content::content<T>(rotate_size - 1)};
            utils::file::set_content(file::archive_name(name, 1),
                                     archive_content);

            auto sink_module{std::make_unique<module_type<T>>(
                name, constant::rotate_info, construct_mode::append_previous,
                std::make_unique<utils::formatter<T>>())};

            WHEN("log message is not exceed the limit")
            {
                constexpr const auto limit{rotate_size - current_log_size};
                constexpr const auto should_not_rotate{
                    static_cast<decltype(limit)>(1U)};

                if (should_not_rotate >= limit)
                {
                    FAIL("cannot allocate proper message to proceed the test. "
                         "{limit: ",
                         limit, ", actual: ", should_not_rotate, "}");
                }

                const auto message{content::content<T>(should_not_rotate)};
                CHECK_NOTHROW({
                    sink_module->log_message(
                        utils::not_used<T>(),
                        utils::message<T>{std::basic_string<T>{message}});
                });

                THEN("content is recorded into the file")
                {
                    sink_module.reset();

                    CHECK(filesystem::exists(name));
                    CHECK_EQ(utils::file::get_content<T, char>(name),
                             std::basic_string<T>{current_log}.append(message));

                    AND_THEN("without touching any archive file")
                    {
                        const auto archive_file{file::archive_name(name, 1)};
                        CHECK(filesystem::exists(archive_file));
                        CHECK_EQ(
                            utils::file::get_content<T, char>(archive_file),
                            archive_content);

                        CHECK(!filesystem::exists(file::archive_name(name, 2)));
                    }
                }
            }

            WHEN("log message is exceed the limit")
            {
                constexpr const auto limit{rotate_size - current_log_size};
                constexpr const auto should_rotate{limit + 1U};

                if (should_rotate >= rotate_size)
                {
                    FAIL("cannot allocate proper message to proceed the test. "
                         "{limit: ",
                         rotate_size, ", actual: ", should_rotate, "}");
                }

                const auto message{content::content<T>(should_rotate)};
                CHECK_NOTHROW({
                    sink_module->log_message(
                        utils::not_used<T>(),
                        utils::message<T>{std::basic_string<T>{message}});
                });

                THEN("the new content is recorded into the original file")
                {
                    sink_module.reset();

                    CHECK(filesystem::exists(name));
                    CHECK_EQ(utils::file::get_content<T, char>(name), message);

                    AND_THEN("old content is rotated into the archive file")
                    {
                        const auto archive_file_1{file::archive_name(name, 1)};
                        CHECK(filesystem::exists(archive_file_1));
                        CHECK_EQ(
                            utils::file::get_content<T, char>(archive_file_1),
                            current_log);

                        const auto archive_file_2{file::archive_name(name, 2)};
                        CHECK_EQ(
                            utils::file::get_content<T, char>(archive_file_2),
                            archive_content);
                    }
                }
            }
        }

        GIVEN("exist full archive file")
        {
            std::string name{unique_file_name(
                "rotation_file_module-log_message-exist_archive_file")};

            constexpr const auto current_log_size{rotate_size / 2};
            const auto current_log{content::content<T>(current_log_size)};
            utils::file::set_content(name, current_log);

            const auto archive_content{content::content<T>(rotate_size - 1)};
            utils::file::set_content(file::archive_name(name, 1),
                                     archive_content);
            utils::file::set_content(file::archive_name(name, 2),
                                     archive_content);

            auto sink_module{std::make_unique<module_type<T>>(
                name, constant::rotate_info, construct_mode::append_previous,
                std::make_unique<utils::formatter<T>>())};

            WHEN("log message is not exceed the limit")
            {
                constexpr const auto limit{rotate_size - current_log_size};
                constexpr const auto should_not_rotate{
                    static_cast<decltype(limit)>(1U)};

                if (should_not_rotate >= limit)
                {
                    FAIL("cannot allocate proper message to proceed the test. "
                         "{limit: ",
                         limit, ", actual: ", should_not_rotate, "}");
                }

                const auto message{content::content<T>(should_not_rotate)};
                CHECK_NOTHROW({
                    sink_module->log_message(
                        utils::not_used<T>(),
                        utils::message<T>{std::basic_string<T>{message}});
                });

                THEN("content is recorded into the file")
                {
                    sink_module.reset();

                    CHECK(filesystem::exists(name));
                    CHECK_EQ(utils::file::get_content<T, char>(name),
                             std::basic_string<T>{current_log}.append(message));

                    AND_THEN("without touching any archive file")
                    {
                        const auto archive_file_1{file::archive_name(name, 1)};
                        CHECK(filesystem::exists(archive_file_1));
                        CHECK_EQ(
                            utils::file::get_content<T, char>(archive_file_1),
                            archive_content);

                        const auto archive_file_2{file::archive_name(name, 1)};
                        CHECK(filesystem::exists(archive_file_2));
                        CHECK_EQ(
                            utils::file::get_content<T, char>(archive_file_2),
                            archive_content);
                    }
                }
            }

            WHEN("log message is exceed the limit")
            {
                constexpr const auto limit{rotate_size - current_log_size};
                constexpr const auto should_rotate{limit + 1U};

                if (should_rotate >= rotate_size)
                {
                    FAIL("cannot allocate proper message to proceed the test. "
                         "{limit: ",
                         rotate_size, ", actual: ", should_rotate, "}");
                }

                const auto message{content::content<T>(should_rotate)};
                CHECK_NOTHROW({
                    sink_module->log_message(
                        utils::not_used<T>(),
                        utils::message<T>{std::basic_string<T>{message}});
                });

                THEN("new content is recorded into the original file")
                {
                    sink_module.reset();

                    CHECK(filesystem::exists(name));
                    CHECK_EQ(utils::file::get_content<T, char>(name), message);

                    AND_THEN("old content is rotated into the archive file")
                    {
                        const auto archive_file_1{file::archive_name(name, 1)};
                        CHECK(filesystem::exists(archive_file_1));
                        CHECK_EQ(
                            utils::file::get_content<T, char>(archive_file_1),
                            current_log);

                        const auto archive_file_2{file::archive_name(name, 2)};
                        CHECK(filesystem::exists(archive_file_2));
                        CHECK_EQ(
                            utils::file::get_content<T, char>(archive_file_2),
                            archive_content);

                        AND_THEN("exceed archive file will be deleted")
                        {
                            CHECK(!filesystem::exists(
                                file::archive_name(name, 3)));
                        }
                    }
                }
            }
        }
    }
}

} // namespace logency::unit_test::sink_module
