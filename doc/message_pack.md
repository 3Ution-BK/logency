# Message pack

The actual message structure to carry over to sink (hereafter referred to as **message pack**) will contain the following content.

```c++
std::shared_ptr<string_type> logger_name; // what logger is construct the message.
message_type message; // your message
```

`logger_name` is passed by std::shared_ptr to ensure thread safety (the sink can parse properly) if the logger is deleted by another thread.
