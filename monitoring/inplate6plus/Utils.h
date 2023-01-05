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
  * @file Utils.h
  * 
  * A collection of helper functions.
  */
#pragma once


extern Inkplate display;

/* Convert the date time to DD.MM.YYYY HH:MM:SS */
String getRTCDateTimeString() 
{
   char buff[32];
   
   display.rtcGetRtcData();
   sprintf(buff, "%02d.%02d.%04d %02d:%02d:%02d",
      display.rtcGetDay(),  display.rtcGetMonth(),  display.rtcGetYear() + 2000,
      display.rtcGetHour(), display.rtcGetMinute(), display.rtcGetSecond());

   return (String) buff;
}

/* Read the RTC timestamp */
time_t GetRTCTime()
{
   tmElements_t tmSet;
   
   display.rtcGetRtcData();
   tmSet.Year   = display.rtcGetYear() + 2000 - 1970;
   tmSet.Month  = display.rtcGetMonth();
   tmSet.Day    = display.rtcGetDay();
   tmSet.Hour   = display.rtcGetHour();
   tmSet.Minute = display.rtcGetMinute();
   tmSet.Second = display.rtcGetSecond();

   // Serial.printf("*** %02d.%02d.%04d %02d:%02d:%02d\n", 
   //    tmSet.Day, tmSet.Month, tmSet.Year + 1970, tmSet.Hour, tmSet.Minute, tmSet.Second);
   
   return makeTime(tmSet);
}

/* UTC date to local time. */
/* UTC date to local time. */
time_t UtcToLocalTime(time_t utcTime)
{
   struct tm *tm = localtime(&utcTime);

   if (tm) {
      tmElements_t tmSet;

      tmSet.Year   = tm->tm_year + 70;
      tmSet.Month  = tm->tm_mon;
      tmSet.Day    = tm->tm_mday;
      tmSet.Hour   = tm->tm_hour;
      tmSet.Minute = tm->tm_min;
      tmSet.Second = tm->tm_sec;
  
      return makeTime(tmSet);
   }
   Serial.println("!!! Error on UtcToLocalTime() !!!");
   return utcTime;
}

/* Convert the date part of the RTC timestamp */
String getRTCDateString() 
{
   char buff[32];

   display.rtcGetRtcData();
   sprintf(buff, "%02d.%02d.%04d",
      display.rtcGetDay(),  display.rtcGetMonth(),  display.rtcGetYear() + 2000);

   return (String) buff;
}

/* Convert the time part of the RTC timestamp */
String getRTCTimeString() 
{
   char buff[32];
   
   display.rtcGetRtcData();
   sprintf(buff, "%02d:%02d:%02d",
      display.rtcGetHour(), display.rtcGetMinute(), display.rtcGetSecond());

   return (String) buff;
}

/* Convert the time_t to the DD.MM.YYYY HH:MM:SS format */
String getDateTimeString(time_t rawtime)
{
   char buff[32];
   
   sprintf(buff,"%02d.%02d.%04d %02d:%02d:%02d",
      day(rawtime), month(rawtime), year(rawtime),
      hour(rawtime), minute(rawtime), second(rawtime));

   return (String) buff;
}

/* Convert the time_t to the DD.MM.YYYY HH:MM:SS format */
String getIoBrokerDateTimeString(DateTime dateTime)
{
   char buff[40];
   
   sprintf(buff,"%04d-%02d-%02dT%02d:%02d:%02dZ",
      dateTime.year(), dateTime.month(),  dateTime.day(),
      dateTime.hour(), dateTime.minute(), dateTime.second());

   return (String) buff;
}

/* Convert the time_t to the date part DD.MM.YYYY */
String getDateString(time_t rawtime)
{
   char buff[32];
   
   sprintf(buff,"%02d.%02d.%04d",
      day(rawtime), month(rawtime), year(rawtime));

   return (String) buff;
}

/* Convert the time_t to the time part HH:MM:SS format */
String getTimeString(time_t rawtime)
{
   char buff[32];
   
   sprintf(buff,"%02d:%02d:%02d",
      hour(rawtime), minute(rawtime), second(rawtime));

   return (String) buff;
}

/* Convert the hour of the time_t */
String getHourString(time_t rawtime)
{
   char buff[32];
   
   sprintf(buff,"%02d",
      hour(rawtime));

   return (String) buff;
}

/* Convert the minute of the time_t */
String getHourMinString(time_t rawtime)
{
   char buff[32];
   
   sprintf(buff,"%02d:%02d",
      hour(rawtime), minute(rawtime));

   return (String) buff;
}

/* Convert the rssi value to a string value between 0 and 100 % */
String WifiGetRssiAsQuality(int rssi)
{
   int quality = 0;

   if (rssi <= -100) {
      quality = 0;
   } else if (rssi >= -50) {
      quality = 100;
   } else {
      quality = 2 * (rssi + 100);
   }
   return String(quality);
}

/* Convert the rssi value to a int value between 0 and 100 % */
int WifiGetRssiAsQualityInt(int rssi)
{
   int quality = 0;

   if (rssi <= -100) {
      quality = 0;
   } else if (rssi >= -50) {
      quality = 100;
   } else {
      quality = 2 * (rssi + 100);
   }
   return quality;
}

/* Trim special characters */
String Trim(String string, String Chars)
{
   String ret;

   for (int i = 0; i < string.length(); i++) {
      if (Chars.indexOf(string[i]) < 0) {
         ret += string[i];
      }
   }
   return ret;
}
