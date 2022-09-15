#include "file_directory.hpp"

#include "logency/detail/string/string.hpp"

#include <cstring>

#include <atomic>
#include <string>

namespace logency::unit_test
{

auto test_dir() -> std::string { return "unit_test_file/"; }

auto unique_file_name(const char *filename) -> std::string
{
    constexpr const char *id_prefix{"-"};
    constexpr const char *id_postfix{".txt"};

    static std::atomic<int> counter_id{0};
    const int unique_id{counter_id.fetch_add(1, std::memory_order_relaxed)};
    const auto unique_id_string{std::to_string(unique_id)};

    return detail::string::concat(test_dir(), filename, id_prefix,
                                  unique_id_string, id_postfix);
}

} // namespace logency::unit_test
