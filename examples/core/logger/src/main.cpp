// KiraFlux-Toolkit Example 'core/logger'

#include <kf/main.hpp>

#include <kf/Logger.hpp>

void kf::main(kf::Init &init) {
    init.logger.info("KiraFlux-Toolkit Example: core/logger");

    // The framework provides a default logger instance via `init.logger` with key "main"
    // Available log levels: debug, info, warn, error
    init.logger.debug("debug");
    init.logger.info("info");
    init.logger.warn("warn");
    init.logger.error("error");

    // The logger supports compile-time format string validation via `mixin::Writable<Impl, char>`
    // Many built-in types are supported: strings, bool, integers, floats, and custom types via `Representable`
    init.logger.debug("Hello {}!, bool is {}, int: {} float: {}", "World", true, 123456, 0.1234f);

    // Users can create additional logger instances with custom keys
    Logger non_buffered_logger{
        "non-buffered",// Each log message is written directly (one writer call per character)
    };

    // Buffered logging reduces the number of writer calls by accumulating data in a fixed-size buffer
    // The buffer is flushed automatically when full or at the end of each log message
    char buffer[64];

    Logger buffered_logger{
        "buffered",
        {buffer},// Implicitly constructs Slice<char> from the array
    };

    // --- Performance comparison: unbuffered vs buffered ---

    // Save the original writer to restore it later (or to chain calls)
    static auto app_logger_function = Logger::writer;

    static usize writer_calls = 0;

    Logger::writer = [](StringView str) {
        app_logger_function(str);
        writer_calls += 1;
    };

    // Unbuffered: each character triggers a writer call
    writer_calls = 0;
    non_buffered_logger.debug("{} calls 0123456789 0123456789 0123456789", writer_calls);
    non_buffered_logger.debug("{} calls 0123456789 0123456789 0123456789", writer_calls);

    // Buffered: only one writer call per flush (buffer holds the entire message)
    writer_calls = 0;
    buffered_logger.debug("{} calls 0123456789 0123456789 0123456789", writer_calls);
    buffered_logger.debug("{} calls 0123456789 0123456789 0123456789", writer_calls);

    // Flush can also be called manually (though the logger flushes automatically after each log message)
    init.logger.flush();
}