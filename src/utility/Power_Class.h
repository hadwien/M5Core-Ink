// Copyright (c) M5Stack. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full
// license information.

#ifndef __M5_Power_Class_H__
#define __M5_Power_Class_H__

class Power_Class {
   public:
    void begin(void);

    /// Turn on/off the power LED.
    /// @param brightness 0=OFF: 1~255=ON (Set brightness if possible.)
    void setLed(int brightness = 255);

    /// Get battery voltage
    /// @return battery voltage [mV]
    int getBatteryVoltage(void);

    int getBatteryLevel(void);
    int getBatteryLevel(int battery_v);

   private:
    float _adc_ratio = 0;
    int _batAdcCh;
};

#endif
