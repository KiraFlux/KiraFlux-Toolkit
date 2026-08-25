// KiraFlux-Toolkit Example 'core/queue'

// Demonstrates FIFO container (Queue) with fixed-size ring buffer.
// Shows enqueue, dequeue, front/back access, and availability checks.

#include <kf/Queue.hpp>
#include <kf/main.hpp>

void kf::main(kf::Init &init) {
    init.logger.info("KiraFlux-Toolkit Example: core/queue");

    // --- Queue overview ---
    // Queue<T> is a non-owning FIFO container that operates on a fixed-size ring buffer.
    // It does NOT inherit from Sequence, so it does NOT provide begin/end or operator[].
    // Memory must be provided by the user (e.g., a local array or kf::Array).
    // Queue provides front() and back() for direct element access.

    constexpr usize n = 8;
    auto buffer = init.arena.allocate<int>(n);

    // Construct an empty queue using the entire buffer.
    // Optionally, you can specify an initial length if the buffer already contains data.
    kf::Queue<int> my_queue{buffer};// equivalent to Queue{buffer, 0}

    // --- Error handling: reading from an empty queue returns none ---
    if (auto const value = my_queue.read(); value.isNone()) {
        init.logger.error("read from empty queue - returns None");
    }

    // --- Writing (enqueue) ---
    // Queue::write() adds an element to the back. Returns true if successful,
    // false if the queue is full (capacity reached).

    for (int i = 0; i < (n + 2); i += 1) {
        if (my_queue.write(i)) {
            init.logger.debug("write: {} -> ok", i);
        } else {
            init.logger.warn("write: {} -> full, ignored", i);
        }
    }

    // Only the first n elements (0..7) were stored.
    // The remaining (8,9) were rejected.

    // --- Availability ---
    // availableForRead() returns the number of elements currently in the queue.
    // availableForWrite() returns the number of free slots (capacity - length).
    init.logger.info("availableForRead: {} (elements in queue)", my_queue.availableForRead());
    init.logger.info("availableForWrite: {} (free slots)", my_queue.availableForWrite());

    // --- Accessing front and back ---
    // front() returns Option<T&> - mutable reference to the oldest element.
    if (auto front = my_queue.front(); front.isSome()) {
        init.logger.info("front element: {}", front.unwrap());
    }

    // back() returns Option<T&> - mutable reference to the newest element.
    if (auto back = my_queue.back(); back.isSome()) {
        init.logger.info("back element: {}", back.unwrap());
    }

    // --- No iteration ---
    // Queue does not inherit from Sequence, so range-based for is not available.
    // To iterate, you must repeatedly call read() until the queue is empty.

    // --- Reading (dequeue) ---
    // read() removes and returns the front element as Option<T>.
    // Returns None if the queue is empty.

    init.logger.info("dequeuing elements:");
    for (int i = 0; i < (n + 2); i += 1) {
        if (auto value = my_queue.read(); value.isSome()) {
            init.logger.debug("read: {} -> ok", value.unwrap());
            // Show updated availability after each dequeue.
            init.logger.debug("  remaining: {}, free: {}", my_queue.availableForRead(), my_queue.availableForWrite());
        } else {
            init.logger.warn("read: {} -> empty, stopped", i);
            break;
        }
    }

    // After reading all elements, the queue is empty again.

    // --- Other utility methods ---
    init.logger.info("empty: {}", my_queue.empty());      // true
    init.logger.info("length: {}", my_queue.length());    // 0
    init.logger.info("capacity: {}", my_queue.capacity());// n

    // Reset discards all elements (length becomes 0).
    my_queue.reset();

    // Slice access: slice() returns a Slice<T> of the queue content (contiguous copy).
    // Note: slice() is not available because Queue does not inherit Sequence.
    // Instead, you can copy elements manually.

    // --- Performance note ---
    // All operations are O(1). No dynamic allocations.
    // Queue is suitable for real-time / embedded contexts.
}