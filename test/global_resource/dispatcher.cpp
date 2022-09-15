#include "dispatcher.hpp"

#include "logency/detail/thread/thread_pool.hpp"
#include "thread_pool.hpp"

#include <iostream>
#include <memory>

namespace logency::unit_test::global_resource::dispatcher
{

auto invalid_thread_pool()
    -> std::shared_ptr<logency::dispatcher<utils::message<char>>>
{
    using message_type = utils::message<char>;
    using dispatcher_type = logency::dispatcher<message_type>;
    using thread_pool_type = logency::detail::thread::thread_pool;

    try
    {
        static auto object{std::make_shared<dispatcher_type>(
            std::weak_ptr<thread_pool_type>())};
        return object;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what();
        return nullptr;
    }
}

auto normal() -> std::shared_ptr<logency::dispatcher<utils::message<char>>>
{
    using message_type = utils::message<char>;
    using dispatcher_type = logency::dispatcher<message_type>;

    try
    {
        static auto object{std::make_shared<dispatcher_type>(
            global_resource::thread_pool::normal())};
        return object;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what();
        return nullptr;
    }
}

} // namespace logency::unit_test::global_resource::dispatcher
