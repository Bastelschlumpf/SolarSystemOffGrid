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
  * @file Battery.h
  * 
  * Helperfunctions for reading the battery value.
  */
#pragma once

#include "Data.h"

/**
  * Read the battery voltage
  */
bool GetBatteryValues(MyData &myData)
{
   myData.batteryVolt     = M5.Power.getBatteryVoltage();
   myData.batteryCapacity = M5.Power.getBatteryLevel();

   Serial.println("batteryVolt: "     + String(myData.batteryVolt));
   Serial.println("batteryCapacity: " + String(myData.batteryCapacity));

   return true;
}