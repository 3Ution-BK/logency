#include "logency/detail/thread/blocking_pair_queue.hpp"

#include "include_doctest.hpp"

#include <atomic>
#include <chrono>
#include <cstddef>
#include <future>
#include <tuple>
#include <vcruntime.h>
#include <vector>

namespace logency::unit_test::detail::thread
{

namespace
{

template <typename T, typename U>
auto set_queue(logency::detail::thread::blocking_pair_queue<T, U> &queue,
               std::size_t count) -> std::size_t;

template <typename T, typename U>
auto set_queue(logency::detail::thread::blocking_pair_queue<T, U> &queue,
               std::size_t count, std::size_t capacity) -> std::size_t;

template <typename T, typename U>
auto set_queue(logency::detail::thread::blocking_pair_queue<T, U> &queue,
               std::size_t count) -> std::size_t
{
    return set_queue(queue, count, count);
}

template <typename T, typename U>
auto set_queue(logency::detail::thread::blocking_pair_queue<T, U> &queue,
               // NOLINTNEXTLINE(*-easily-swappable-parameters)
               std::size_t count, std::size_t capacity) -> std::size_t
{
    // enqueue one dummy to make try_swap_bulk() works.
    std::ignore = queue.enqueue(T{}, U{});

    std::vector<T> actual_first_container(count, T{});
    std::vector<U> actual_second_container(count, U{});

    actual_first_container.reserve(capacity);
    actual_second_container.reserve(capacity);

    std::size_t actual_capacity{actual_first_container.capacity()};

    std::ignore =
        queue.try_swap_bulk(actual_first_container, actual_second_container);

    return actual_capacity;
}

class threading_fixture
{
public:
    struct producer_argument
    {
        int producer;
        int messages_per_producer;
    };

    using first_message_type = int;
    using second_message_type = int;

    template <typename T>
    using container_type = std::vector<T>;

    template <class Mutex>
    using lock_type = std::unique_lock<Mutex>;
    using mutex_type = std::mutex;
    using condition_variable_type = std::condition_variable;

    using queue_type =
        logency::detail::thread::blocking_pair_queue<first_message_type,
                                                     second_message_type>;

    explicit threading_fixture();
    ~threading_fixture();

    threading_fixture(const threading_fixture &other) = delete;
    threading_fixture(threading_fixture &&other) noexcept = delete;
    auto operator=(const threading_fixture &other)
        -> threading_fixture & = delete;
    auto operator=(threading_fixture &&other) noexcept
        -> threading_fixture & = delete;

    void set_producer(int how_many) noexcept;
    void set_consumer(int how_many) noexcept;
    void set_messages_per_producer(int how_many) noexcept;

    void act();

    int count() const noexcept;

protected:
    using futures_type = std::vector<std::future<void>>;

    auto create_producer() -> std::vector<std::future<void>>;
    auto create_consumer() -> std::vector<std::future<void>>;

private:
    void producer_procedure(int messages_per_producer);
    void consumer_procedure();

    bool is_produce_finish() const noexcept;

    std::atomic<int> count_{0};
    queue_type queue_{};

    mutex_type wait_mutex_{};
    condition_variable_type wait_{};

    int producer_{0};
    int consumer_{0};
    int messages_per_producer_{0};

