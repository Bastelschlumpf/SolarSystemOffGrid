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
   Serial.println("Shutdown (" + String((int) (sec / 60)) + " min)");
   Serial.flush();
   delay(100); 
   M5.Rtc.clearIRQ();
   M5.Rtc.setAlarmIRQ(sec);
   delay(10);
   esp_deep_sleep_start();
}
