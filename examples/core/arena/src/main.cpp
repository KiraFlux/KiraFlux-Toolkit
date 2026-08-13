// KiraFlux-Toolkit Example 'core/arena'

// Demonstrates Arena – a linear allocator for fast, deterministic memory allocation.
// Shows:
// - Raw byte allocation (Slice<u8>)
// - Typed array allocation (Slice<T>)
// - Object construction (create<T>())
// - Reset (discards all allocations in O(1))
// - Ownership check (owns())
// - Availability tracking (available())

#include <kf/Arena.hpp>
#include <kf/main.hpp>

#include <kf/String.hpp>
#include <kf/StringView.hpp>

#include <kf/mixin/ExtraAllocationLength.hpp>

struct Player : kf::mixin::ExtraAllocationLength<Player> {
    static constexpr kf::usize max_name_length{32};

    kf::String name;
    int score;

    // Constructor for create<T>().
    explicit constexpr Player(kf::Arena &arena, kf::StringView n, int s) noexcept :
        name{arena.allocate<char>(max_name_length + s)}, score{s} {
        name.append(n);
    }

private:
    KF_IMPL_EXTRA_ALLOCATION_LENGTH(Player);
    static constexpr auto getExtraAllocationLengthImpl(auto, int score, auto const &...) noexcept {
        return max_name_length + score;
    }
};

void kf::main(kf::Init &init) {
    init.logger.info("KiraFlux-Toolkit Example: core/arena");

    // --- 1. Create arena with a static buffer ---
    static u8 memory[2048];
    Arena arena{{memory}};
    init.logger.info("Arena created with {} bytes", arena.available());

    // --- 2. Raw byte allocation (Slice<u8>) ---
    auto raw = arena.allocate(64);
    if (not raw.empty()) {
        init.logger.info("Allocated 64 raw bytes at offset {}",
                         static_cast<usize>(raw.data() - memory));
        raw[0] = 0xAA;
        raw[63] = 0xFF;
        init.logger.debug("raw[0] = {}, raw[63] = {}", raw[0], raw[63]);
    }

    // --- 3. Typed array allocation (Slice<T>) ---
    auto scores = arena.allocate<int>(10);
    if (not scores.empty()) {
        init.logger.info("Allocated 10 ints ({} bytes)", scores.length() * sizeof(int));
        for (usize i = 0; i < scores.length(); i += 1) {
            scores[i] = static_cast<int>(i * 10);
        }
        init.logger.debug("scores[0] = {}, scores[9] = {}", scores[0], scores[9]);
    }

    // --- 4. Object construction with constructor (create<T>()) ---
    auto player_opt = arena.create<Player>("Alice", 100);
    if (player_opt.isSome()) {
        auto &p = player_opt.unwrap();
        init.logger.info("Created player: '{}' (name capacity={}), score={}",
                         p.name, p.name.capacity(), p.score);
    }

    auto player2_opt = arena.create<Player>("Bob", 200);
    if (player2_opt.isSome()) {
        auto &p = player2_opt.unwrap();
        init.logger.info("Created player: '{}' (name capacity={}), score={}",
                         p.name, p.name.capacity(), p.score);
    }

    // --- 5. Array of structures ---
    auto players = arena.allocate<Player>(3);
    if (not players.empty()) {
        init.logger.info("Allocated 3 players");
        players[0] = Player(arena, "Charlie", 50);
        players[1] = Player(arena, "Dave", 75);
        players[2] = Player(arena, "Eve", 120);
        init.logger.debug("players[0].name = {}", players[0].name);
    }

    // --- 6. Availability and ownership ---
    init.logger.info("Remaining bytes: {}", arena.available());
    int *outside = new int(42);
    init.logger.info("owns(scores) = {}", arena.owns(scores.data()));
    init.logger.info("owns(outside) = {}", arena.owns(outside));
    delete outside;

    // --- 7. Reset — O(1) discard all allocations ---
    init.logger.info("Resetting arena...");
    arena.reset();
    init.logger.info("After reset, available: {}", arena.available());

    // --- 8. Reuse after reset ---
    auto new_int = arena.allocate<int>(5);
    if (not new_int.empty()) {
        new_int[0] = 999;
        init.logger.info("Reused arena: new_int[0] = {}", new_int[0]);
    }

    // --- 9. Sub-arena ---
    auto sub_slice = arena.allocate(256);
    if (not sub_slice.empty()) {

        Arena sub_arena{sub_slice};
        auto sub_data = sub_arena.allocate<int>(20);
        if (not sub_data.empty()) {
            sub_data[0] = 12345;
            init.logger.info("Sub-arena allocated int[20], sub_data[0] = {}", sub_data[0]);
        }
        sub_arena.reset();
        init.logger.info("Sub-arena reset, main arena still has {} bytes available",
                         arena.available());
    }

    arena.reset();
    init.logger.info("Final reset: available = {}", arena.available());
}