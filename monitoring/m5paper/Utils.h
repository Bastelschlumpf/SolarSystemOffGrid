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
#include <stdarg.h>
#include <Time.h>
#include <TimeLib.h> 

/**
  * Profiling helper class.
  */
class CTimeProf
{
protected:
   int    m_iMillis;
   String m_Info;

public:
   CTimeProf(String Info)
      : m_Info(Info)
      , m_iMillis(millis())
   {
      // Serial.printf("[% 8.2f sec %s\n", (float) (m_iMillis / 1000.0), m_Info.c_str());
   }
   ~CTimeProf()
   {
      int iMillis = millis();

      // if ((float) (iMillis - m_iMillis) / 1000.0 >= 0.02) {
         Serial.printf("\n***% 2.2f sec [% 2.2f sec] %s\n", (float) (iMillis - m_iMillis) / 1000.0, (float) (iMillis / 1000.0), m_Info.c_str());
      // }
   }
};

#define TIME_PROF(m) CTimeProf TimeProf(m);

/**
  * HistoryData: A collection af history float values with its DateTime position.
  */
class HistoryData
{
public:
   int       size_;     //!< Size of the history items.
   float    *values_;   //!< History float values.
   DateTime *dates_;    //!< History timeline.
   String    unitName_; //!< Unit Name of the values
   int      *counts_;   //!< Occurrence.
   float     max_;      //!< Max value.

public:
   HistoryData(int historySize, String unitName)
      : size_(historySize)
      , unitName_(unitName)
      , max_(0.0)
   {
      values_ = new float[size_];
      dates_  = new DateTime[size_];
      counts_ = new int[size_];

      clear();
   }
   ~HistoryData()
   {
      delete [] values_;
      delete [] dates_;
      delete [] counts_;
   }

   float getMax()
   {
      return ceil(max_);
   }

   void clear()
   {
      memset(values_, 0, size_ * sizeof(float));
      memset(dates_,  0, size_ * sizeof(DateTime));
      memset(counts_, 0, size_ * sizeof(int));
      max_ = 0.0;
   }
};

/* Printf to a String */
String StringPrintf(char *fmt, ... )
{
   char buf[255];

   memset(buf, 0, sizeof(buf));
   va_list args;
   va_start (args, fmt );
   vsnprintf(buf, sizeof(buf), fmt, args);
   va_end (args);
   return String(buf);
}

/* Convert the RTC date time to DD.MM.YYYY HH:MM:SS */
String getRTCDateTimeString() 
{
   char           buff[32];
   m5::rtc_date_t date_struct;
   m5::rtc_time_t time_struct;
   
   M5.Rtc.getDate(&date_struct);
   M5.Rtc.getTime(&time_struct);

   sprintf(buff, "%02d.%02d.%04d %02d:%02d:%02d",
      date_struct.date,
      date_struct.month,
      date_struct.year,
      time_struct.hours,
      time_struct.minutes,
      time_struct.seconds);

   return String(buff);
}

/* Read the RTC timestamp */
time_t GetRTCTime()
{
   tmElements_t   tmSet;
   m5::rtc_date_t date_struct;
   m5::rtc_time_t time_struct;
   
   M5.Rtc.getDate(&date_struct);
   M5.Rtc.getTime(&time_struct);

   tmSet.Year   = date_struct.year - 1970;
   tmSet.Month  = date_struct.month;
   tmSet.Day    = date_struct.date;
   tmSet.Hour   = time_struct.hours;
   tmSet.Minute = time_struct.minutes;
   tmSet.Second = time_struct.seconds;

   return makeTime(tmSet);
}

/* UTC date to local time. */
time_t UtcToLocalTime(time_t utcTime)
{
   struct tm *tm = localtime(&utcTime);

   if (tm) {
      tmElements_t tmSet;

      tmSet.Year   = tm->tm_year + 1900 - 1970;
      tmSet.Month  = tm->tm_mon + 1;
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
   char           buff[32];
   m5::rtc_date_t date_struct;
   
   M5.Rtc.getDate(&date_struct);

   sprintf(buff,"%02d.%02d.%04d",
      date_struct.date, date_struct.month, date_struct.year);

   return String(buff);
}

/* Convert the time part of the RTC timestamp */
String getRTCTimeString() 
{
   char           buff[32];
   m5::rtc_time_t time_struct;
   
   M5.Rtc.getTime(&time_struct);

   sprintf(buff,"%02d:%02d:%02d",      
      time_struct.hours, time_struct.minutes, time_struct.seconds);

   return String(buff);
}

/* Convert the DateTime to the IoBroker ISO8601 format */
String getIoBrokerDateTimeString(DateTime dateTime)
{
   char buff[40];
   
   sprintf(buff,"%04d-%02d-%02dT%02d:%02d:%02dZ",
      dateTime.year(), dateTime.month(),  dateTime.day(),
      dateTime.hour(), dateTime.minute(), dateTime.second());

   return String(buff);
}

/* Convert the time_t to the date part DD.MM.YYYY */
String getDateString(time_t rawtime)
{
   char buff[32];
   
   sprintf(buff,"%02d.%02d.%04d",
      day(rawtime), month(rawtime), year(rawtime));

   return String(buff);
}

/* Convert the time_t to the time part HH:MM:SS format */
String getTimeString(time_t rawtime)
{
   char buff[32];
   
   sprintf(buff,"%02d:%02d:%02d",
      hour(rawtime), minute(rawtime), second(rawtime));

   return String(buff);
}

/* Convert the hour of the time_t */
String getHourString(time_t rawtime)
{
   char buff[32];
   
   sprintf(buff,"%02d",
      hour(rawtime));

   return String(buff);
}

/* Convert the hour and minute of the time_t */
String getHourMinString(time_t rawtime)
{
   char buff[32];
   
   sprintf(buff,"%02d:%02d",
      hour(rawtime), minute(rawtime));

   return String(buff);
}

/* Convert the RSSI value to a string value between 0 and 100 % */
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

/* Convert the RSSI value to an int value between 0 and 100 % */
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
