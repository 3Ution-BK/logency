#include "logency/core/exception.hpp"

#include "include_doctest.hpp"
#include "utils/string.hpp"

#include <memory>
#include <ostream>
#include <string>

namespace logency::unit_test
{

TEST_SUITE("logency::exception::runtime_error")
{
    SCENARIO("runtime_error::runtime_error(const char *)")
    {
        GIVEN("not instantiate object")
        {
            std::unique_ptr<logency::runtime_error> object{nullptr};

            WHEN("instantiate")
            {
                CHECK_NOTHROW({
                    object = std::make_unique<logency::runtime_error>("what");
                });

                THEN("object is instantiated") { CHECK(object); }
            }
        }
    }

    SCENARIO("runtime_error::runtime_error(const std::string &)")
    {
        GIVEN("not instantiate object")
        {
            std::unique_ptr<logency::runtime_error> object{nullptr};

            WHEN("instantiate")
            {
                CHECK_NOTHROW({
                    object = std::make_unique<logency::runtime_error>(
                        std::string{"what"});
                });

                THEN("object is instantiated") { CHECK(object); }
            }
        }
    }

    SCENARIO("auto runtime_error::what() const -> const char *")
    {
        GIVEN("an object with specified message")
        {
            std::string expect{"what"};
            logency::runtime_error exception{expect};

            WHEN("call function")
            {
                std::string actual;

                CHECK_NOTHROW({ actual = exception.what(); });

                THEN("return same message") { CHECK_EQ(actual, expect); }
            }
        }
    }
}

TEST_SUITE("logency::exception::system_error")
{
    SCENARIO("system_error::system_error(std::error_code)")
    {
        GIVEN("not instantiate object")
        {
            std::unique_ptr<logency::system_error> object{nullptr};

            WHEN("instantiate")
            {
                CHECK_NOTHROW({
                    object = std::make_unique<logency::system_error>(
                        std::error_code{});
                });

                THEN("object is instantiated") { CHECK(object); }
            }
        }
    }

    SCENARIO("system_error::system_error(std::error_code, const char *)")
    {
        GIVEN("not instantiate object")
        {
            std::unique_ptr<logency::system_error> object{nullptr};

            WHEN("instantiate")
            {
                CHECK_NOTHROW({
                    object = std::make_unique<logency::system_error>(
                        std::error_code{}, "what");
                });

                THEN("object is instantiated") { CHECK(object); }
            }
        }
    }

    SCENARIO("system_error::system_error(std::error_code, const std::string &)")
    {
        GIVEN("not instantiate object")
        {
            std::unique_ptr<logency::system_error> object{nullptr};

            WHEN("instantiate")
            {
                using namespace std::string_literals;

                CHECK_NOTHROW({
                    object = std::make_unique<logency::system_error>(
                        std::error_code{}, "what"s);
                });

                THEN("object is instantiated") { CHECK(object); }
            }
        }
    }

    SCENARIO("auto system_error::what() const -> const char *")
    {
        GIVEN("an object with specified message")
        {
            std::string expect{"what"};
            logency::system_error exception{std::error_code{}, expect};

            WHEN("call function")
            {
                std::string actual;

                CHECK_NOTHROW({ actual = exception.what(); });

                THEN("return message start with same info")
                {
                    CHECK(utils::string::start_with(actual, expect));
                }
            }
        }
    }

    SCENARIO(
        "auto system_error::code() const noexcept -> const std::error_code &")
    {
        GIVEN("an object with specified std::error_code")
        {
            auto expect{std::error_code{}};
            logency::system_error exception{expect, "what"};

            WHEN("call function")
            {
                std::error_code actual{exception.code()};

                THEN("return same code") { CHECK_EQ(actual, expect); }
            }
        }
    }
}

} // namespace logency::unit_test
