#ifndef LOGENCY_TEST_INCLUDE_DOCTEST_HPP_
#define LOGENCY_TEST_INCLUDE_DOCTEST_HPP_

#include "doctest/doctest.h"

#include <filesystem>
#include <sstream>
#include <tuple>

template <>
struct doctest::StringMaker<
    std::tuple<std::filesystem::path, std::filesystem::path>>
{
    static String convert(
        const std::tuple<std::filesystem::path, std::filesystem::path> &value)
    {
        /*
         * Not used a lot(not time critical).
         * std::ostringstream should do the tricks easily.
         */
        std::ostringstream stream;

        stream << "std::tuple{" << std::get<0>(value) << ", "
               << std::get<1>(value) << "}";

        return stream.str().c_str();
    }
};

template <>
struct doctest::StringMaker<std::wstring>
{
    /*
     * Not used a lot(not time critical).
     * std::string.append() should do the tricks easily.
     */
    static std::string to_string(const std::wstring &value)
    {
        std::string output{"std::wstring{"};

        if (!value.empty())
        {
            /**
             * !!! Use it with caution. You have been warned.
             *
             * This is *NOT* the general way to convert wstring to string.
             *
             * We use it because our test case string contains ascii character
             * only.
             */
            output.append(std::begin(value), std::end(value));
        }

        output.append("}");

        return output;
    }

    static String convert(const std::wstring &value)
    {
        return String{to_string(value).c_str()};
    }
};

#endif // LOGENCY_TEST_INCLUDE_DOCTEST_HPP_
