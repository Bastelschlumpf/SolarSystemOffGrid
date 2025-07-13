/*
   Copyright (C) 2022 SFini

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
  * @file EPD.h
  * 
  * Helper functions for initialisizing and shutdown of the M5Paper.
  */
#pragma once

#define GRAYSCALE_0  0

/* Initialize the M5Paper */
void InitEPD(bool clearDisplay = true)
{
   Serial.println("InitEPD: Initializing M5PaperS3");

   auto cfg = M5.config();

   cfg.fallback_board = m5::board_t::board_M5PaperS3;
   cfg.external_rtc  = true;
   cfg.clear_display = false;
   M5.begin(cfg);
   Serial.println("InitEPD: M5PaperS3 initialized");
   M5.Lcd.setRotation(1);
   if (clearDisplay) {
      M5.Lcd.fillScreen(GRAYSCALE_0);
   }
   // disableCore0WDT();
}

/* 
 *  Shutdown the M5Paper 
 *  NOTE: the M5Paper could not shutdown while on usb connection.
 *        In this case use the esp_deep_sleep_start() function.
*/
void ShutdownEPD(int sec)
{
   Serial.begin(115200);
   Serial.println("Shutdown ("        + String((int) (sec / 60)) + " min)");
   Serial.println("Battery Voltage: " + String(M5.Power.getBatteryVoltage()) + " mV");

   // Disable E-Ink display
   M5.Display.sleep();
   M5.Display.waitDisplay();

   // Disable WiFi and Bluetooth
   WiFi.mode(WIFI_OFF);
   btStop();

   // Disable main power pin
   #define M5EPD_MAIN_PWR_PIN GPIO_NUM_2
   pinMode(M5EPD_MAIN_PWR_PIN, OUTPUT);
   digitalWrite(M5EPD_MAIN_PWR_PIN, LOW);
   gpio_hold_en(M5EPD_MAIN_PWR_PIN);
   gpio_deep_sleep_hold_en();

   // Configure unused GPIOs for low power
   int unused_pins[] = {1, 3, 5, 6, 7, 10, 16, 17, 18, 21, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48};
   for (int i = 0; i < sizeof(unused_pins) / sizeof(unused_pins[0]); i++) {
      pinMode(unused_pins[i], INPUT_PULLUP);
      gpio_hold_en((gpio_num_t) unused_pins[i]);
  }

   // Configure touch interrupt pin
   pinMode(GPIO_NUM_36, INPUT_PULLUP);
   gpio_hold_en(GPIO_NUM_36);

   // Enable deep sleep with timer
   M5.Power.deepSleep((uint64_t) sec * 1000000);
}
