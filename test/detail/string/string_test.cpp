#include "logency/detail/string/string.hpp"

#include "include_doctest.hpp"

#include <functional>
#include <string>

namespace logency::unit_test::detail::string
{

TEST_SUITE("logency::detail::string")
{
    SCENARIO("template <typename... Args> "
             "auto concat(std::string_view arg, Args &&...args) -> std::string")
    {
        GIVEN("const char *, const char *")
        {
            const std::string expect{"etaoin shrdlu"};

            const char *lhs{"etaoin "};
            const char *rhs{"shrdlu"};

            WHEN("concatenate together")
            {
                std::string actual;

                CHECK_NOTHROW(
                    { actual = logency::detail::string::concat(lhs, rhs); });

                THEN("get correct result") { CHECK_EQ(actual, expect); }
            }
        }

        GIVEN("std::string, std::string")
        {
            const std::string expect{"etaoin shrdlu"};

            const std::string lhs{"etaoin "};
            const std::string rhs{"shrdlu"};

            WHEN("concatenate together")
            {
                std::string actual;

                CHECK_NOTHROW(
                    { actual = logency::detail::string::concat(lhs, rhs); });

                THEN("get correct result") { CHECK_EQ(actual, expect); }
            }
        }

        GIVEN("const char *, std::string")
        {
            const std::string expect{"etaoin shrdlu"};

            const char *lhs{"etaoin "};
            const std::string rhs{"shrdlu"};

            WHEN("concatenate together")
            {
                std::string actual;

                CHECK_NOTHROW(
                    { actual = logency::detail::string::concat(lhs, rhs); });

                THEN("get correct result") { CHECK_EQ(actual, expect); }
            }
        }

        GIVEN("std::string, const char *")
        {
            const std::string expect{"etaoin shrdlu"};

            const std::string lhs{"etaoin "};
            const char *rhs{"shrdlu"};

            WHEN("concatenate together")
            {
                std::string actual;

                CHECK_NOTHROW(
                    { actual = logency::detail::string::concat(lhs, rhs); });

                THEN("get correct result") { CHECK_EQ(actual, expect); }
            }
        }

        GIVEN("multiple strings")
        {
            const std::string expect{"Lorem ipsum dolor sit amet"};

            WHEN("concatenate together")
            {
                std::string actual;

                CHECK_NOTHROW({
                    actual = logency::detail::string::concat(
                        "Lorem ", "ipsum ", "dolor ", "sit ", "amet");
                });

                THEN("get correct result") { CHECK_EQ(actual, expect); }
            }
        }
    }

    SCENARIO(
        "auto concat_list(std::initializer_list<std::string_view> arguments)"
        "-> std::string")
    {
        GIVEN("Lorem ipsum dolor sit amet")
        {
            std::string expect{"Lorem ipsum dolor sit amet"};

            WHEN("concatenate together")
            {
                std::string actual;

                CHECK_NOTHROW({
                    actual = logency::detail::string::concat_list(
                        {"Lorem ", "ipsum ", "dolor ", "sit ", "amet"});
                });

                THEN("get correct result") { CHECK_EQ(actual, expect); }
            }
        }
    }
}

} // namespace logency::unit_test::detail::string
