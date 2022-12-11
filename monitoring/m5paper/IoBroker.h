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
  * @file IoBroker.h
  * 
  * Helper function to communicate with the IoBroker.
  */
#pragma once
#include "RTClib.h"
#include <TimeLib.h>
#include <HTTPClient.h>

#define IOBROKER_QUERY     "/query/"
#define IOBROKER_GET       "/get/"
#define IOBROKER_GET_PLAIN "/getPlainValue/"


/* Helper class for getting data from IoBroker. */
class IoBroker
{
public:
   WiFiClient client;

protected:
   bool   connect();
   void   waitForAvailable();
   int    timedRead();
   size_t readChars(String &string, size_t length);
   String readString(int len);
   void   parseContentLen(int &len, String line);
   bool   parsPPVHistory(float ppvHistory[], float &ppvMax, String historyData, DateTime fromDate, DateTime toDate);
   
public:
   IoBroker();
   ~IoBroker();

   bool getPlainValue(String &value,       String topic, String method = IOBROKER_GET_PLAIN, String param = "");
   bool getPlainValue(double &value,       String topic, String method = IOBROKER_GET_PLAIN, String param = "");
   bool getPPVHistory(float ppvHistory[], float &ppvMax, String topic);
};

/* Constructor: Connect to the IoBroker server. */
IoBroker::IoBroker()
{
   connect();
}

/* Destructor: Disconnect. */
IoBroker::~IoBroker()
{
   if (client.connected()) {
      client.stop();
   }
}

/* Connect to the specific IoBroker server. */
bool IoBroker::connect()
{
   Serial.println();
   Serial.println("ty to connect!");
   if (!client.connect(IOBROKER_URL, IOBROKER_PORT)) {
      Serial.println("connection failed!");
   } else {
      Serial.println("connected!");
      delay(100);
   }
}

/* Wait until the server is available. */
void IoBroker::waitForAvailable()
{
   unsigned long _startMillis = millis();
   unsigned long _timeout     = client.getTimeout();

   while (!client.available()) {
      if (millis() - _startMillis > _timeout) {
         Serial.println("timeout!");          
         client.stop(); 
         break;
      }
   }
}

/* Read one item.. */
int IoBroker::timedRead()
{
   int c;
   unsigned long _startMillis = millis();
   unsigned long _timeout     = client.getTimeout();
   
   do {
      c = client.read();
      //delay(5);
      if (c >= 0) return c;
   } while(millis() - _startMillis < _timeout);
   return -1;     // -1 indicates timeout
} 

/* Read a string. */
size_t IoBroker::readChars(String &string, size_t length)
{
   size_t count = 0;

   //Serial.println("len: " + String(length)); 
   while (count < length) {
      int c = timedRead();
      if (c < 0) {
         Serial.println("break!");
         break;
      }
      string += (char) c;
      count++;
   }
   //Serial.println("data: <" + string + ">"); 
   if (client.available()) {
      String rest = client.readString();
      //Serial.println("rest: " + String(rest));
   }
   return count;
} 

/* Read a string. */
String IoBroker::readString(int len)
{
   String ret;
   
   if (len > 0) {
      readChars(ret, len);
   } else {
      ret = client.readString();
   }
   return ret;
}

/* Read the Content-Length header. */
void IoBroker::parseContentLen(int &len, String line)
{
   String lenString = "Content-Length:";
   int    index     = line.indexOf(lenString);
   
   if (index != -1) {
      len = atoi(line.substring(index + lenString.length()).c_str());
   }
}

/* Read a String from IoBroker. */
bool IoBroker::getPlainValue(String &string, String topic, String method /*= IOBROKER_GET_PLAIN*/, String param /*= ""*/) 
{
   bool ret = false;

   Serial.println("getPlainValue()...");
   if (!client.connected()) {
      connect();
   }
   if (client.connected()) {
      int    len = 0;
      String url = method + topic + param;
            
      // This will send the request to the server
      Serial.println("Send request! " + url);
      client.print((String) "GET " + url + " HTTP/1.1\r\n\r\n");
      client.flush();

      waitForAvailable();
      while (client.available()) {
         String line = client.readStringUntil('\n');

         parseContentLen(len, line);
         if (line == "\r") {
            break;
         }
      }    
      while (client.available()) {
         //Serial.println(len);
         string += readString(len);
         //Serial.println("read...");
         ret = true;
      }    
      client.flush();
      if (string.length() < 100) {
         Serial.println("Plain value: " + string);
      } else {
         Serial.println("Plain value: " + string.substring(0, 100) + "...");
      }
   }
   return ret;
}

/* Read a plain value. */
bool IoBroker::getPlainValue(double &value, String topic, String method /*= IOBROKER_GET_PLAIN*/, String param /*= ""*/) 
{
   String plainString;

   if (getPlainValue(plainString, topic, method)) {
      value = plainString.toDouble();
      return true;
   }
   return false;
}

/* Reads the solar panel power over a specific time period. */
bool IoBroker::getPPVHistory(float ppvHistory[], float &ppvMax, String topic)
{
   DateTime toDay    = GetRTCTime();
   DateTime toDate   = toDay  + TimeSpan( 1, 0, 0, 0);
   DateTime fromDate = toDate - TimeSpan(30, 0, 0, 0);
   String   plainString;
   String   param = "?dateFrom=" + String(fromDate.year()) + "-" + String(fromDate.month()) + "-" + String(fromDate.day()) + 
                    "&dateTo="   + String(toDate.year())   + "-" + String(toDate.month())   + "-" + String(toDate.day());

   if (getPlainValue(plainString, topic, IOBROKER_QUERY, param)) {
      return parsPPVHistory(ppvHistory, ppvMax, plainString, fromDate, toDate);
   }
   return false;
}

