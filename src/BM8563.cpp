#include "BM8563.h"

RTC::RTC() : _i2c() {
}

void RTC::begin(void) {
    _i2c.begin(GPIO_NUM_21, GPIO_NUM_22, BM8563_I2C_ADDR);
    _i2c.writeByte(0x00, 0x00);
    _i2c.writeByte(0x0E, 0x03);
}

void RTC::GetBm8563Time(void) {
    if (_i2c.readBytes(0x02, _trdata, 7) == ESP_OK) {
        DataMask();
        Bcd2asc();
        Str2Time();
    }
}

void RTC::Str2Time(void) {
    Second = (asc[0] - 0x30) * 10 + asc[1] - 0x30;
    Minute = (asc[2] - 0x30) * 10 + asc[3] - 0x30;
    Hour   = (asc[4] - 0x30) * 10 + asc[5] - 0x30;
    /*
  uint8_t Hour;
  uint8_t Week;
  uint8_t Day;
  uint8_t Month;
  uint8_t  Year;
  */
}

void RTC::DataMask() {
    _trdata[0] = _trdata[0] & 0x7f;  // 秒
    _trdata[1] = _trdata[1] & 0x7f;  // 分
    _trdata[2] = _trdata[2] & 0x3f;  // 时

    _trdata[3] = _trdata[3] & 0x3f;  // 日
    _trdata[4] = _trdata[4] & 0x07;  // 星期
    _trdata[5] = _trdata[5] & 0x1f;  // 月

    _trdata[6] = _trdata[6] & 0xff;  // 年
}
/********************************************************************
 void Bcd2asc(void)

***********************************************************************/
void RTC::Bcd2asc(void) {
    uint8_t i, j;
    for (j = 0, i = 0; i < 7; i++) {
        asc[j++] = (_trdata[i] & 0xf0) >> 4 | 0x30;
        asc[j++] = (_trdata[i] & 0x0f) | 0x30;
    }
}

uint8_t RTC::bcd2ToByte(uint8_t value) {
    return ((value >> 4) * 10) + (value & 0x0F);
}

uint8_t RTC::byteToBcd2(uint8_t value) {
    uint_fast8_t bcdhigh = value / 10;
    return (bcdhigh << 4) | (value - (bcdhigh * 10));
}

void RTC::GetTime(RTC_TimeTypeDef *RTC_TimeStruct) {
    uint8_t buf[3] = {0};
    if (_i2c.readBytes(0x02, buf, 3) == ESP_OK) {
        RTC_TimeStruct->Seconds = bcd2ToByte(buf[0] & 0x7f);
        RTC_TimeStruct->Minutes = bcd2ToByte(buf[1] & 0x7f);
        RTC_TimeStruct->Hours   = bcd2ToByte(buf[2] & 0x3f);
    }
}

void RTC::SetTime(RTC_TimeTypeDef *RTC_TimeStruct) {
    if (RTC_TimeStruct == NULL) return;

    uint8_t buf[] = {byteToBcd2(RTC_TimeStruct->Seconds),
                     byteToBcd2(RTC_TimeStruct->Minutes),
                     byteToBcd2(RTC_TimeStruct->Hours)};

    _i2c.writeBytes(0x02, buf, sizeof(buf));
}

void RTC::GetDate(RTC_DateTypeDef *RTC_DateStruct) {
    uint8_t buf[4] = {0};
    if (_i2c.readBytes(0x05, buf, 4) == ESP_OK) {
        RTC_DateStruct->Date    = bcd2ToByte(buf[0] & 0x3f);
        RTC_DateStruct->WeekDay = bcd2ToByte(buf[1] & 0x07);
        RTC_DateStruct->Month   = bcd2ToByte(buf[2] & 0x1f);
        RTC_DateStruct->Year =
            bcd2ToByte(buf[3] & 0xff) + ((0x80 & buf[2]) ? 1900 : 2000);
    }
}

void RTC::SetDate(RTC_DateTypeDef *RTC_DateStruct) {
    if (RTC_DateStruct == NULL) return;

    uint8_t buf[] = {byteToBcd2(RTC_DateStruct->Date),
                     byteToBcd2(RTC_DateStruct->WeekDay),
                     (uint8_t)(byteToBcd2(RTC_DateStruct->Month) +
                               (RTC_DateStruct->Year < 2000 ? 0x80 : 0)),
                     byteToBcd2(RTC_DateStruct->Year % 100)};

    _i2c.writeBytes(0x05, buf, sizeof(buf));
}

