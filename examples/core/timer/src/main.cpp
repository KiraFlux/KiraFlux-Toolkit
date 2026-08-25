// KiraFlux-Toolkit Example 'core/timer'

#include <kf/Timer.hpp>
#include <kf/main.hpp>
#include <kf/rtos/Clock.hpp>
#include <kf/rtos/Task.hpp>

void kf::main(kf::Init &init) {
    init.logger.info("KiraFlux-Toolkit Example: core/timer");

    // --- Timer configurations ---

    Timer::Config one_shot_config{.value = 2000};// fires once after 2 seconds
    Timer::Config periodic_config{.value = 500}; // fires every 500 ms (2 Hz)
    Timer::Config stopwatch_config{.value = 0};  // zero period = stopwatch mode

    Timer one_shot{one_shot_config};
    Timer periodic{periodic_config};
    Timer stopwatch{stopwatch_config};

    // --- Start all timers ---

    auto now = rtos::Clock::now();
    one_shot.start(now);
    periodic.start(now);
    stopwatch.start(now);

    bool one_shot_armed{true};

    // --- Main loop (100 iterations, 100 ms each = 10 seconds total) ---

    for (auto i = 0u; i < 100u; i += 1) {
        now = rtos::Clock::now();

        // One-shot timer: show remaining time
        init.logger.debug("One-shot remaining: {} ms", one_shot.remaining(now));

        // Check if one-shot expired
        if (one_shot_armed and one_shot.expired(now)) {
            one_shot_armed = false;
            init.logger.info("One-shot expired");
        }

        // Periodic timer: tick every 100 ms
        if (periodic.expired(now)) {
            init.logger.info("Periodic tick at {}", now);
            periodic.start(now);// re-arm for next period
        }

        // Stopwatch: show elapsed time
        init.logger.debug("Stopwatch: {} ms", stopwatch.elapsed(now));

        // Small delay to avoid flooding the log
        rtos::Task::sleep(100);
    }

    init.logger.info("Timer demo finished");
}