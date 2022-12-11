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
#include <TimeLib.h>
#include <ArduinoJson.h>

#define IOBROKER_QUERY "/query/"
#define IOBROKER_GET   "/get/"
#define IOBROKER_PLAIN "/getPlainValue/"


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
   bool   getPPVHistory(DynamicJsonDocument &doc);
   
public:
   IoBroker();
   ~IoBroker();

   bool getDateTime  (String &dateTime, String topic);
   bool getPlainValue(String &value,    String topic);
   bool getPlainValue(double &value,    String topic);
   bool getPPVHistory();
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
      delay(5);
      if (c >= 0) return c;
   } while(millis() - _startMillis < _timeout);
   return -1;     // -1 indicates timeout
} 

/* Read a string. */
size_t IoBroker::readChars(String &string, size_t length)
{
   size_t count = 0;

   // Serial.println("len: " + String(length)); 
   while (count < length) {
      int c = timedRead();
      if (c < 0) {
         // Serial.println("break!");
         break;
      }
      string += (char) c;
      count++;
   }
   // Serial.println("data: <" + string + ">"); 
   if (client.available()) {
      String rest = client.readString();
      Serial.println("rest: " + String(rest));
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

/* Read a dateTime value. */
bool IoBroker::getDateTime(String &dateTime, String topic) 
{
   bool ret = false;

   Serial.println("getDateTime()...");
   if (!client.connected()) {
      connect();
   }
   if (client.connected()) {
      int    len = 0;
      String url = (String) IOBROKER_GET + topic;
            
      // This will send the request to the server
      Serial.println("Send request! " + url);
      client.print((String) "GET " + url + " HTTP/1.1\r\n\r\n");
      client.flush();

      waitForAvailable();
      while (client.available()) {
         String line = client.readStringUntil('\n');
         
         parseContentLen(len , line);
         if (line == "\r") {
            break;
         }
      }    
      while (client.available()) {
         String tsBegin = "\"lc\":";
         String tsEnd   = "\"";
         String body    = readString(len);

         int index = body.indexOf(tsBegin);
         if (index != -1) {
            int indexEnd = body.indexOf(tsEnd, index + tsBegin.length());

            if (indexEnd) {
               String ts = body.substring(index + tsBegin.length(), indexEnd - 1);

               if (ts.length() > 0) {
                  char      buff[32];
                  long long dateTimeMS = atoll(ts.c_str());
                  time_t    datetime   = dateTimeMS / 1000;
      
                  sprintf(buff, "%02d.%02d.%02d %02d:%02d:%02d", 
                     day(datetime),  month(datetime),  year(datetime), 
                     hour(datetime), minute(datetime), second(datetime));

                  dateTime = buff;
                  ret = true;
               }
            }
         }
      }    
      client.flush();
   }
   return ret;
}

/* Read a plain value. */
bool IoBroker::getPlainValue(double &value, String topic) 
{
   bool ret = false;

   Serial.println("getPlainValue()...");
   if (!client.connected()) {
      connect();
   }
   if (client.connected()) {
      int    len = 0;
      String url = (String) IOBROKER_PLAIN + topic;
            
      // This will send the request to the server
      Serial.println("Send request! " + url);
      client.print((String) "GET " + url + " HTTP/1.1\r\n\r\n");
      client.flush();

      waitForAvailable();
      while (client.available()) {
         String line  = client.readStringUntil('\n');

         parseContentLen(len , line);
         if (line == "\r") {
            break;
         }
      }    
      while (client.available()) {
         String body = readString(len);
         
         Serial.println(body);
         value = body.toDouble();
         ret   = true;
      }    
      client.flush();
   }
   return ret;
}

/* Read a plain value. */
bool IoBroker::getPlainValue(String &value, String topic) 
{
   bool ret = false;

   Serial.println("getPlainValue()...");
   if (!client.connected()) {
      connect();
   }
   if (client.connected()) {
      int    len = 0;
      String url = (String) IOBROKER_PLAIN + topic;
            
      // This will send the request to the server
      Serial.println("Send request! " + url);
      client.print((String) "GET " + url + " HTTP/1.1\r\n\r\n");
      client.flush();

      waitForAvailable();
      while (client.available()) {
         String line  = client.readStringUntil('\n');

         parseContentLen(len , line);
         if (line == "\r") {
            break;
         }
      }    
      while (client.available()) {
         String body = readString(len);
         
         Serial.println(body);
         value = body;
         ret   = true;
      }    
      client.flush();
   }
   return ret;
}

/* Reads the history in the json object. */
bool IoBroker::getPPVHistory(DynamicJsonDocument &doc)
{
   return true;
}

/* Reads the solar panel power over a specific time period. */
bool IoBroker::getPPVHistory()
{
   DynamicJsonDocument doc(35 * 1024);
   
   if (getPPVHistory(doc)) {
      // return Fill(doc.as<JsonObject>());
   }
   return false;
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

   ioBroker.getPPVHistory();
}
