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
  * @file vedirect.ino 
  *
  * ve.direct to MQQT bridge
  */
#include <WiFi.h>
#include <PubSubClient.h>
#include <HardwareSerial.h>
#include "vereader.h"

#include "Config.h"
#define USE_CONFIG_OVERRIDE //!< Switch to use ConfigOverride
#ifdef USE_CONFIG_OVERRIDE
  #include "ConfigOverride.h"
#endif

WiFiClient     wifiClient;
PubSubClient   pubSubClient(wifiClient);

VEDirectReader veDirectReader1(Serial1);
VEDirectReader veDirectReader2(Serial2);

#define SEND_EVEREY_MILLIS  10000
#define SEND_STATUS_MILLIS  60000
#define LED_PIN                 2

int     bmvBlockCompleted  = 0;
int     bmvCheckSumOk      = 0;
int     bmvCheckSumError   = 0;
int     bmvMqqtSend        = 0;
int     mpptBlockCompleted = 0;
int     mpptCheckSumOk     = 0;
int     mpptCheckSumError  = 0;
int     mpptMqqtSend       = 0;

/** Connect to the WiFi network */
void SetupWifi() 
{
   delay(10);
   // We start by connecting to a WiFi network
   Serial.println();
   Serial.print("Connecting to ");
   Serial.println(WIFI_SID);

   WiFi.begin(WIFI_SID, WIFI_PW);

   // Wait max 10 seconds for connection
   for (int i = 0; WiFi.status() != WL_CONNECTED && i < 20; i++) {  
      delay(500);
      Serial.print(".");
   }
   Serial.println("");
   if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi connection failed!!!");
   } else {
      Serial.println("WiFi connected");
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
   }
}

/** Set the MQQT server */
void SetupMqqt()
{
   pubSubClient.setServer(MQTT_SERVER, MQTT_PORT);
}

/** Checks the wifi and mqqt connection and connect if needed. */
void Reconnect() 
{
   // Reconnect WiFi if needed
   if (WiFi.status() != WL_CONNECTED) {
      WiFi.reconnect();
   }
   
   // Loop until we're reconnected (5 retries)
   for (int i = 0; !pubSubClient.connected() && i < 5; i++) {  
      Serial.print("Attempting MQTT connection...");
      // Attempt to connect
      // If you do not want to use a username and password, change next line to
      // if (client.connect(MQTT_NAME)) {
      if (pubSubClient.connect(MQTT_NAME, MQTT_USER, MQTT_PASSWORD)) {
         Serial.println("connected");
      } else {
         Serial.print("failed, rc=");
         Serial.print(pubSubClient.state());
         Serial.println(" try again in 5 seconds");
         // Wait 5 seconds before retrying
         delay(5000);
      }
   }
   if (!pubSubClient.connected()) {
      Serial.println("MQTT failed!!!");
   }
}

/** Publish one value to the server. */
void Publish(String prefix, String keyword, String value)
{
   // Ignore empty keywords and ignore hex data ':'
   if (!keyword.isEmpty() && keyword[0] != ':') {
      String topic = prefix + "/" + keyword;

      Serial.println((String) "publish: [" + topic + "]=[" + value + "]");
      pubSubClient.publish(topic.c_str(), value.c_str(), true);
      // We need a small delay here otherwise the values will not arrive correctly.
      delay(10);
      yield();
   }
}

/** Main setup function. */
void setup() 
{
   Serial.begin(19200);
   Serial1.begin(19200, SERIAL_8N1, 27, 26);
   Serial2.begin(19200);
   pinMode(LED_PIN, OUTPUT);
   SetupWifi();
   SetupMqqt();
}

/** Main loop
  * Reconnect to the MQQT server if needed.
  * Read the ve.direct serial lines.
  * If the block ist completed and the checksum is ok, 
  * send the keyword value pairs to the server.
  */
void loop() 
{
   static unsigned long ms1 = 0;
   static unsigned long ms2 = 0;
   static unsigned long ms3 = 0;

   veDirectReader1.readLine();
   veDirectReader2.readLine();
   digitalWrite(LED_PIN, LOW);

   if (veDirectReader1.isBlockCompleted()) {
      bmvBlockCompleted++;
      Serial.println("BMV block completed");
      if (!veDirectReader1.isCheckSumOk()) {
         Serial.println(" -> Checksum Error!");
         bmvCheckSumError++;
      } else {
         bmvCheckSumOk++;
         digitalWrite(LED_PIN, HIGH);
         delay(10);
         // Send only every x seconds
         if (millis() - ms1 > SEND_EVEREY_MILLIS) {
            ms1 = millis();
   
            if (!pubSubClient.connected()) {
               Reconnect();
            }
            if (pubSubClient.connected()) {
               Serial.println("BMV: publish to mqqt server.");
               for (int i = 0; i < veDirectReader1.getValueCount(); i++) {
                  Publish("bmv", veDirectReader1.keywords_[i], veDirectReader1.values_[i]);
               }   
               pubSubClient.loop();
               bmvMqqtSend++;
            }
         }
      }
   }
   
   if (veDirectReader2.isBlockCompleted()) {
      mpptBlockCompleted++;
      Serial.println("MPPT block completed");
      if (!veDirectReader2.isCheckSumOk()) {
         Serial.println(" -> Checksum Error!");
         mpptCheckSumError++;
      } else {
         mpptCheckSumOk++;
         digitalWrite(LED_PIN, HIGH);
         delay(10);
         // Send only every x seconds
         if (millis() - ms2 > SEND_EVEREY_MILLIS) {
            ms2 = millis();
            
            if (!pubSubClient.connected()) {
               Reconnect();
            }
            if (pubSubClient.connected()) {
               Serial.println("MPPT: publish to mqqt server.");
               for (int i = 0; i < veDirectReader2.getValueCount() - 1; i++) {
                  Publish("mppt", veDirectReader2.keywords_[i], veDirectReader2.values_[i]);
               }   
               pubSubClient.loop();
               mpptMqqtSend++;
            }
         }
      }
   }
   
   // Send only every x seconds
   if (millis() - ms3 > SEND_STATUS_MILLIS) {
      ms3 = millis();
      
      if (!pubSubClient.connected()) {
         Reconnect();
      }
      if (pubSubClient.connected()) {
         Serial.println("SOLAR: publish status to mqqt server.");
         Publish("bmv/Statistic",  "BlockCompleted", String(bmvBlockCompleted));
         Publish("bmv/Statistic",  "CheckSumOk",     String(bmvCheckSumOk));
         Publish("bmv/Statistic",  "CheckSumError",  String(bmvCheckSumError));
         Publish("bmv/Statistic",  "MqqtSend",       String(bmvMqqtSend));
         Publish("mppt/Statistic", "BlockCompleted", String(mpptBlockCompleted));
         Publish("mppt/Statistic", "CheckSumOk",     String(mpptCheckSumOk));
         Publish("mppt/Statistic", "CheckSumError",  String(mpptCheckSumError));
         Publish("mppt/Statistic", "MqqtSend",       String(mpptMqqtSend));
         pubSubClient.loop();
      }
   }
   yield();
   delay(10); 
}
