// KiraFlux-Toolkit Example 'core/stack'

#include <kf/main.hpp>

#include <kf/Stack.hpp>

void kf::main(kf::Init &init) {
    init.logger.info("KiraFlux-Toolkit Example: core/stack");

    constexpr usize n = 8;

    // Allocate memory for Stack
    int buffer[n]{
        // 12345, // uncomment
    };

    // create instance
    Stack<int> my_stack{

        // Memory slice
        {buffer},

        // items already in buffer
        // 1,
    };

    // Trying to read
    // if stack has no items read will return none
    if (const auto read = my_stack.read(); read.isNone()) {
        init.logger.error("Failed to read");
    }

    // Fill with numbers from 0 to n-1
    // Should be: {0, 1, 2, ... , n-2, n-1}

    for (int i = 0; i < 10; i += 1) {
        if (my_stack.write(i)) {
            init.logger.info("write ok");
        } else {
            init.logger.error("write fail");
        }
    }

    // Foreach (from Sequence)
    for (int number : my_stack) {

    }

    // Read numbers
    // Should be: {n-1, n-2, ..., 2, 1, 0}

    for (int i = 0; i < 10; i += 1) {
        const auto read = my_stack.read();

        if (read.isSome()) {
            init.logger.info("read ok");
        } else {
            init.logger.error("read fail");
        }
    }

    // Other stack methods

    (void) my_stack.top(); // -> Option<int &>
    (void) my_stack.full(); // -> bool
    (void) my_stack.empty(); // -> bool
    (void) my_stack.length(); // usize -- items stored
    (void) my_stack.capacity(); // usize -- max length
    (void) my_stack.reset(); // discard all values
}
