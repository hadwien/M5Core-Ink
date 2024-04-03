// Copyright (c) M5Stack. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full
// license information.

#ifndef __M5_Data_Class_H__
#define __M5_Data_Class_H__

#include "nvs_flash.h"
#include "nvs.h"

class Data_Class {
   public:
    void begin(void);

    int getSavedBattery(void);
    int getSavedTime(void);

    void pushData(int battery_v);
    /// Get battery voltage
    /// @return battery voltage [mV]
    // int getBatteryVoltage(void);

   private:
    bool _is_nvs_init = false;
    nvs_handle_t _nvs_handle;
};

#endif
