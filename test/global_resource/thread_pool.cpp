#include "thread_pool.hpp"

#include <iostream>

namespace logency::unit_test::global_resource::thread_pool
{

auto normal() -> std::shared_ptr<logency::detail::thread::thread_pool>
{
    try
    {
        static auto pool{
            std::make_shared<logency::detail::thread::thread_pool>(1)};
        return pool;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what();
        return nullptr;
    }
}

} // namespace logency::unit_test::global_resource::thread_pool
