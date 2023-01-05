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
  * @file RTCTime.h
  * 
  * Helper function to read date/time from a NTP server and update the internal RTC.
  */
#pragma once
#include "time.h"

/* Set the internal RTC clock with the weather timestamp */
void UpdateRTCFromNTP()
{
   Serial.println("Update RTC from NTP.");
   
   configTime(0, 3600, "pool.ntp.org");
   setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1);
   tzset();

   Serial.print(F("Waiting for NTP time sync: "));
   time_t nowSecs = time(nullptr);
   while (nowSecs < 8 * 3600 * 2) {
      delay(500);
      Serial.print(F("."));
      yield();
      nowSecs = time(nullptr);
   }
   Serial.println(" -> ok");
   
   // Used to store time info
   struct tm timeinfo;
   gmtime_r(&nowSecs, &timeinfo);
   
   Serial.print(F("Current time: "));
   Serial.print(asctime(&timeinfo));    

   display.rtcReset();
   display.rtcSetDate(0, timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900 - 2000);
   display.rtcSetTime(timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
}