/* Pars the history json data, analyse it and put the result into the ppvHistoryArray. */
bool IoBroker::parsPPVHistory(float ppvHistory[], float &ppvMax, String historyData, DateTime fromDate, DateTime toDate)
{
   int historyCount[PPV_HISTORY_SIZE];
   int index = historyData.indexOf('[');

   ppvMax = 0.0;
   memset(ppvHistory,   0, sizeof(ppvHistory));
   memset(historyCount, 0, sizeof(historyCount));
  
   while (index >= 0) {
      if (isDigit(historyData[index + 1])) {
         int sep = historyData.indexOf(',', index + 1);

         if (sep >= 0) {
            int end = historyData.indexOf(']', sep + 1);

            if (end >= 0) {
               String value     = historyData.substring(index + 1, sep);
               String timestamp = historyData.substring(sep + 1,   end - 3); // no milliseconds

               DateTime jsonDate(timestamp.toInt());

               int historyIndex = (double) PPV_HISTORY_SIZE / (double) (toDate.secondstime() - fromDate.secondstime()) * (double) (jsonDate.secondstime() - fromDate.secondstime());

               if (historyIndex >= 0 && historyIndex < PPV_HISTORY_SIZE) {
                  ppvHistory[historyIndex] += value.toFloat();
                  historyCount[historyIndex]++;
               } else {
                  Serial.println("Wrong history index! [" + String(historyIndex) + ']');
               }
               Serial.printf("**** Index: %d Value: %s/%f Timestamp: %d-%d-%d %d:%d:%d\n", historyIndex, value.c_str(), value.toFloat(), jsonDate.year(), jsonDate.month(), jsonDate.day(), jsonDate.hour(), jsonDate.minute(), jsonDate.second());
            }
         }
      }
      index = historyData.indexOf('[', index + 1);
   }
   for (int i = 0; i < PPV_HISTORY_SIZE; i++) {
      if (historyCount[i] > 0) {
         ppvHistory[i] = ppvHistory[i] / historyCount[i];
         if (ppvMax < ppvHistory[i]) {
            ppvMax = ppvHistory[i];
         }
      }
   }

   return true;
}

/* Helper Funktion to read all the IoBroker data into the data object. */
void GetIoBrokerValues(MyData &myData)
{
   IoBroker ioBroker;

   ioBroker.getPlainValue(myData.bmv.consumedAmpHours,           "mqtt.0.bmv.CE");
   ioBroker.getPlainValue(myData.bmv.stateOfCharge,              "mqtt.0.bmv.SOC");
   ioBroker.getPlainValue(myData.bmv.midPointDeviation,          "mqtt.0.bmv.DM");
   ioBroker.getPlainValue(myData.bmv.numberOfChargeCycles,       "mqtt.0.bmv.H4");
   ioBroker.getPlainValue(myData.bmv.dischargedEnergy,           "mqtt.0.bmv.H17");
   ioBroker.getPlainValue(myData.bmv.chargedEnergy,              "mqtt.0.bmv.H18");
   ioBroker.getPlainValue(myData.bmv.cumulativeAmpHoursDrawn,    "mqtt.0.bmv.H6");
   ioBroker.getPlainValue(myData.bmv.secondsSinceLastFullCharge, "mqtt.0.bmv.H9");
   ioBroker.getPlainValue(myData.bmv.batteryCurrent,             "mqtt.0.bmv.I");
   ioBroker.getPlainValue(myData.bmv.instantaneousPower,         "mqtt.0.bmv.P");
   ioBroker.getPlainValue(myData.bmv.relay,                      "mqtt.0.bmv.Relay");
   ioBroker.getPlainValue(myData.bmv.timeToGo,                   "mqtt.0.bmv.TTG");
   ioBroker.getPlainValue(myData.bmv.mainVoltage,                "mqtt.0.bmv.V");

   ioBroker.getPlainValue(myData.mppt.stateOfOperation,          "mqtt.0.mppt.CS");
   ioBroker.getPlainValue(myData.mppt.yieldTotal,                "mqtt.0.mppt.H19");
   ioBroker.getPlainValue(myData.mppt.yieldToday,                "mqtt.0.mppt.H20");
   ioBroker.getPlainValue(myData.mppt.maximumPowerToday,         "mqtt.0.mppt.H21");
   ioBroker.getPlainValue(myData.mppt.yieldYesterday,            "mqtt.0.mppt.H22");
   ioBroker.getPlainValue(myData.mppt.maximumPowerYesterday,     "mqtt.0.mppt.H23");
   ioBroker.getPlainValue(myData.mppt.batteryCurrent,            "mqtt.0.mppt.I");
   ioBroker.getPlainValue(myData.mppt.panelPower,                "mqtt.0.mppt.PPV");
   ioBroker.getPlainValue(myData.mppt.mainVoltage,               "mqtt.0.mppt.V");
   ioBroker.getPlainValue(myData.mppt.panelVoltage,              "mqtt.0.mppt.VPV");

   ioBroker.getPPVHistory(myData.ppvHistory, myData.ppvMax, "mqtt.0.mppt.PPV");
}
