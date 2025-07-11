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

/* Update the internal rtc */
void updateRTC()
{
   struct tm timeinfo;
   
   if (!getLocalTime(&timeinfo)) {
      Serial.println("updateRTC: Failed to obtain system time");
      return;
   }

   m5::rtc_time_t RTCtime;
   RTCtime.hours   = timeinfo.tm_hour;
   RTCtime.minutes = timeinfo.tm_min;
   RTCtime.seconds = timeinfo.tm_sec;
   M5.Rtc.setTime(&RTCtime);

   m5::rtc_date_t RTCDate;
   RTCDate.month   = timeinfo.tm_mon + 1;
   RTCDate.date    = timeinfo.tm_mday;
   RTCDate.year    = (timeinfo.tm_year + 1900);
   RTCDate.weekDay = timeinfo.tm_wday;
   M5.Rtc.setDate(&RTCDate);

   Serial.println("updateRTC: RTC updated to " + getRTCDateTimeString());
}

/* Set the internal RTC clock with the weather timestamp */
void UpdateRTCFromNTP()
{
   struct timeval tv = {0, 0};
   settimeofday(&tv, nullptr);   

   configTime(0, 3600, "pool.ntp.org");
   setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1);
   tzset();

   updateRTC();
}
