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
  rtc_date_t RTCDate;
  rtc_time_t RTCtime;
  struct tm  timeinfo;
  
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }

  RTCtime.hour = timeinfo.tm_hour;
  RTCtime.min  = timeinfo.tm_min;
  RTCtime.sec  = timeinfo.tm_sec;
  M5.RTC.setTime(&RTCtime);

  RTCDate.year = timeinfo.tm_year + 1900;
  RTCDate.mon  = timeinfo.tm_mon + 1;
  RTCDate.day  = timeinfo.tm_mday;
  M5.RTC.setDate(&RTCDate);
}

/* Set the internal RTC clock with the weather timestamp */
void UpdateRTCFromNTP()
{
   configTime(0, 3600, "pool.ntp.org");
   setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1);
   tzset();

   updateRTC();
}
