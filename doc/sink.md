# Sink

## Overview

Sink is the main output for logging messages. It get the message from logger, format the message into string buffer, and output content.

Each sink has its own `sink_module`, where the actual logging takes place.

```c++
auto sink = manager.new_sink("this is sink", std::make_unique<sink_module>(/*args*/));
```

When the sinking thread is accessing the `sink` instance (and the `sink_module`), **no other thread can access the `sink` and `sink_module` instance**. It will be blocked by the individual sink `mutex`.

---

## Sink module

When the sink module received the message, it will log out the message to its target.

Each sink module has its own specification. You can use the default sink module this library provided, or implement your own one.

Each sink should have **one and exactly one** instance of sink module.
When the sink instantiate, the sink_module should be created before sink and pass it to the sink instance.
When the sink destroyed, the sink_module will be destroyed.

sink module should be derived from `logency::sink_module::module_interface`. It have 2 virtual functions. These functions represent how the message should be manipulated.

```c++
virtual void module_interface::log_message(string_view_type logger, const message_type &message);

virtual void module_interface::flush();
```

---

## Connection with loggers

We use 'many-to-many' model for connection between logger and sink. Each logger can connect from 0 to many sinks independently.

The connection job is responsible for logger. See [Logger/Connection with sinks](logger.md#connection-with-sinks) for more info.

## Filter

Sink can assign a filter to filter out the message before output the content.

It use `std::function<bool(string_view_type, const message_type &)()` to filter the message.

```c++
void sink::set_filter(
    [](std::string_view logger, const example_message &message)
    {
        // your actual filter
    }
);
```

Default sink does not have any filter inside.

Each sink has its own filter.

Access the filter (e.g. when sinking thread is processing in sink) does not change the state of the filter. **And there is no other thread can access the filter when one thread already occupy the sink instance.**
It is thread-safe to just access the filter when sinking.

Changing the filter is **not** thread safe. You should make sure that no other thread is accessing the filter when you try to change the filter.
Use `manager::wait_until_idle()` to make sure that no other threads are active.
See [Manager/Wait until idle](manager.md#wait-until-idle) for more info.

see [`example/filter.cpp`](../example/filter.cpp) for more examples.

---

## Flusher

Like filter, Sink can assign a flusher to flush the target when the content is pushed into the target buffer.

It use `std::function<bool(string_view_type, const message_type &)()` to flush the message.

```c++
void sink::set_flush(
    [](std::string_view logger, const example_message &message)
    {
        // your actual flusher
    }
);
```

Default sink does not have any flusher inside.

Each sink has its own flusher.

Access the flusher (e.g. when sinking thread is processing in sink) does not change the state of the flusher. **And there is no other thread can access the flusher when one thread already occupy the sink instance.**
It is thread-safe to just access the flusher when sinking.

Changing the flusher is **not** thread safe. You should make sure that no other thread is accessing the flusher when you try to change the flusher.
Use `manager::wait_until_idle()` to make sure that no other threads are active.
See [Manager/Wait until idle](manager.md#wait-until-idle) for more info.

see [`example/flusher.cpp`](../example/flusher.cpp) for more examples.

---

## Lifetime

Based on the architecture. It instantiate after the manager it created. And it **should** be destructed when manager asked to delete it or when manager destructed under normal circumstances.

However, the sink is covered by `std::shared_ptr<>`. That means it **might** live a little longer than the manager if someone is still reluctantly holding the sink instance.

Sink rely on the thread pool. It means that if the manager is destroyed, these is no other thread pool to manipulate the message. When the user try to sink *manually* (by calling the function `sink::log()`), it will throw `logency::runtime_error()` to indicate the user that the thread pool is not exist.