    std::atomic<bool> is_produce_finish_{false};
};

threading_fixture::threading_fixture() = default;

threading_fixture::~threading_fixture() = default;

void threading_fixture::act()
{
    auto producer_future{create_producer()};
    auto consumer_future{create_consumer()};

    for (auto &future : producer_future)
    {
        future.wait();
    }

    is_produce_finish_.store(true, std::memory_order::memory_order_relaxed);
    wait_.notify_all();

    for (auto &future : consumer_future)
    {
        future.wait();
    }
}

void threading_fixture::consumer_procedure()
{
    for (;;)
    {
        container_type<first_message_type> first_tray;
        container_type<second_message_type> second_tray;

        if (queue_.try_swap_bulk(first_tray, second_tray))
        {
            count_.fetch_add(static_cast<int>(first_tray.size()));
        }
        else if (is_produce_finish())
        {
            break;
        }

        {
            lock_type<mutex_type> lock{wait_mutex_};
            wait_.wait_for(lock, std::chrono::milliseconds(1),
                           [&] { return is_produce_finish(); });
        }
    }
}

auto threading_fixture::create_producer() -> std::vector<std::future<void>>
{
    futures_type future;
    future.reserve(static_cast<typename futures_type::size_type>(producer_));

    for (int iter{0}; iter < producer_; ++iter)
    {
        future.push_back(std::async(std::launch::async,
                                    &threading_fixture::producer_procedure,
                                    this, messages_per_producer_));
    }

    return future;
}

int threading_fixture::count() const noexcept
{
    return count_.load(std::memory_order::memory_order_relaxed);
}

auto threading_fixture::create_consumer() -> std::vector<std::future<void>>
{
    futures_type future;
    future.reserve(static_cast<typename futures_type::size_type>(consumer_));

    for (int iter{0}; iter < consumer_; ++iter)
    {
        future.push_back(std::async(
            std::launch::async, &threading_fixture::consumer_procedure, this));
    }

    return future;
}

bool threading_fixture::is_produce_finish() const noexcept
{
    return is_produce_finish_.load(std::memory_order::memory_order_relaxed);
}

void threading_fixture::producer_procedure(int messages_per_producer)
{
    for (int message{0}; message < messages_per_producer; ++message)
    {
        std::ignore =
            queue_.enqueue(first_message_type{}, second_message_type{});
        wait_.notify_one();
    }
}

void threading_fixture::set_producer(int how_many) noexcept
{
    producer_ = how_many;
}

void threading_fixture::set_consumer(int how_many) noexcept
{
    consumer_ = how_many;
}

void threading_fixture::set_messages_per_producer(int how_many) noexcept
{
    messages_per_producer_ = how_many;
}

} // namespace

TEST_SUITE("logency::detail::thread::blocking_pair_queue")
{
    using first_type = int;
    using second_type = int;
    using queue_type =
        logency::detail::thread::blocking_pair_queue<first_type, second_type>;

    template <typename T>
    using container_type = typename queue_type::container_type<T>;

    SCENARIO("blocking_queue::blocking_pair_queue(size_type)")
    {
        GIVEN("not instantiated object")
        {
            std::unique_ptr<queue_type> queue{nullptr};

            WHEN("instantiate it with default container size")
            {
                CHECK_NOTHROW({ queue = std::make_unique<queue_type>(); });

                THEN("object is instantiated") { CHECK(queue); }
            }

            WHEN("instantiate it with specific container size")
            {
                constexpr const auto requested_size{1U};
                CHECK_NOTHROW(
                    { queue = std::make_unique<queue_type>(requested_size); });

                THEN("object is instantiated")
                {
                    CHECK(queue);

                    AND_THEN(
                        "capacity should be no less than its requested size")
                    {
                        CHECK_GE(queue->capacity(), requested_size);
                    }
                }
            }
        }
    }

    SCENARIO("blocking_queue::~blocking_queue()")
    {
        GIVEN("instantiated object")
        {
            auto queue{std::make_unique<queue_type>()};

            WHEN("destruct")
            {
                CHECK_NOTHROW({ queue.reset(); });

                THEN("object is destroyed") { CHECK(!queue); }
            }
        }
    }

    SCENARIO("void blocking_queue::reserve(size_type)")
    {
        GIVEN("a queue with items")
        {
            queue_type queue;
            const auto capacity{set_queue(queue, 4)};

            WHEN("reserve the queue with larger than its current capacity")
            {
                auto new_capacity{capacity + 1};

                CHECK_NOTHROW({ queue.reserve(new_capacity); });

                THEN("the capacity became larger than or equal to its original "
                     "reserved value")
                {
                    container_type<first_type> first_container;
                    container_type<second_type> second_container;
                    std::ignore =
                        queue.try_swap_bulk(first_container, second_container);

                    CHECK_GE(first_container.capacity(), new_capacity);
                    CHECK_GE(second_container.capacity(), new_capacity);
                }
            }

            WHEN("reserve the queue with no larger than its current capacity")
            {
                auto new_capacity{capacity};

                CHECK_NOTHROW({ queue.reserve(new_capacity); });

                THEN("the capacity remain unchanged")
                {
                    container_type<first_type> first_container;
                    container_type<second_type> second_container;
                    std::ignore =
                        queue.try_swap_bulk(first_container, second_container);

                    CHECK_EQ(first_container.capacity(), capacity);
                    CHECK_EQ(second_container.capacity(), capacity);
                }
            }
        }
    }

    SCENARIO("void blocking_queue::shrink_to_fit()")
    {
        GIVEN("a queue with items and enough capacity")
        {
            queue_type queue;
            const auto capacity{set_queue(queue, 4U)};

            WHEN("call function")
            {
                CHECK_NOTHROW({ queue.shrink_to_fit(); });

                THEN("the capacity remain unchanged")
                {
                    container_type<first_type> first_container;
                    container_type<second_type> second_container;
                    std::ignore =
                        queue.try_swap_bulk(first_container, second_container);

                    CHECK_EQ(first_container.capacity(), capacity);
                    CHECK_EQ(second_container.capacity(), capacity);
                }
            }
        }

        GIVEN("a queue with items and some extra capacity")
        {
            constexpr const std::size_t requested_size{4U};
            constexpr const std::size_t requested_capacity{2 * requested_size};

            queue_type queue;
            const auto capacity{
                set_queue(queue, requested_size, requested_capacity)};

            WHEN("call function")
            {
                CHECK_NOTHROW({ queue.shrink_to_fit(); });

                THEN("the new capacity become no larger than its original one")
                {
                    container_type<first_type> first_container;
                    container_type<second_type> second_container;
                    std::ignore =
                        queue.try_swap_bulk(first_container, second_container);

                    CHECK_LE(first_container.capacity(), capacity);
                    CHECK_LE(second_container.capacity(), capacity);
                }
            }
        }
    }

    SCENARIO(
        "void blocking_queue::enqueue(const first_type &, const second_type &)")
    {
        GIVEN("an empty queue")
        {
            queue_type queue;

            WHEN("enqueue one item")
            {
                constexpr const first_type first_item{0};
                constexpr const second_type second_item{1};
                bool return_code{};

                CHECK_NOTHROW(
                    { return_code = queue.enqueue(first_item, second_item); });

                THEN("return true")
                {
                    CHECK(return_code);

                    AND_THEN("expect item is enqueued inside")
                    {
                        container_type<first_type> first_tray;
                        container_type<second_type> second_tray;

                        std::ignore =
                            queue.try_swap_bulk(first_tray, second_tray);

                        CHECK_EQ(first_tray.at(0), first_item);
                        CHECK_EQ(second_tray.at(0), second_item);
                    }
                }
            }

            WHEN("enqueue multiple items")
            {
                const container_type<first_type> first_items{{0, 1, 2}};
                const container_type<second_type> second_items{{3, 4, 5}};
                std::vector<bool> return_codes{};

                CHECK_NOTHROW({
                    for (auto [first_it, second_it] = std::tuple(
                             first_items.begin(), second_items.begin());
                         (first_it != first_items.end() &&
                          second_it != second_items.end());
                         ++first_it, ++second_it)
                    {
                        bool return_code{queue.enqueue(*first_it, *second_it)};
                        return_codes.push_back(return_code);
                    }
                });

                THEN("first one return true, others return false")
                {
                    CHECK_EQ(return_codes,
                             std::vector<bool>{true, false, false});

                    AND_THEN("expect item is enqueued inside in the same order")
                    {
                        container_type<first_type> first_tray;
                        container_type<second_type> second_tray;
                        std::ignore =
                            queue.try_swap_bulk(first_tray, second_tray);

                        CHECK_EQ(first_tray, first_items);
                        CHECK_EQ(second_tray, second_items);
                    }
                }
            }
        }

        GIVEN("a non empty queue")
        {
            queue_type queue;
            set_queue(queue, 1);

            WHEN("enqueue one item")
            {
                constexpr const first_type first_item{0};
                constexpr const second_type second_item{1};
                bool return_code{};

                CHECK_NOTHROW(
                    { return_code = queue.enqueue(first_item, second_item); });

                THEN("return false")
                {
                    CHECK(!return_code);

                    AND_THEN("expect item is enqueued inside")
                    {
                        container_type<first_type> first_tray;
                        container_type<first_type> second_tray;

                        std::ignore =
                            queue.try_swap_bulk(first_tray, second_tray);

                        CHECK_EQ(first_tray.back(), first_item);
                        CHECK_EQ(second_tray.back(), second_item);
                    }
                }
            }

            WHEN("enqueue multiple items")
            {
                const container_type<first_type> first_items{{0, 1, 2}};
                const container_type<second_type> second_items{{3, 4, 5}};
                std::vector<bool> return_codes{};

                CHECK_NOTHROW({
                    for (auto [first_it, second_it] = std::tuple(
                             first_items.begin(), second_items.begin());
                         (first_it != first_items.end() &&
                          second_it != second_items.end());
                         ++first_it, ++second_it)
                    {
                        bool return_code{queue.enqueue(*first_it, *second_it)};
                        return_codes.push_back(return_code);
                    }
                });

                THEN("all calls return false")
                {
                    CHECK_EQ(return_codes,
                             std::vector<bool>{false, false, false});

                    AND_THEN("expect item is enqueued inside in the same order")
                    {
                        container_type<first_type> first_tray;
                        container_type<second_type> second_tray;
                        std::ignore =
                            queue.try_swap_bulk(first_tray, second_tray);

                        CHECK_EQ(
                            container_type<first_type>{
                                first_tray.end() -
                                    static_cast<int>(first_items.size()),
                                first_tray.end()},
                            first_items);
                        CHECK_EQ(
                            container_type<second_type>{
                                second_tray.end() -
                                    static_cast<int>(second_items.size()),
                                second_tray.end()},
                            second_items);
                    }
                }
            }
        }
    }

    SCENARIO("void blocking_queue::enqueue(first_type &&, second_type &&)")
    {
        GIVEN("an empty queue")
        {
            queue_type queue;

            WHEN("enqueue one item")
            {
                constexpr const first_type first_item{0};
                constexpr const second_type second_item{1};
                bool return_code{};

                CHECK_NOTHROW({
                    return_code = queue.enqueue(first_type{first_item},
                                                second_type{second_item});
                });

                THEN("return true")
                {
                    CHECK(return_code);

                    AND_THEN("expect item is enqueued inside")
                    {
                        container_type<first_type> first_tray;
                        container_type<second_type> second_tray;

                        std::ignore =
                            queue.try_swap_bulk(first_tray, second_tray);

                        CHECK_EQ(first_tray.at(0), first_item);
                        CHECK_EQ(second_tray.at(0), second_item);
                    }
                }
            }

            WHEN("enqueue multiple items")
            {
                const container_type<first_type> first_items{{0, 1, 2}};
                const container_type<second_type> second_items{{3, 4, 5}};
                std::vector<bool> return_codes{};

                CHECK_NOTHROW({
                    for (auto [first_it, second_it] = std::tuple(
                             first_items.begin(), second_items.begin());
                         (first_it != first_items.end() &&
                          second_it != second_items.end());
                         ++first_it, ++second_it)
                    {
                        bool return_code{queue.enqueue(
                            first_type{*first_it}, second_type{*second_it})};
                        return_codes.push_back(return_code);
                    }
                });

                THEN("first one return true, others return false")
                {
                    CHECK_EQ(return_codes,
                             std::vector<bool>{true, false, false});

                    AND_THEN("expect item is enqueued inside in the same order")
                    {
                        container_type<first_type> first_tray;
                        container_type<second_type> second_tray;
                        std::ignore =
                            queue.try_swap_bulk(first_tray, second_tray);

                        CHECK_EQ(first_tray, first_items);
                        CHECK_EQ(second_tray, second_items);
                    }
                }
            }
        }

        GIVEN("a non empty queue")
        {
            queue_type queue;
            set_queue(queue, 1);

            WHEN("enqueue one item")
            {
                constexpr const first_type first_item{0};
                constexpr const second_type second_item{1};
                bool return_code{};

                CHECK_NOTHROW({
                    return_code = queue.enqueue(first_type{first_item},
                                                second_type{second_item});
                });

                THEN("return false")
                {
                    CHECK(!return_code);

                    AND_THEN("expect item is enqueued inside")
                    {
                        container_type<first_type> first_tray;
                        container_type<first_type> second_tray;

                        std::ignore =
                            queue.try_swap_bulk(first_tray, second_tray);

                        CHECK_EQ(first_tray.back(), first_item);
                        CHECK_EQ(second_tray.back(), second_item);
                    }
                }
            }

            WHEN("enqueue multiple items")
            {
                const container_type<first_type> first_items{{0, 1, 2}};
                const container_type<second_type> second_items{{3, 4, 5}};
                std::vector<bool> return_codes{};

                CHECK_NOTHROW({
                    for (auto [first_it, second_it] = std::tuple(
                             first_items.begin(), second_items.begin());
                         (first_it != first_items.end() &&
                          second_it != second_items.end());
                         ++first_it, ++second_it)
                    {
                        bool return_code{queue.enqueue(
                            first_type{*first_it}, second_type{*second_it})};
                        return_codes.push_back(return_code);
                    }
                });

                THEN("all calls return false")
                {
                    CHECK_EQ(return_codes,
                             std::vector<bool>{false, false, false});

                    AND_THEN("expect item is enqueued inside in the same order")
                    {
                        container_type<first_type> first_tray;
                        container_type<second_type> second_tray;
                        std::ignore =
                            queue.try_swap_bulk(first_tray, second_tray);

                        CHECK_EQ(
                            container_type<first_type>{
                                first_tray.end() -
                                    static_cast<int>(first_items.size()),
                                first_tray.end()},
                            first_items);
                        CHECK_EQ(
                            container_type<second_type>{
                                second_tray.end() -
                                    static_cast<int>(second_items.size()),
                                second_tray.end()},
                            second_items);
                    }
                }
            }
        }
    }

    SCENARIO("template <typename TIterator, typename UIterator> "
             "bool blocking_queue::enqueue_bulk("
             "TIterator, TIterator, UIterator, UIterator")
    {
        GIVEN("an empty queue")
        {
            queue_type queue;

            WHEN("enqueue one item")
            {
                constexpr const first_type first_item{0};
                constexpr const second_type second_item{1};
                bool return_code{};

                CHECK_NOTHROW({
                    // clang-format off
                    return_code = queue.enqueue_bulk(&first_item, &first_item + 1, &second_item, &second_item + 1); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                    // clang-format on
                });

                THEN("return true")
                {
                    CHECK(return_code);

                    AND_THEN("expect item is enqueued inside")
                    {
                        container_type<first_type> first_tray;
                        container_type<second_type> second_tray;

                        std::ignore =
                            queue.try_swap_bulk(first_tray, second_tray);

                        CHECK_EQ(first_tray.at(0), first_item);
                        CHECK_EQ(second_tray.at(0), second_item);
                    }
                }
            }

            WHEN("enqueue multiple items")
            {
                const container_type<first_type> first_items{{0, 1, 2}};
                const container_type<second_type> second_items{{3, 4, 5}};
                bool return_code{};

                CHECK_NOTHROW({
                    return_code = queue.enqueue_bulk(
                        first_items.begin(), first_items.end(),
                        second_items.begin(), second_items.end());
                });

                THEN("return true")
                {
                    CHECK(return_code);

                    AND_THEN("expect item is enqueued inside in the same order")
                    {
                        container_type<first_type> first_tray;
                        container_type<second_type> second_tray;
                        std::ignore =
                            queue.try_swap_bulk(first_tray, second_tray);

                        CHECK_EQ(first_tray, first_items);
                        CHECK_EQ(second_tray, second_items);
                    }
                }
            }
        }

        GIVEN("a non empty queue")
        {
            queue_type queue;
            set_queue(queue, 1);

            WHEN("enqueue one item")
            {
                constexpr const first_type first_item{0};
                constexpr const second_type second_item{1};
                bool return_code{};

                CHECK_NOTHROW({
                    // clang-format off
                    return_code = queue.enqueue_bulk(&first_item, &first_item + 1, &second_item, &second_item + 1); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                    // clang-format on
                });

                THEN("return false")
                {
                    CHECK(!return_code);

                    AND_THEN("expect item is enqueued at the end of the queue")
                    {
                        container_type<first_type> first_tray;
                        container_type<second_type> second_tray;

                        std::ignore =
                            queue.try_swap_bulk(first_tray, second_tray);

                        CHECK_EQ(first_tray.back(), first_item);
                        CHECK_EQ(second_tray.back(), second_item);
                    }
                }
            }

            WHEN("enqueue multiple items")
            {
                const container_type<first_type> first_items{{0, 1, 2}};
                const container_type<second_type> second_items{{3, 4, 5}};
                bool return_code{};

                CHECK_NOTHROW({
                    return_code = queue.enqueue_bulk(
                        first_items.begin(), first_items.end(),
                        second_items.begin(), second_items.end());
                });

                THEN("return false")
                {
                    CHECK(!return_code);

                    AND_THEN("expect item is enqueued inside in the same order "
                             "at the end of the queue")
                    {
                        container_type<first_type> first_tray;
                        container_type<second_type> second_tray;
                        std::ignore =
                            queue.try_swap_bulk(first_tray, second_tray);

                        CHECK_EQ(
                            container_type<first_type>{
                                first_tray.end() -
                                    static_cast<int>(first_items.size()),
                                first_tray.end()},
                            first_items);
                        CHECK_EQ(
                            container_type<second_type>{
                                second_tray.end() -
                                    static_cast<int>(second_items.size()),
                                second_tray.end()},
                            second_items);
                    }
                }
            }
        }
    }

    SCENARIO("bool blocking_queue::try_swap_bulk("
             "container_type<first_type> &, container_type<second_type> &)")
    {
        GIVEN("a empty queue")
        {
            queue_type queue;

            WHEN("try to swap container")
            {
                // More easily to test with contents.
                container_type<first_type> first_tray(4);
                container_type<second_type> second_tray(4);

                bool is_swap{false};

                CHECK_NOTHROW({
                    is_swap = queue.try_swap_bulk(first_tray, second_tray);
                });

                THEN("return false")
                {
                    CHECK_EQ(is_swap, false);

                    AND_THEN("will not swap the content of the container")
                    {
                        CHECK_EQ(first_tray, container_type<first_type>(4));
                        CHECK_EQ(second_tray, container_type<second_type>(4));
                    }
                }
            }
        }

        GIVEN("a not empty queue")
        {
            queue_type queue;
            set_queue(queue, 4);

            WHEN("try to swap container")
            {
                container_type<first_type> first_tray;
                container_type<second_type> second_tray;

                bool is_swap{false};

                CHECK_NOTHROW({
                    is_swap = queue.try_swap_bulk(first_tray, second_tray);
                });

                THEN("return true")
                {
                    CHECK_EQ(is_swap, true);

                    THEN("will swap the content of the container")
                    {
                        CHECK_EQ(first_tray, container_type<first_type>(4));
                        CHECK_EQ(second_tray, container_type<second_type>(4));
                    }
                }
            }
        }
    }

    SCENARIO("auto blocking_queue::capacity() -> size_type")
    {
        GIVEN("a queue with items")
        {
            queue_type queue;
            const auto capacity{set_queue(queue, 4)};

            WHEN("call function")
            {
                std::size_t actual{0U};

                CHECK_NOTHROW({ actual = queue.capacity(); });

                THEN("return the requested capacity")
                {
                    CHECK_EQ(actual, capacity);
                }
            }
        }
    }

    SCENARIO("auto blocking_queue::size() -> size_type")
    {
        GIVEN("a queue with items")
        {
            const std::size_t size{4U};
            queue_type queue;
            set_queue(queue, size);

            WHEN("call function")
            {
                std::size_t actual{0U};

                CHECK_NOTHROW({ actual = queue.capacity(); });

                THEN("return the requested size") { CHECK_EQ(actual, size); }
            }
        }
    }

    SCENARIO("bool blocking_queue::is_empty()")
    {
        GIVEN("a queue with no item")
        {
            queue_type queue;

            WHEN("call function")
            {
                std::size_t actual{0U};

                CHECK_NOTHROW({ actual = queue.is_empty(); });

                THEN("return true") { CHECK_EQ(actual, true); }
            }
        }

        GIVEN("a queue with items")
        {
            queue_type queue;
            set_queue(queue, 4U);

            WHEN("call function")
            {
                std::size_t actual{0U};

                CHECK_NOTHROW({ actual = queue.is_empty(); });

                THEN("return false") { CHECK_EQ(actual, false); }
            }
        }
    }

    SCENARIO("blocking_queue threading" * doctest::timeout(0.5))
    {
        threading_fixture fixture;

        constexpr const int single{1};
        constexpr const int multiple{4};

        constexpr const int no_item{0};
        constexpr const int multiple_items{1024};

        GIVEN("single producer single consumer")
        {
            fixture.set_producer(single);
            fixture.set_consumer(single);

            WHEN("produce no item")
            {
                fixture.set_messages_per_producer(no_item);

                CHECK_NOTHROW({ fixture.act(); });

                THEN("no item were consumed")
                {
                    CHECK_EQ(fixture.count(), no_item);
                }
            }

            WHEN("produce multiple items")
            {
                fixture.set_messages_per_producer(multiple_items);

                CHECK_NOTHROW({ fixture.act(); });

                THEN("consume exactly how many it was produced")
                {
                    CHECK_EQ(fixture.count(), multiple_items);
                }
            }
        }

        GIVEN("single producer multiple consumer")
        {
            fixture.set_producer(single);
            fixture.set_consumer(multiple);

            WHEN("produce no item")
            {
                fixture.set_messages_per_producer(no_item);

                CHECK_NOTHROW({ fixture.act(); });

                THEN("no item were consumed")
                {
                    CHECK_EQ(fixture.count(), no_item);
                }
            }

            WHEN("produce multiple items")
            {
                fixture.set_messages_per_producer(multiple_items);

                CHECK_NOTHROW({ fixture.act(); });

                THEN("consume exactly how many it was produced")
                {
                    CHECK_EQ(fixture.count(), multiple_items);
                }
            }
        }

        GIVEN("multiple producer single consumer")
        {
            fixture.set_producer(multiple);
            fixture.set_consumer(single);

            WHEN("produce no item")
            {
                fixture.set_messages_per_producer(no_item);

                CHECK_NOTHROW({ fixture.act(); });

                THEN("no item were consumed")
                {
                    CHECK_EQ(fixture.count(), no_item);
                }
            }

            WHEN("produce multiple items")
            {
                fixture.set_messages_per_producer(multiple_items);

                CHECK_NOTHROW({ fixture.act(); });

                THEN("consume exactly how many it was produced")
                {
                    CHECK_EQ(fixture.count(), multiple * multiple_items);
                }
            }
        }

        GIVEN("multiple producer multiple consumer")
        {
            fixture.set_producer(multiple);
            fixture.set_consumer(multiple);

            WHEN("produce no item")
            {
                fixture.set_messages_per_producer(no_item);

                CHECK_NOTHROW({ fixture.act(); });

                THEN("no item were consumed")
                {
                    CHECK_EQ(fixture.count(), no_item);
                }
            }

            WHEN("produce multiple items")
            {
                fixture.set_messages_per_producer(multiple_items);

                CHECK_NOTHROW({ fixture.act(); });

                THEN("consume exactly how many it was produced")
                {
                    CHECK_EQ(fixture.count(), multiple * multiple_items);
                }
            }
        }
    }
}

} // namespace logency::unit_test::detail::thread
