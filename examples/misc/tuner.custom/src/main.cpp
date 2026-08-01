// KiraFlux-Toolkit Example 'tuner/custom'

#include <kf/main.hpp>
#include <kf/rtos/Task.hpp>
#include <kf/tuner/Tuner.hpp>

// --- Forward declaration ---

struct MyTuner;

// --- Configuration for the tuner ---
// The tuner will collect samples and compute middle (min+max/2) and average.

struct MyConfig {
    static constexpr int value_limit = 1000;

    char const *name;
    int middle;
    int average;

    // Factory method: create a tuner for this config.
    MyTuner createTuner(int samples) noexcept;
};

// --- Simulated sensor reading ---

int mySensorReadRaw() noexcept {
    static int a = 0x12'34'56'78;
    a = (a << 5) * (a + 1);
    return (a >> 16);
}

// --- Tuner implementation ---
// Inherits from kf::tuner::Tuner<MyTuner> and implements reset, poll, running.

struct MyTuner : kf::tuner::Tuner<MyTuner> {
    explicit MyTuner(MyConfig &config, int samples) noexcept :
        _config{config}, _samples_total{samples} {}

private:
    MyConfig &_config;       // reference to config (modified after calculation)
    int const _samples_total;// total samples to collect
    int _samples_collected{0};
    int _min{0}, _max{0}, _sum{0};

    enum class State : kf::u8 {
        Idle,
        Collecting,
        Calculating,
    } _state{State::Idle};

    // Process a single sample: update min, max, sum.
    void processSample(int sample) noexcept {
        _sum += sample;
        if (sample < _min) _min = sample;
        if (sample > _max) _max = sample;
    }

    // --- CRTP implementation ---

    KF_IMPL_RESETTABLE(MyTuner);
    void resetImpl() noexcept {
        _samples_collected = 0;
        _min = MyConfig::value_limit;
        _max = 0;
        _sum = 0;
        _state = State::Collecting;
    }

    KF_IMPL_POLLABLE(MyTuner);
    void pollImpl() noexcept {
        switch (_state) {
            case State::Idle:
                return;

            case State::Collecting: {
                processSample(mySensorReadRaw());
                _samples_collected += 1;

                if (_samples_collected >= _samples_total) {
                    _state = State::Calculating;
                }
                break;
            }

            case State::Calculating: {
                _config.middle = (_min + _max) / 2;
                _config.average = _sum / _samples_total;
                _state = State::Idle;
                break;
            }
        }
    }

    KF_IMPL_TUNER(MyTuner);
    bool runningImpl() const noexcept {
        return _state != State::Idle;
    }
};

// --- Factory method implementation ---

MyTuner MyConfig::createTuner(int samples) noexcept {
    return MyTuner{*this, samples};
}

// --- Main application ---

void kf::main(kf::Init &init) {
    init.logger.info("KiraFlux-Toolkit Example: tuner/custom");

    // --- Create three configurations with different sample counts ---
    MyConfig config_1{"Config 1", 0, 0};
    MyConfig config_2{"Config 2", 0, 0};
    MyConfig config_3{"Config 3", 0, 0};

    // --- Create tuners for each config ---
    auto tuner_1 = config_1.createTuner(50);
    auto tuner_2 = config_2.createTuner(250);
    auto tuner_3 = config_3.createTuner(1000);

    // --- Launch all tuners (parallel) ---
    tuner_1.reset();
    tuner_2.reset();
    tuner_3.reset();

    init.logger.info("Tuners started (samples: 50, 250, 1000)");

    // Poll all tuners until all are done.
    while (tuner_1.running() or tuner_2.running() or tuner_3.running()) {
        tuner_1.poll();
        tuner_2.poll();
        tuner_3.poll();
        rtos::Task::sleep(1);
    }

    // --- Results ---
    init.logger.info("All tuners completed");
    init.logger.info("{}: middle={}, average={}", config_1.name, config_1.middle, config_1.average);
    init.logger.info("{}: middle={}, average={}", config_2.name, config_2.middle, config_2.average);
    init.logger.info("{}: middle={}, average={}", config_3.name, config_3.middle, config_3.average);
}