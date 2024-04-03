// Copyright (c) M5Stack. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full
// license information.

#include "Data_Class.h"

void Data_Class::begin(void) {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
        err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        if (nvs_flash_erase() == ESP_OK) {
            err = nvs_flash_init();
        }
    }

    if (err == ESP_OK) {
        err = nvs_open("storage", NVS_READWRITE, &_nvs_handle);
        if (err == ESP_OK) {
            _is_nvs_init = true;
        } else {
            printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
            _is_nvs_init = false;
        }
    }
}

int Data_Class::getSavedBattery(void) {
    int battery_v = 0;
    esp_err_t err = nvs_get_i32(_nvs_handle, "battery", &battery_v);
    if (err != ESP_OK) {
        battery_v = 0;
    }

    return battery_v;
}

int Data_Class::getSavedTime(void) {
    int battery_v = 0;
    esp_err_t err = nvs_get_i32(_nvs_handle, "battery", &battery_v);
    if (err != ESP_OK) {
        battery_v = 0;
    }

    return battery_v;
}

void Data_Class::pushData(int battery_v) {
    esp_err_t err = nvs_set_i32(_nvs_handle, "battery", battery_v);
    if (err == ESP_OK) {
        if (nvs_commit(_nvs_handle) != ESP_OK) {
            printf("Failed!\n");
        }
    }

    // Close
    // nvs_close(my_handle);
}
