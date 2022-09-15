# Logger

## Overview

Logger is the main entry point for logging messages. Use `log()` to log the message.

```c++
logger->log(foo, bar, ...);
```

The logger will create the message structure by using **constructor**.

Define your constructor properly to let logger construct your message (or use default one if possible).

> e.g.
>
> ```c++
> struct message
> {
>     std::string my_content;
>     int id;
> };
>
> logger->log(/* my_content */, /* id */);
> ```
>
> ```c++
> struct message
> {
>     explicit message(const std::string &content)
>         : my_content{content}, time{std::chrono::system_clock::now()}
>    {
>    }
>
>     std::string my_content;
>     std::chrono::system_clock::time_point time;
> };
>
> logger->log(/* my_content */);
> ```

---

## Connection with sinks

We use 'many-to-many' model for connection between logger and sink. Each logger can connect from 0 to many sinks independently.

All of the function listed here are meant to be thread safe.

### Connect new sink to the logger

```c++
void logger::add_sink(std::shared_ptr<sink> sink);
```

Add the connection between the logger and the sink if they are not connect, otherwise throw `logency::runtime_error()`.

### Find the sink

```c++
auto logger::find_sink(const string_view_type &name) -> std::shared_ptr<sink>;
```

Return the instance if the sink is found, otherwise return `std::shared_ptr<sink>(nullptr)`.

### Delete the sink

```c++
void logger::delete_sink(const string_type &name);
void logger::delete_sink(std::shared_ptr<sink> sink);
```

Delete the sink if the sink is connected to the logger, otherwise throw `std::runtime_error()`.

---

## Filter

Logger can assign a filter to filter out the message before pushing into the manager queue.

It use `std::function<bool(string_view_type, const message_type &)()` to filter the message.

```c++
void logger::set_filter(
    [](std::string_view logger, const example_message &message)
    {
        // your actual filter
    }
);
```

Default logger does not have any filter inside.

Each logger has its own filter.

**The message will be created first, then it will be filtered.**

Access the filter (e.g. logging using `log()`) does not change the state of the filter. It should be thread-safe.
However, if your filter need to set not thread-safe functionality, consider using mutex or atomic to cover the non thread-safe region.

> e.g.
> ```c++
> void logger::set_filter(
>     [](std::string_view logger, const example_message &message)
>     {
>         // thread-safe operations.
>     }
> );
> ```
>
> ```c++
> std::mutex my_mutex; // Remember to keep it valid for logger.
>
> void logger::set_filter(
>     [&](std::string_view logger, const example_message &message)
>     {
>         std::scoped_lock<std::mutex> lock{my_mutex};
>         // Not thread-safe operations.
>     }
> );
> ```

Changing the filter is **not** thread safe. You should make sure that no other thread is accessing the filter when you try to change the filter.
Use `manager::wait_until_idle()` to make sure that no other threads are active.
See [Manager/Wait until idle](manager.md#wait-until-idle) for more info.

See [`example/filter.cpp`](../example/filter.cpp) for more examples.

---

## Error handler

Like filter, logger can assign a error handler to prevent exception during logging.

Default logger does not have any error handler inside. If something throws, it will throw out of the `log()`.

It use `std::function<void(const std::exception &)>` to handle the error.

```c++
void logger::set_error_handler(
    [](const std::exception &) {/* your actual handler */});
```

Each logger has its own error handler.

It is meant to be thread safe. That means you can try to change the handler in one thread while other thread try to access the handler in logger.
Only one thread can access/modify the error handler at one time.

see [`example/error_handler.cpp`](../example/error_handler.cpp) for more examples.

---

## Lifetime

Based on the architecture. It instantiate after the manager it created. And it **should** be destructed when manager asked to delete it or when manager destructed under normal circumstances.

However, the logger is covered by `std::shared_ptr<>`. That means it **might** live a little longer than the manager if someone is still reluctantly holding the logger instance.

Fortunately, the logger will be marked as destroyed when this situation happened.
When you try to log this kind of logger, it will generate the `logency::runtime_error()` to tell user that it should not be used any longer.

Let go my dear, it is not functional anymore.
