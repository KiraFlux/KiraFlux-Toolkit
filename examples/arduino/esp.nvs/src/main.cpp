// KiraFlux-Toolkit Example 'arduino/esp.nvs'

#include <kf/esp/NVS.hpp>
#include <kf/main.hpp>

using kf::esp::NVS;

// --- NVS instance ---
// Namespace name must be ≤ 16 characters (NVS_NS_NAME_MAX_SIZE).
// NVS does not own the string; it must remain valid.
NVS my_nvs{"my_namespace"};

// --- Application configuration stored in NVS ---
// The 'magic' field allows detecting corruption or version changes.
struct AppConfig {
    kf::u32 magic;
    char device_name[16];
    float x, y, z;
} my_app_config{};

void kf::main(kf::Init &init) {
    init.logger.info("KiraFlux-Toolkit Example: arduino/esp.nvs");

    // Initialize NVS (opens the namespace)
    auto init_result = my_nvs.init();
    if (init_result.isError()) {
        init.logger.error("NVS init failed: {}", init_result.error());
        return;
    }

    // Try to load existing config from NVS
    auto load_result = my_nvs.getBlob(
        "app_config",
        {reinterpret_cast<u8 *>(&my_app_config), sizeof(my_app_config)});

    // If config is missing, corrupted, or magic mismatch – reset to defaults
    if (load_result.isError() or my_app_config.magic != 0xAA'BB'CC'DD) {
        // Default values
        my_app_config = {
            .magic = 0xAA'BB'CC'DD,
            .device_name = "MyExample dev",
            .x = 1.0f,
            .y = 2.0f,
            .z = 3.0f,
        };

        // Write defaults back to NVS
        auto save_result = my_nvs.setBlob(
            "app_config",
            {reinterpret_cast<const u8 *>(&my_app_config), sizeof(my_app_config)});

        if (save_result.isError()) {
            init.logger.error("Failed to save config: {}", save_result.error());
        } else {
            init.logger.info("Default config saved");
        }
    }

    // --- Use the config ---
    init.logger.info("Device: {}", my_app_config.device_name);
    init.logger.info("Position: ({}, {}, {})", my_app_config.x, my_app_config.y, my_app_config.z);

    // --- Cleanup ---
    // Always release NVS resources when done.
    my_nvs.quit();
}