#include "logency/sink_module/ostream_module.hpp"

#include "include_doctest.hpp"
#include "logency/core/exception.hpp"
#include "utils/test_message.hpp"

#include <memory>
#include <sstream>
#include <tuple>
#include <vector>

namespace logency::unit_test::sink_module
{

namespace
{

namespace content
{

template <typename value_type>
auto pangram() -> std::basic_string<value_type>;

template <>
auto pangram() -> std::basic_string<char>
{
    return "The quick brown fox jumps over the lazy dog";
}

template <>
auto pangram() -> std::basic_string<wchar_t>
{
    return L"The quick brown fox jumps over the lazy dog";
}

} // namespace content

} // namespace

TEST_SUITE("logency::sink_module::ostream_module")
{
    template <typename value_type>
    using buffer_type = typename std::basic_string<value_type>;

    template <typename value_type>
    using module_type =
        logency::sink_module::ostream_module<utils::message<value_type>,
                                             utils::formatter<value_type>>;

    SCENARIO_TEMPLATE(
        "ostream_module<MessageType, Formatter>::"
        "ostream_module(ostream_type *stream, formatter_type &&formatter)",
        T, char, wchar_t)
    {
        GIVEN("valid ostream object")
        {
            std::basic_stringstream<T> stream;

            WHEN("instantiate with it")
            {
                std::unique_ptr<module_type<T>> sink_module{nullptr};

                CHECK_NOTHROW({
                    sink_module = std::make_unique<module_type<T>>(
                        &stream, std::make_unique<utils::formatter<T>>());
                });

                THEN("object is instantiated") { CHECK(sink_module); }
            }
        }

        GIVEN("invalid ostream object")
        {
            WHEN("instantiate with it")
            {
                auto act{[&]
                         {
                             std::ignore = std::make_unique<module_type<T>>(
                                 nullptr,
                                 std::make_unique<utils::formatter<T>>());
                         }};

                THEN("expect throw logency::runtime_error")
                {
                    CHECK_THROWS_WITH_AS(act(), "stream is nullptr.",
                                         logency::runtime_error);
                }
            }
        }
    }

    SCENARIO_TEMPLATE(
        "ostream_module<MessageType, Formatter>::~ostream_module()", T, char,
        wchar_t)
    {
        GIVEN("instantiated object")
        {
            std::basic_stringstream<T> stream;
            auto sink_module{std::make_unique<module_type<T>>(
                &stream, std::make_unique<utils::formatter<T>>())};

            WHEN("destruct")
            {
                CHECK_NOTHROW({ sink_module.reset(); });

                THEN("object is destroyed") { CHECK(!sink_module); }
            }
        }
    }

    SCENARIO_TEMPLATE("void ostream_module<MessageType, Formatter>::flush()", T,
                      char, wchar_t)
    {
        GIVEN("instantiated sink with content written inside")
        {
            std::basic_stringstream<T> stream;
            stream << content::pangram<T>();

            auto sink_module{std::make_unique<module_type<T>>(
                &stream, std::make_unique<utils::formatter<T>>())};

            WHEN("flush the sink")
            {
                CHECK_NOTHROW({ sink_module->flush(); });

                THEN("content is flushed into the stream successfully")
                {
                    CHECK_EQ(stream.str(), content::pangram<T>());
                }
            }
        }
    }

    SCENARIO_TEMPLATE("void ostream_module<MessageType, Formatter>::"
                      "log_message(std::string_view logger, "
                      "const message_type &message)",
                      T, char, wchar_t)
    {
        GIVEN("instantiated sink module")
        {
            std::basic_stringstream<T> stream;
            auto sink_module{std::make_unique<module_type<T>>(
                &stream, std::make_unique<utils::formatter<T>>())};

            WHEN("write the content into the sink module")
            {
                CHECK_NOTHROW({
                    sink_module->log_message(
                        utils::not_used<T>(),
                        utils::message<T>{content::pangram<T>()});
                });

                THEN("content is written into the sink module successfully")
                {
                    CHECK_EQ(stream.str(), content::pangram<T>());
                }
            }
        }
    }
}

} // namespace logency::unit_test::sink_module
