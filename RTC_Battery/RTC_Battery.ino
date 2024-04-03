#include "M5CoreInk.h"

// every hour at minute 55 do a full ink display refresh
#define FULL_REFRESH_MINUTE (55)

static RTC_TimeTypeDef sTime;
static RTC_DateTypeDef sDate;
Ink_Sprite TimePageSprite(&M5.M5Ink);

void resetTime() {
  sTime.Hours = 0;
  sTime.Minutes = 0;
  sTime.Seconds = 0;
  M5.rtc.SetTime(&sTime);

  sDate.Date = 1;
  sDate.Month = 12;
  sDate.Year = 2020;
  M5.rtc.SetDate(&sDate);
}

void ApplyDataOnScreen(int battery_v) {
  char volt_array[15] = { 0 };
  char percentage_array[15] = { 0 };
  char time_array[15] = { 0 };

  int battery_level = M5.Power.getBatteryLevel(battery_v);

  int batt_remaining_capacity = round(390 * battery_level / 100);

  sprintf(time_array, "%02dD %02d:%02d", sDate.Date, sTime.Hours, sTime.Minutes);
  sprintf(volt_array, "%4dV", battery_v);
  sprintf(percentage_array, "%3d % | %3dmAh", battery_level, batt_remaining_capacity);

  TimePageSprite.drawString(0, 0, time_array);
  TimePageSprite.drawString(0, 15, volt_array);
  TimePageSprite.drawString(0, 30, percentage_array);
  TimePageSprite.pushSprite();
}

void setup() {
  // Check power on reason before calling M5.begin()
  //  which calls Rtc.begin() which clears the timer flag.
  uint8_t data = 0;
  Wire1.begin(21, 22);
  Wire1.beginTransmission(BM8563_I2C_ADDR);
  Wire1.write(0x01);
  Wire1.endTransmission();
  if (Wire1.requestFrom(BM8563_I2C_ADDR, 1)) {
    data = Wire1.read();
  }

  // Do not yet init ink display
  M5.begin(false, false, false);

  // Green LED - indicates ESP32 is running
  digitalWrite(LED_EXT_PIN, HIGH);

  // Check timer flag
  if ((data & 0b00000100) == 0b00000100) {
    //Serial.println("Power on by: RTC timer");

    RTC_TimeTypeDef time;
    RTC_DateTypeDef date;

    M5.rtc.GetTime(&time);
    M5.rtc.GetDate(&date);

    // Full refresh once per hour
    if (time.Minutes == FULL_REFRESH_MINUTE - 1) {
      // Full ink display init
      M5.M5Ink.begin();
      M5.M5Ink.clear();
    } else {
      // Partial ink display init to avoid flickering
      M5.M5Ink.init_without_reset();
      M5.M5Ink.setEpdMode(epd_mode_t::epd_text);
      M5.M5Ink.invertDisplay(true);
    }
  } else {

    // Full ink display init
    M5.M5Ink.begin();
    M5.M5Ink.clear();

    resetTime();
  }

  TimePageSprite.creatSprite(0, 0, 200, 200);

  int battery_v = M5.Data.getSavedBattery();
  M5.rtc.GetTime(&sTime);
  M5.rtc.GetDate(&sDate);

  ApplyDataOnScreen(battery_v);

  while ((sTime.Seconds != 0)) {
    M5.rtc.GetTime(&sTime);
    delay(200);
  }

  // Draw new time and date
  battery_v = M5.Power.getBatteryVoltage();
  M5.Data.pushData(battery_v);
  ApplyDataOnScreen(battery_v);

  // Full refresh once per hour
  if (sTime.Minutes == FULL_REFRESH_MINUTE - 1) {
    // Allow extra time for full ink refresh
    // Shutdown for 55 seconds only
    M5.shutdown(55);
    return;
  }
  // Shutdown for 58 seconds
  M5.shutdown(59);
}

void loop() {
}
