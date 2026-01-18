// arch detection

#if defined(__AVR__) || defined(AVR) || defined(__AVR_ATmega328P__) || defined(ARDUINO_AVR_NANO)
#define kf_arch_avr 1
#define kf_arch_name "avr"
#else
#define kf_arch_avr 0
#endif

#if defined(ESP32) || defined(ESP_PLATFORM)
#define kf_arch_esp32 1
#define kf_arch_name "esp32"
#else
#define kf_arch_esp32 0
#endif

#if kf_arch_avr

#define kf_port_has_utility 0
#define kf_port_has_algorithm 0
#define kf_port_has_type_traits 0
#define kf_port_has_functional 0
#define kf_port_has_memory 0
#define kf_port_has_array 0
#define kf_port_has_vector 0
#define kf_port_has_deque 0
#define kf_port_has_queue 0

#else

#define kf_port_has_utility 1
#define kf_port_has_algorithm 1
#define kf_port_has_type_traits 1
#define kf_port_has_functional 1
#define kf_port_has_memory 1
#define kf_port_has_array 1
#define kf_port_has_vector 1
#define kf_port_has_deque 1
#define kf_port_has_queue 1

#endif
