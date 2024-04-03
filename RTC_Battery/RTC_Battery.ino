// Copyright (c) 2024 by GWENDESIGN. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.
//
// https://github.com/m5stack/M5Core-Ink
// https://github.com/m5stack/M5GFX


// This RTC clock example keeps the system mostly in shutdown mode and
//  only wakes up every 58 seconds for a brief period of time during
//  which the time and date are updated on the ink display.
//
// When started initially or via power button a full ink display refresh
//  is executed to clear the display.
// The current time and date are fetched via NTP and shown on the ink display.
// After waiting for the full minute the system is put into shutdown
//  mode for about 58 seconds.
// When the RTC timer expires (just befor the next minute change) the
//  system is powered on.
// The ink display is updated with the current time and date.
// Then the system goes back into shutdown mode for about 58 seconds and
//  the cycle begins anew.
// Every hour a full ink display refresh is executed to keep the ink
//  display crisp.
//
// Note: If WiFi connection fails - some fantasy time and date are used.
// Note: System will not enter shutdown mode while USB is connected.

#include "M5CoreInk.h"

// every hour at minute 45 do a full ink display refresh
#define FULL_REFRESH_MINUTE (55)

static RTC_TimeTypeDef sTime;
static RTC_DateTypeDef sDate;
Ink_Sprite TimePageSprite(&M5.M5Ink);

void getNTPTime() {
  // WiFi connection failed - set fantasy time and date
  RTC_TimeTypeDef time;
  time.Hours = 0;
  time.Minutes = 0;
  time.Seconds = 0;
  M5.rtc.SetTime(&time);

  RTC_DateTypeDef date;
  date.Date = 1;
  date.Month = 12;
  date.Year = 2020;
  M5.rtc.SetDate(&date);
}

void drawTimeAndDate(RTC_TimeTypeDef time, RTC_DateTypeDef date) {
  char buf[11];

  snprintf(buf, 6, "%02d:%02d", time.Hours, time.Minutes);
  TimePageSprite.drawString(40, 20, buf);
  snprintf(buf, 11, "%02d.%02d.%02d", date.Date, date.Month, date.Year - 2000);
  TimePageSprite.drawString(4, 70, buf);
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
  digitalWrite(LED_EXT_PIN, LOW);

  RTC_TimeTypeDef time;
  RTC_DateTypeDef date;

  // Check timer flag
  if ((data & 0b00000100) == 0b00000100) {
    //Serial.println("Power on by: RTC timer");

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
    //Serial.println("Power on by: power button");

    // Full ink display init
    M5.M5Ink.begin();
    M5.M5Ink.clear();

    // Fetch current time from Internet
    getNTPTime();
    M5.rtc.GetTime(&time);
    M5.rtc.GetDate(&date);
  }

  // After every shutdown the sprite is created anew.
  // But the sprite doesn't know about the current image on the
  //  ink display therefore the same time and date, as have been
  //  drawn before the shutdown, are redrawn.
  // This is required, else drawing new time and date only adds
  //  pixels to the already drawn pixels instead of clearing the
  //  previous time and date and then draw the new time and date.
  TimePageSprite.creatSprite(0, 0, 200, 200);

  // drawTimeAndDate(time, date);
  int battery_v = M5.Data.getSavedBattery();
  M5.rtc.GetTime(&sTime);
  M5.rtc.GetDate(&sDate);

  ApplyDataOnScreen(battery_v);

  TimePageSprite.pushSprite();

  // Wait until full minute, e.g. seconds are 0
  while ((time.Seconds != 0)) {
    M5.rtc.GetTime(&time);
    delay(200);
  }

  M5.rtc.GetTime(&sTime);
  M5.rtc.GetDate(&sDate);

  // Draw new time and date
  // drawTimeAndDate(time, date);
  battery_v = M5.Power.getBatteryVoltage();
  M5.Data.pushData(battery_v);
  ApplyDataOnScreen(battery_v);

  TimePageSprite.pushSprite();

  //Serial.printf("Shutdown...\n");
  //Serial.flush();

  // Full refresh once per hour
  if (time.Minutes == FULL_REFRESH_MINUTE - 1) {
    // Allow extra time for full ink refresh
    // Shutdown for 55 seconds only
    M5.shutdown(55);
    return;
  }
  // Shutdown for 58 seconds
  M5.shutdown(58);
}

void loop() {
}
