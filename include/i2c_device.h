#ifndef _I2C_DEVICE_
#define _I2C_DEVICE_

#include "driver/i2c_master.h"

class I2C_DEVICE {
   public:
    i2c_master_bus_config_t _i2c_bus_config;
    i2c_master_bus_handle_t _i2c_bus_handle;
    i2c_device_config_t _dev_cfg;
    i2c_master_dev_handle_t _dev_handle;

    I2C_DEVICE();
    void begin(gpio_num_t sda, gpio_num_t scl, uint16_t address);

    esp_err_t writeByte(uint8_t reg, uint8_t data);
    esp_err_t writeBytes(uint8_t reg, uint8_t const *buffer, size_t length);

    esp_err_t readByte(uint8_t reg, uint8_t *data);
    esp_err_t readBytes(uint8_t reg, uint8_t *buffer, size_t length);

    esp_err_t writeBitOn(uint8_t reg, uint8_t data);
    esp_err_t writeBitOff(uint8_t reg, uint8_t data);
};

#endif