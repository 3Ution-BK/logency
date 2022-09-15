#include "logency/detail/thread/thread_pool.hpp"
#include "logency/detail/thread/thread_unit_interface.hpp"

#include "include_doctest.hpp"

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <memory>
#include <thread>
#include <vcruntime.h>
#include <vector>

namespace logency::unit_test::detail::thread
{

namespace
{

namespace constant
{

const logency::detail::thread::thread_pool::size_type default_thread{1U};

} // namespace constant

class fake_thread_unit : public logency::detail::thread::thread_unit_interface
{
public:
    explicit fake_thread_unit(std::atomic<int> *ref) noexcept;

protected:
    void operate_by_thread() override;

private:
    std::atomic<int> *finished_;
};

fake_thread_unit::fake_thread_unit(std::atomic<int> *ref) noexcept
    : finished_(ref)
{
}

void fake_thread_unit::operate_by_thread()
{
    finished_->fetch_add(1, std::memory_order::memory_order_relaxed);
}

} // namespace

TEST_SUITE("logency::detail::thread::thread_pool")
{
    using thread_pool_type = logency::detail::thread::thread_pool;
    using size_type = thread_pool_type::size_type;

    SCENARIO("thread_pool::thread_pool(size_type)")
    {
        GIVEN("not instantiate object")
        {
            std::unique_ptr<thread_pool_type> pool{nullptr};

            WHEN("instantiate it with specified size")
            {
                constexpr const size_type expect{1U};

                CHECK_NOTHROW(
                    { pool = std::make_unique<thread_pool_type>(expect); });

                THEN("object is instantiated with specified pool size")
                {
                    CHECK(pool);

                    CHECK_EQ(pool->pool_size(), expect);
                }
            }
        }
    }

    SCENARIO("thread_pool::~thread_pool(); destruct; expect success")
    {
        GIVEN("instantiated pool")
        {
            auto pool{
                std::make_unique<thread_pool_type>(constant::default_thread)};

            WHEN("destruct")
            {
                CHECK_NOTHROW({ pool.reset(); });

                THEN("pool is destroyed") { CHECK(!pool); }
            }
        }
    }

    SCENARIO("auto thread_pool::pool_size() const noexcept -> size_type"
             "get pool size; "
             "expect success")
    {
        GIVEN("a pool with specified size")
        {
            constexpr const size_type expect{1U};
            auto pool{std::make_unique<thread_pool_type>(expect)};

            WHEN("get pool size")
            {
                thread_pool_type::size_type actual{1U};
                CHECK_NOTHROW({ actual = pool->pool_size(); });

                THEN("expect it have the same value")
                {
                    CHECK_EQ(actual, expect);
                }
            }
        }
    }

    SCENARIO("void thread_pool::wait_until_queue_empty()" *
             doctest::timeout(0.5))
    {
        GIVEN("a pool with queued task")
        {
            auto pool{
                std::make_unique<thread_pool_type>(constant::default_thread)};

            constexpr const int expect{65536};
            std::atomic<int> counter_{0};

            for (int index{0}; index < expect; ++index)
            {
                pool->enqueue(std::make_unique<fake_thread_unit>(&counter_));
            }

            WHEN("wait until thread queue is empty")
            {
                CHECK_NOTHROW({ pool->wait_until_queue_empty(); });

                THEN("expect all tasks which were enqueued before "
                     "this function are finished")
                {
                    CHECK_EQ(
                        counter_.load(std::memory_order::memory_order_relaxed),
                        expect);
                }
            }
        }
    }
}

} // namespace logency::unit_test::detail::thread
