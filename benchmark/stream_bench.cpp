#include "logency/core/exception.hpp"
#include "logency/manager.hpp"
#include "logency/message/stream_message.hpp"
#include "logency/sink_module/basic_file_module.hpp"
#include "logency/sink_module/null_module.hpp"
#include "logency/sink_module/rotation_file_module.hpp"

#include <cstdint>
#include <cstdio>

#include <fstream>
#include <future>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

using bench_message = logency::message::stream_message<char>;
using bench_formatter = logency::message::stream_message_formatter<char>;

using bench_manager = logency::manager<bench_message>;
using bench_clock = std::chrono::high_resolution_clock;

namespace constant
{

constexpr const size_t default_thread_in_manager{1};
constexpr const int default_push_thread_number{4};
constexpr const int default_message_per_thread{62500};

} // namespace constant

struct input_argument;

template <typename MessageType, typename FormatterType>
static void benchmark(input_argument input);

template <typename MessageType>
static void
benchmark_sink(input_argument input, bench_manager &manager,
               const std::shared_ptr<logency::sink<MessageType>> &sink);

static void help(char *name);

static void info(input_argument input);

struct input_argument
{
    int thread_count{constant::default_push_thread_number};
    int message_per_thread{constant::default_message_per_thread};
    size_t thread_in_manager{constant::default_thread_in_manager};
};

int main(int argc, char *argv[])
{
    try
    {
        input_argument input;

        if (argc == 4)
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            input.thread_count = std::stoi(argv[1]);

            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            input.message_per_thread = std::stoi(argv[2]);

            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            input.thread_in_manager = static_cast<size_t>(std::stoull(argv[3]));
        }
        else if (argc != 1)
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            help(argv[0]);
            return 1;
        }

        info(input);

        benchmark<bench_message, bench_formatter>(input);
    }
    catch (const logency::runtime_error &e)
    {
        std::cerr << "Error occur: " << e.what();
        return 1;
    }

    return 0;
}

template <typename MessageType, typename FormatterType>
static void benchmark(input_argument input)
{
    using message = MessageType;
    using formatter = FormatterType;

    std::cout << "--------------------\n"
              << "MessageType: " << typeid(message).name() << "\n"
              << "MessageType formatter: " << typeid(formatter).name() << "\n"
              << "--------------------" << std::endl;

    logency::manager<message> manager{input.thread_in_manager};

    {
        // basic file sink
        auto basic_file_sink{manager.new_sink(
            "basic_file_sink",
            std::make_unique<
                logency::sink_module::basic_file_module<message, formatter>>(
                "log/basic_file_sink.txt", logency::file_open_mode::truncate,
                std::make_unique<formatter>()))};

        benchmark_sink(input, manager, std::move(basic_file_sink));

        manager.delete_sink("basic_file_sink");
    }
    {
        // rotate file sink
        const logency::sink_module::rotation_file::rotate_info rotate_info{
            static_cast<std::uintmax_t>(1 * 1024 * 1024), 5};
        const auto mode{logency::sink_module::rotation_file::construct_mode::
                            create_new_file};

        auto rotation_file_sink{manager.new_sink(
            "rotation_file_sink",
            std::make_unique<
                logency::sink_module::rotation_file_module<message, formatter>>(
                "log/rotation_file_sink.txt", rotate_info, mode,
                std::make_unique<formatter>()))};

        benchmark_sink(input, manager, std::move(rotation_file_sink));

        manager.delete_sink("rotation_file_sink");
    }

    {
        // null sink
        auto null_sink{manager.new_sink(
            "null_sink",
            std::make_unique<logency::sink_module::null_module<message>>())};

        benchmark_sink(input, manager, std::move(null_sink));

        manager.delete_sink("null_sink");
    }
}

template <typename MessageType>
static void
benchmark_sink(input_argument input, bench_manager &manager,
               const std::shared_ptr<logency::sink<MessageType>> &sink)
{
    const std::string &name{sink->name()};

    std::vector<std::future<void>> futures;
    futures.reserve(
        static_cast<std::vector<std::thread>::size_type>(input.thread_count));

    auto logger{manager.new_logger(name)};
    logger->add_sink(sink);

    auto start{bench_clock::now()};

    for (int id{0}; id < input.thread_count; ++id)
    {
        futures.push_back(std::async(
            [&, id]()
            {
                for (int number{0}; number < input.message_per_thread; ++number)
                {
                    logger->log(logency::log_level::info,
                                "MessageType (id - number): ", id, " - ",
                                number);
                }
            }));
    }

    for (auto &future : futures)
    {
        future.wait();
    };

    auto push_complete_time{
        std::chrono::duration_cast<std::chrono::duration<double>>(
            bench_clock::now() - start)
            .count()};

    manager.wait_until_idle();

    auto finish_time{std::chrono::duration_cast<std::chrono::duration<double>>(
                         bench_clock::now() - start)
                         .count()};

    const auto total_count{input.thread_count * input.message_per_thread};

    std::cout << "Name: " << name << "\n"
              << "[Thread push finish] \tElapsed: " << push_complete_time
              << "sec \tMessage per sec: " << (total_count / push_complete_time)
              << "\n"
              << "[Logging finish] \tElapsed: " << finish_time
              << "sec \tMessage per sec: " << (total_count / finish_time)
              << "\n"
              << std::endl;

    manager.delete_logger(name);
}

static void help(char *name)
{
    std::cout
        << "Error: incorrect argument\n"
        << "usage: " << name
        << " [thread_count] [message_per_thread] [thread_in_manager]\n"
        << "\tthread_count (int): how many thread should this benchmark run.\n"
        << "\tmessage_per_thread (int): how many message should this benchmark "
           "send for each thread.\n"
        << "\tthread_in_manager (size_t): how many thread is operating "
           "inside the manager.";
}

static void info(input_argument input)
{
    const auto total_count{input.thread_count * input.message_per_thread};
    std::cout << "[Benchmark Info]\n"
              << "Input Thread: " << input.thread_count << "\n"
              << "MessageType per thread: " << input.message_per_thread << "\n"
              << "Total messages: " << total_count << "\n"
              << "Threads in manager: " << input.thread_in_manager << std::endl;
}