int RTC::SetAlarmIRQ(int afterSeconds) {
    uint8_t reg_value = 0U;
    _i2c.readByte(0x01, &reg_value);
    reg_value = reg_value & ~0x0C;
    if (afterSeconds < 0) {  // disable timer
        _i2c.writeByte(0x01, reg_value & ~0x01);
        _i2c.writeByte(0x0E, 0x03);
        return -1;
    }

    size_t div         = 1;
    uint8_t type_value = 0x82;
    if (afterSeconds < 270) {
        if (afterSeconds > 255) {
            afterSeconds = 255;
        }
    } else {
        div          = 60;
        afterSeconds = (afterSeconds + 30) / div;
        if (afterSeconds > 255) {
            afterSeconds = 255;
        }
        type_value = 0x83;
    }

    _i2c.writeByte(0x0E, type_value);
    _i2c.writeByte(0x0F, afterSeconds);
    _i2c.writeByte(0x01, (reg_value | 0x01) & ~0x80);
    return afterSeconds * div;
}

int RTC::SetAlarmIRQ(const RTC_TimeTypeDef &RTC_TimeStruct) {
    uint8_t irq_enable = false;
    uint8_t out_buf[4] = {0x80, 0x80, 0x80, 0x80};

    if (RTC_TimeStruct.Minutes >= 0) {
        irq_enable = true;
        out_buf[0] = byteToBcd2(RTC_TimeStruct.Minutes) & 0x7f;
    }

    if (RTC_TimeStruct.Hours >= 0) {
        irq_enable = true;
        out_buf[1] = byteToBcd2(RTC_TimeStruct.Hours) & 0x3f;
    }

    out_buf[2] = 0x80;
    out_buf[3] = 0x80;

    uint8_t reg_value = 0U;
    _i2c.readByte(0x01, &reg_value);

    if (irq_enable) {
        reg_value |= (1 << 1);
    } else {
        reg_value &= ~(1 << 1);
    }

    for (int i = 0; i < 4; i++) {
        _i2c.writeByte(0x09 + i, out_buf[i]);
    }
    _i2c.writeByte(0x01, reg_value);

    return irq_enable ? 1 : 0;
}

int RTC::SetAlarmIRQ(const RTC_DateTypeDef &RTC_DateStruct,
                     const RTC_TimeTypeDef &RTC_TimeStruct) {
    uint8_t irq_enable = false;
    uint8_t out_buf[4] = {0x80, 0x80, 0x80, 0x80};

    if (RTC_TimeStruct.Minutes >= 0) {
        irq_enable = true;
        out_buf[0] = byteToBcd2(RTC_TimeStruct.Minutes) & 0x7f;
    }

    if (RTC_TimeStruct.Hours >= 0) {
        irq_enable = true;
        out_buf[1] = byteToBcd2(RTC_TimeStruct.Hours) & 0x3f;
    }

    if (RTC_DateStruct.Date >= 0) {
        irq_enable = true;
        out_buf[2] = byteToBcd2(RTC_DateStruct.Date) & 0x3f;
    }

    if (RTC_DateStruct.WeekDay >= 0) {
        irq_enable = true;
        out_buf[3] = byteToBcd2(RTC_DateStruct.WeekDay) & 0x07;
    }

    uint8_t reg_value = 0U;
    _i2c.readByte(0x01, &reg_value);

    if (irq_enable) {
        reg_value |= (1 << 1);
    } else {
        reg_value &= ~(1 << 1);
    }

    for (int i = 0; i < 4; i++) {
        _i2c.writeByte(0x09 + i, out_buf[i]);
    }
    _i2c.writeByte(0x01, reg_value);

    return irq_enable ? 1 : 0;
}

void RTC::clearIRQ() {
    _i2c.writeBitOff(0x01, 0x0C);
}

void RTC::disableIRQ() {
    // disable alarm (bit7:1=disabled)
    static constexpr const uint8_t buf[4] = {0x80, 0x80, 0x80, 0x80};
    _i2c.writeBytes(0x09, (uint8_t *)buf, 4);

    // disable timer (bit7:0=disabled)
    _i2c.writeByte(0x0E, 0);

    // clear flag and INT enable bits
    _i2c.writeByte(0x01, 0x00);
}
