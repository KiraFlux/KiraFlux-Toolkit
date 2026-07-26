// KiraFlux-Toolkit Example 'core/string'

// Demonstrates dynamic string building with kf::String.
// String inherits from Stack<char>, providing LIFO operations
// plus string-specific features like formatting and null-termination.

#include <kf/String.hpp>
#include <kf/main.hpp>

void kf::main(kf::Init &init) {
    init.logger.info("KiraFlux-Toolkit Example: core/string");

    // --- String-specific features ---

    char buffer[256];
    kf::String str{buffer};// Construct from Slice<char> (Array decays to Slice)

    // String provides compile-time format string validation via `mixin::Writable<Impl, char>`
    // Use `{}` as anchor, `{{` or `}}` to display literal braces
    // Supported types: StringView, C-strings, integers, floats, booleans, char, and custom types via `Representable`
    str.format(
        "{{ hello }} {} {} {} {} {} {} {}\n",
        kf::StringView{"string-view"},
        "c-string",
        123,  // integer
        6.7,  // floating-point
        true, // boolean
        false,// boolean
        'H'   // char
    );

    init.logger.info("formatted content: {}", str);

    // View as StringView (non-owning, read-only)
    init.logger.info("view length: {}", str.view().length());

    // Null-terminate the string for C-API compatibility (e.g., printf, Serial.print)
    init.logger.info("c-string: {}", str.cString());

    // Static factory: create a formatted string directly into an Array
    auto const arr = kf::String::formatted<64>("Number: {}, Float: {}", 42, 3.1415);
    init.logger.info("static formatted: {}", arr);

    // --- Stack<char> inherited behavior ---

    // String inherits all methods from Stack<char>:
    // - write, read, reset, top, length, capacity, full, empty, data
    // - availableForRead, availableForWrite (from ReadAvailable/WriteAvailable mixins)
    // - Sequence methods: begin, end, operator[], slice, iteration

    // Reset clears the string (length becomes 0)
    str.reset();

    // Check availability after reset (empty string)
    init.logger.info("after reset - availableForRead: {}, availableForWrite: {}",
                     str.availableForRead(), str.availableForWrite());

    // Append various types (inherited from mixin::Writable<Impl, char>)
    str.append(kf::StringView{"sv"});// any Sequence<char>
    str.append("c-str");             // C-string
    str.append(true);                // boolean
    str.append(false);               // boolean
    str.append(kf::i64{13456789});   // integer
    str.append(0.4 - 0.3, 50);       // float with custom precision
    str.append('c');                 // char

    // Availability after appending content
    init.logger.info("after append - availableForRead: {} (characters in string), availableForWrite: {} (free space)",
                     str.availableForRead(), str.availableForWrite());

    // Stack operations: write (push) and read (pop)
    bool const ok = str.write('c');            // writes a character to the stack
    kf::Option<char> const popped = str.read();// reads a character from the stack

    // Availability after push/pop
    init.logger.info("after write+read - availableForRead: {}, availableForWrite: {}",
                     str.availableForRead(), str.availableForWrite());

    // Iteration (inherited from Sequence)
    init.logger.info("iterating over string:");
    for (char c: str) {
        // Note: iteration includes all characters, including null-terminator if present
        // Use str.length() to limit iteration to actual content
    }
    init.logger.info("final length: {}", str.length());

    // --- Error handling ---

    // Overflows are silently ignored (capacity is fixed)
    // Check full() before writing if needed
    if (str.full()) {
        init.logger.warn("string buffer is full");
    }
}