// KiraFlux-Toolkit Example 'core/bare'

#include <kf/main.hpp>

void kf::main(kf::Init &init) {
    init.logger.info("KiraFlux-Toolkit Example: core/bare");

    // --- Bare entry point overview ---

    // kf::main() is the application entry point for KiraFlux Toolkit programs.
    // It replaces setup()/loop() on Arduino or main() on native platforms.
    // The framework initializes the environment and passes an Init context.

    // Init provides:
    //   - logger: a Logger instance for formatted output (info, debug, warn, error)
    //   - io: a platform-agnostic binary I/O interface (read/write bytes, buffers)

    // This example demonstrates the minimal structure of a KiraFlux application.

    // --- Using the logger ---

    // The logger supports compile-time format string validation and
    // many built‑in types (strings, booleans, integers, floats).
    // Use {} as placeholder for arguments.

    init.logger.info("Hello from {}! ", "KiraFlux Toolkit");

    // --- Using the I/O interface ---

    // The 'io' member provides readByte(), writeByte(),
    // readBuffer(), writeBuffer(), and availability checks.

    // Write a simple message as bytes.
    // For text, you can also use writeBuffer() with a string view.

    init.logger.debug("Writing 'Hello, World!' as bytes...");

    u8 const msg[] = "Hello, World!\n";
    (void) init.io.writeBuffer({msg});

    for (u8 i = 0x20; i < 0x80; i += 1) {
        (void) init.io.writeByte(i);
    }

    // --- Simple echo loop ---

    // This loop reads bytes from the input and echoes them back.
    // It demonstrates non‑blocking I/O with availableForRead().

    init.logger.info("Entering echo loop. Type something...");

    while (true) {
        if (init.io.availableForRead() > 0) {
            auto const read = init.io.readByte();

            if (read.isOk()) {
                u8 byte = read.ok();

                // Log the byte (only for non‑control characters)
                if (byte >= 0x20 && byte < 0x7F) {
                    init.logger.debug("Echoed: '{}'", static_cast<char>(byte));
                }

            } else {
                init.logger.error("Read error");
            }
        }
    }

    // --- Performance note ---

    // This minimal application uses no dynamic memory allocation.
    // It is suitable for resource‑constrained devices.
    // The framework handles startup and platform specifics transparently.
}