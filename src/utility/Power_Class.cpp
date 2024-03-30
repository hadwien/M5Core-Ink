// Copyright (c) M5Stack. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full
// license information.

#include "Power_Class.h"

#include <driver/adc.h>
#include <esp_adc_cal.h>
#include <soc/adc_channel.h>

#define BASE_VOLTAGE (3600)

void Power_Class::begin(void) {
    _adc_ratio = 25.1f / 5.1f;
}

static int getBatteryAdcRaw(void) {
    static esp_adc_cal_characteristics_t* adc_chars = nullptr;
    if (adc_chars == nullptr) {
        adc1_config_width(ADC_WIDTH_BIT_12);
        adc1_config_channel_atten(ADC1_GPIO35_CHANNEL, ADC_ATTEN_DB_11);

        adc_chars = (esp_adc_cal_characteristics_t*)calloc(
            1, sizeof(esp_adc_cal_characteristics_t));
        esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12,
                                 BASE_VOLTAGE, adc_chars);
    }

    int raw = adc1_get_raw(ADC1_GPIO35_CHANNEL);

    return esp_adc_cal_raw_to_voltage(raw, adc_chars);
}

int Power_Class::getBatteryVoltage(void) {
    return (getBatteryAdcRaw() * _adc_ratio);
}

int Power_Class::getBatteryLevel(void) {
    float mv  = getBatteryAdcRaw() * _adc_ratio;
    int level = (mv - 3300) * 100 / (float)(4150 - 3350);

    return (level < 0) ? 0 : (level >= 100) ? 100 : level;
}
