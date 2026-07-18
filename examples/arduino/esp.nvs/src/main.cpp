// KiraFlux-Toolkit Example 'arduino/esp.nvs'

#include <kf/esp/NVS.hpp>
#include <kf/main.hpp>

using kf::esp::NVS;

NVS my_nvs_entry{
    "my namespace",// max length is 16 (NVS_NS_NAME_MAX_SIZE)
};

struct MyAppConfig {
    kf::u32 magic;
    char device_name[16];
    float x, y, z;

} my_app_config;

void kf::main(kf::Init &init) {
    init.logger.info("KiraFlux-Toolkit Example: arduino/esp.nvs");

    if (const auto result = my_nvs_entry.init(); result.isError()) {
        const NVS::Error cause = result.error();
        return;
    }

    const char *name = my_nvs_entry.name();
    // name: "my namespace"

    const auto get_blob_result = my_nvs_entry.getBlob(
        // blob key
        "my_data",
        // write blob into struct
        {reinterpret_cast<kf::u8 *>(&my_app_config), sizeof(my_app_config)});

    // Config is corrupted or outdated
    if (my_app_config.magic != 0xAA'BB'CC'DD) {

        my_app_config = {
            .magic = 0xAA'BB'CC'DD,
            .device_name = "MyExample dev",
            .x = 1,
            .y = 2,
            .z = 3,
        };

        const auto set_blob_result = my_nvs_entry.setBlob(
            // blob key
            "my_data",
            // read blob from struct
            {reinterpret_cast<const kf::u8 *>(&my_app_config), sizeof(my_app_config)});
    }

    my_nvs_entry.quit();// free resources

    // any NVS operation here will cause an error
}
