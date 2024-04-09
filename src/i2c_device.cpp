#include "i2c_device.h"
#include <string.h>

#define BM8563_I2C_ADDR (0x51U)

void UART_Init(void);
void UART_Printf(char const *to_print);

static uint8_t temp_buff[128U] = {0U};

I2C_DEVICE::I2C_DEVICE() {
}

void I2C_DEVICE::begin(gpio_num_t sda, gpio_num_t scl, uint16_t address) {
    _i2c_bus_config.i2c_port                     = I2C_NUM_0;
    _i2c_bus_config.sda_io_num                   = sda;
    _i2c_bus_config.scl_io_num                   = scl;
    _i2c_bus_config.clk_source                   = I2C_CLK_SRC_DEFAULT;
    _i2c_bus_config.glitch_ignore_cnt            = 7;
    _i2c_bus_config.intr_priority                = 0;
    _i2c_bus_config.trans_queue_depth            = 10;
    _i2c_bus_config.flags.enable_internal_pullup = 1;

    ESP_ERROR_CHECK(i2c_new_master_bus(&_i2c_bus_config, &_i2c_bus_handle));

    _dev_cfg.dev_addr_length = I2C_ADDR_BIT_LEN_7,
    _dev_cfg.device_address  = address;
    _dev_cfg.scl_speed_hz    = 100000;

    ESP_ERROR_CHECK(
        i2c_master_bus_add_device(_i2c_bus_handle, &_dev_cfg, &_dev_handle));
}

esp_err_t I2C_DEVICE::writeByte(uint8_t reg, uint8_t data) {
    temp_buff[0U] = reg;
    temp_buff[1U] = data;
    esp_err_t err = i2c_master_transmit(_dev_handle, temp_buff, 2U, 100);

    return err;
}

esp_err_t I2C_DEVICE::writeBytes(uint8_t reg, uint8_t const *buffer,
                                 size_t length) {
    temp_buff[0U] = reg;
    memcpy(&temp_buff[1U], buffer, length);
    esp_err_t err =
        i2c_master_transmit(_dev_handle, temp_buff, 1U + length, 10000);

    return err;
}

esp_err_t I2C_DEVICE::readByte(uint8_t reg, uint8_t *data) {
    esp_err_t err =
        i2c_master_transmit_receive(_dev_handle, &reg, 1, data, 1, 100);

    return err;
}

esp_err_t I2C_DEVICE::readBytes(uint8_t reg, uint8_t *buffer, size_t length) {
    esp_err_t err =
        i2c_master_transmit_receive(_dev_handle, &reg, 1, buffer, length, -1);

    if (length == 3) {
        char array[50] = {0};
        sprintf(array, "%u %u %u\n", buffer[0U], buffer[1U], buffer[2U]);
        UART_Printf(array);
    }

    return err;
}

esp_err_t I2C_DEVICE::writeBitOn(uint8_t reg, uint8_t data) {
    uint8_t temp;
    uint8_t write_back;
    esp_err_t err = readByte(reg, &temp);
    if (err == ESP_OK) {
        write_back = (temp | data);
        err        = writeByte(reg, write_back);
    }

    return err;
}

esp_err_t I2C_DEVICE::writeBitOff(uint8_t reg, uint8_t data) {
    uint8_t temp;
    uint8_t write_back;
    esp_err_t err = readByte(reg, &temp);
    if (err == ESP_OK) {
        write_back = (temp & (~data));
        err        = writeByte(reg, write_back);
    }

    return err;
}
