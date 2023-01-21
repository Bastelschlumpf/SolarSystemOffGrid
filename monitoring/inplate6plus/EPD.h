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
#include "driver/rtc_io.h" //ESP32 library used for deep sleep and RTC wake up pins

/* Initialize the M5Paper */
void InitEPD(bool clearDisplay = true)
{
   Serial.println("Init");
   
   display.begin();

   // Switch on the front lighting briefly so that you can recognize a start.
   display.frontlight(1);
   display.setFrontlight(10);
   delay(100);
   display.frontlight(0);

   if (clearDisplay) {
      display.clearDisplay();
   }
}

/* 
 *  Shutdown the module
 *  NOTE: the modul could not shutdown while on usb connection.
 *        In this case use the esp_deep_sleep_start() function.
*/
void ShutdownEPD(int sec)
{
   Serial.println("Shutdown");

   display.tsShutdown();                            // Turn off the display touchscreen
   display.frontlight(0);                           // Turn off the frontlight
   rtc_gpio_isolate(GPIO_NUM_12);                   // Isolate/disable GPIO12 on ESP32 (only to reduce power consumption in sleep)
   esp_sleep_enable_timer_wakeup(sec * 1000000);    // Activate wake-up timer -- wake up after sec here
   esp_sleep_enable_ext0_wakeup(GPIO_NUM_36, LOW);  // Enable wakeup from deep sleep on gpio 36 (wake button)
   esp_deep_sleep_start();                          // Put ESP32 into deep sleep. Program stops here.    
}
