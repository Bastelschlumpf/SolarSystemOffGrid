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
  * @file Config.h
  *
  * Public configuration information. 
  * Could be set before starting the program or via web interface.
  */

#define WIFI_SID      "sid"                    //!< WiFi SID
#define WIFI_PW       "password"               //!< WiFi password

#define MQTT_NAME     "SolarSystemOffGrid"     //!< MQTT name
#define MQTT_SERVER   "server"                 //!< MQTT Server URL
#define MQTT_PORT     1883                     //!< MQTT Port (Default is 1883)
#define MQTT_USER     "user"                   //!< MQTT connection user
#define MQTT_PASSWORD "password"               //!< MQTT connection password
