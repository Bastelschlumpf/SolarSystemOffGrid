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
  * @file EPDWifi.h
  * 
  * Helper functions for the wifi connection.
  */
#pragma once
#include <WiFi.h>

/* Connect to wifi */
bool ConnectToWifi(String ssid, String pw) 
{
   Serial.print("Connecting to ");
   Serial.println(ssid);
   delay(100);
   
   WiFi.begin(ssid.c_str(), pw.c_str());
   for (int retry = 0; WiFi.status() != WL_CONNECTED && retry < 30; retry++) {
      delay(500);
      Serial.print(".");
   }
   if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nWiFi connected at: " + WiFi.localIP().toString());
      return true;
   } else {
      Serial.println("\nWiFi connection *** FAILED ***");
      return false;
   }
}

/* Start and connect to the wifi */
bool StartWiFi(int &rssi) 
{
   IPAddress dns(8, 8, 8, 8); // Google DNS
   
   WiFi.mode(WIFI_STA);
   WiFi.disconnect();
   WiFi.setAutoConnect(true);
   WiFi.setAutoReconnect(true);

   rssi = 0;
   if (ConnectToWifi(WIFI_SSID_1, WIFI_PW_1) || ConnectToWifi(WIFI_SSID_2, WIFI_PW_2)) {
      rssi = WiFi.RSSI();
      return true;
   } else {
      return false;
   }
}

/* Stop the wifi connection */
void StopWiFi() 
{
   Serial.println("Stop WiFi");
   WiFi.disconnect();
   WiFi.mode(WIFI_OFF);
}
