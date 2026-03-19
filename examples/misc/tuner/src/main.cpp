// KiraFlux-Toolkit Demo 'tuner'
#include <Arduino.h>

#include <kf/tuner/Tuner.hpp>

struct MyTuner;// forward delaration

// config imitation for this demo
// tuner`s goal is find config values
struct MyConfig {
    static constexpr int value_limit{1000};

    const char *name;
    int middle, average;

    MyTuner createTuner(int samples) noexcept;

    void print() const noexcept {
        Serial.printf("%s: middle=%d, average=%d\n", name, middle, average);
    }
};

int mySensorReadRaw() noexcept {
    return random(0, MyConfig::value_limit);
}

// state-machine based tuner
struct MyTuner : kf::tuner::Tuner<MyTuner> {
    explicit MyTuner(MyConfig &config, int samples) noexcept : _config{config}, _samples_total{samples} {}

private:
    MyConfig &_config;
    const int _samples_total;
    int _samples_collected{};
    int _min{}, _max{}, _sum{};

    enum class State {
        Idle,
        Collecting,
        Calculating
    } _state;

    void processSample(int sample) {
        _sum += sample;
        _min = min(sample, _min);
        _max = max(sample, _max);
    }

    // impl

    KF_IMPL_RESETTABLE(MyTuner);
    void resetImpl() noexcept {
        // reset tuner state
        _samples_collected = 0;
        _min = MyConfig::value_limit;
        _max = 0;
        _sum = 0;
        _state = State::Collecting;

        Serial.printf("[%d] %s: starting\n", millis(), _config.name);
    }

    KF_IMPL_POLLABLE(MyTuner);
    void pollImpl() noexcept {
        // on pull - state-depended
        switch (_state) {
            case State::Idle:
                return;

            case State::Collecting: {
                processSample(mySensorReadRaw());

                _samples_collected += 1;

                if (_samples_collected >= _samples_total) {
                    _state = State::Calculating;
                }
            }
                return;

            case State::Calculating: {
                _config.middle = (_min + _max) / 2;
                _config.average = _sum / _samples_total;

                _state = State::Idle;

                Serial.printf("[%d] %s: done\n", millis(), _config.name);
            }
                return;
        }
    }

    // Tuner CRTP interface impl
    friend struct kf::tuner::Tuner<MyTuner>;

    // tuner is running if it is not idle
    bool runningImpl() const noexcept {
        return _state != State::Idle;
    }
};

MyTuner MyConfig::createTuner(int samples) noexcept { return MyTuner{*this, samples}; }

void setup() {
    Serial.begin(115200);

    // create separate configs
    MyConfig config_1{"Config 1"}, config_2{"Config 2"}, config_3{"Config 3"};

    // for each config create it own tuner
    auto tuner_1{config_1.createTuner(50)};
    auto tuner_2{config_1.createTuner(250)};
    auto tuner_3{config_1.createTuner(1000)};

    // launch tuners
    tuner_1.reset();
    tuner_2.reset();
    tuner_3.reset();

    // parallel tuners polling
    while (tuner_1.running() or tuner_2.running() or tuner_3.running()) {
        tuner_1.poll();
        tuner_2.poll();
        tuner_3.poll();
        delay(1);
    }

    // arter this all tuners are done

    config_1.print();
    config_2.print();
    config_3.print();
}

void loop() {}
