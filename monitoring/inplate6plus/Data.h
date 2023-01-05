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
  * @file Data.h
  * 
  * Class with all the global runtime data.
  */
#pragma once

#include <nvs.h>
#include "Utils.h"

#define CHARGE_HISTORY_SIZE 775
#define PPV_HISTORY_SIZE    775
#define GRID_HISTORY_SIZE   340

const DateTime EmptyDateTime(2000, 1, 1, 0, 0, 0);

/**
  * HistoryData: A collection af history float values with its DateTime position.
  */
class HistoryData
{
public:
   int       size_;     //!< Size of the history items.
   float    *values_;   //!< Histpry double values.
   DateTime *dates_;    //!< History timeline.
   String    unitName_; //!< Unit Name of the values
   int      *counts_;   //!< Occurence.
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

   void clear()
   {
      memset(values_, 0, size_ * sizeof(float));
      memset(dates_,  0, size_ * sizeof(DateTime));
      memset(counts_, 0, size_ * sizeof(int));
      max_ = 0.0;
   }
};

/**
  * BMV data.
  */
class BMV
{
public:
   double      consumedAmpHours;           //!< CE (mAh)
   double      stateOfCharge;              //!< SOC (promille)
   double      midPointDeviation;          //!< DM (promille)
   double      numberOfChargeCycles;       //!< H4 
   double      dischargedEnergy;           //!< H17 (0.01 kWh)
   double      chargedEnergy;              //!< H18 (0.01 kWh)
   double      cumulativeAmpHoursDrawn;    //!< H6 (mAh)
   double      secondsSinceLastFullCharge; //!< H9 (Seconds)
   double      batteryCurrent;             //!< I (mA)
   double      instantaneousPower;         //!< P (W)
   String      relay;                      //!< Relay state (ON | OFF)
   double      timeToGo;                   //!< TTG (Minutes)
   double      mainVoltage;                //!< V (mV)
   double      alarmReason;                //!< Alarm?!
   DateTime    lastChange;                 //!< Last change of the data

   HistoryData chargeHistory;              //!< SOC charge statistic

public:
   BMV()
      : consumedAmpHours(0.0)
      , stateOfCharge(0.0)
      , midPointDeviation(0.0)
      , numberOfChargeCycles(0.0)
      , dischargedEnergy(0.0)
      , chargedEnergy(0.0)
      , cumulativeAmpHoursDrawn(0.0)
      , secondsSinceLastFullCharge(0.0)
      , batteryCurrent(0.0)
      , instantaneousPower(0.0)
      , relay("")
      , timeToGo(0.0)
      , mainVoltage(0.0)
      , alarmReason(0.0)
      , lastChange(EmptyDateTime)
      , chargeHistory(CHARGE_HISTORY_SIZE, "%")
   {}
   
   void Dump();
};

/**
  * MPPT data.
  */
class MPPT
{
public:   
   double      stateOfOperation;      //!< CS (Off 0, Low power 1, Fault 2, Bulk 3, Absorption 4, Float 5, Inverting 9)
   double      yieldTotal;            //!< H19 (0.01 kWh)
   double      yieldToday;            //!< H20 (0.01 kWh)
   double      maximumPowerToday;     //!< H21 (W)
   double      yieldYesterday;        //!< H22 (0.01 kWh)
   double      maximumPowerYesterday; //!< H23 (W)
   double      batteryCurrent;        //!< I (mA)
   double      panelPower;            //!< PPV (W)
   double      mainVoltage;           //!< V (mV)
   double      panelVoltage;          //!< VPV (mV)
   double      errorCode;             //!< error?!
   DateTime    lastChange;            //!< Last change of the data

   HistoryData ppvHistory;            //!< Panel power history
   HistoryData yieldHistory;          //!< Yield history
   
public:
   MPPT()
      : stateOfOperation(0.0)
      , yieldTotal(0.0)
      , yieldToday(0.0)
      , maximumPowerToday(0.0)
      , yieldYesterday(0.0)
      , maximumPowerYesterday(0.0)
      , batteryCurrent(0.0)
      , panelPower(0.0)
      , mainVoltage(0.0)
      , panelVoltage(0.0)
      , errorCode(0.0)
      , lastChange(EmptyDateTime)
      , ppvHistory(PPV_HISTORY_SIZE, "W")
      , yieldHistory(PPV_HISTORY_SIZE, "kWh")
   {
   }

   void Dump();
};

/**
  * TasmotaElite data.
  */
class TasmotaElite
{
public:   
   double      voltage;      //!< grid voltage
   double      ampere;       //!< grid power consumption
   double      power;        //!< consumption
   DateTime    lastChange;   //!< Last change of the data
   String      alive;        //!< Switched on or off

   HistoryData powerHistory; //!< Grid power history
   HistoryData yieldHistory; //!< Grid power history

public:
   TasmotaElite()
      : voltage(0.0)
      , ampere(0.0)
      , power(0.0)
      , lastChange(EmptyDateTime)
      , powerHistory(GRID_HISTORY_SIZE, "W")
      , yieldHistory(GRID_HISTORY_SIZE, "kWh")
   {
   }

   void Dump();
};

/**
  * Class for collecting all the global data.
  */
class MyData
{
public:
   uint16_t     nvsCounter;       //!< Non volatile counter

   int          wifiRSSI;         //!< The wifi signal strength
   float        batteryVolt;      //!< The current battery voltage
   int          batteryCapacity;  //!< The current battery capacity
   int          sht30Temperatur;  //!< SHT30 temperature

   BMV          bmv;              //!< The BMW data
   MPPT         mppt;             //!< The MPPT data
   TasmotaElite tasmotaElite;     //!< The Tasmota Elite data

public:
   MyData()
      : wifiRSSI(0)
      , batteryVolt(0.0)
      , batteryCapacity(0)
      , sht30Temperatur(0)
   {
   }

   void Dump();
   void LoadNVS();
   void SaveNVS();
};


/* helper function to dump all the collected data */
void MyData::Dump()
{
   Serial.println("DateTime: "         + getRTCDateTimeString());
    
   Serial.println("WifiRSSI: "         + String(wifiRSSI));
   Serial.println("BatteryVolt: "      + String(batteryVolt));
   Serial.println("BatteryCapacity: "  + String(batteryCapacity));
   Serial.println("Sht30Temperatur: "  + String(sht30Temperatur));
   
   bmv.Dump();
   mppt.Dump();
}

/* Load the NVS data from the non volatile memory */
void MyData::LoadNVS()
{
   nvs_handle nvs_arg;
   nvs_open("Setting", NVS_READONLY, &nvs_arg);
   nvs_get_u16(nvs_arg, "nvsCounter", &nvsCounter);
   nvs_close(nvs_arg);
}

/* Store the NVS data to the non volatile memory */
void MyData::SaveNVS()
{
   nvs_handle nvs_arg;
   nvs_open("Setting", NVS_READWRITE, &nvs_arg);
   nvs_set_u16(nvs_arg, "nvsCounter", nvsCounter);
   nvs_commit(nvs_arg);
   nvs_close(nvs_arg);
}

/* helper function to dump all the collected data */
void BMV::Dump()
{
   Serial.println("[bmv] consumedAmpHours: "           + String(consumedAmpHours));
   Serial.println("[bmv] stateOfCharge: "              + String(stateOfCharge));
   Serial.println("[bmv] midPointDeviation: "          + String(midPointDeviation));
   Serial.println("[bmv] numberOfChargeCycles: "       + String(numberOfChargeCycles));
   Serial.println("[bmv] dischargedEnergy: "           + String(dischargedEnergy));
   Serial.println("[bmv] chargedEnergy: "              + String(chargedEnergy));
   Serial.println("[bmv] cumulativeAmpHoursDrawn: "    + String(cumulativeAmpHoursDrawn));
   Serial.println("[bmv] secondsSinceLastFullCharge: " + String(secondsSinceLastFullCharge));
   Serial.println("[bmv] batteryCurrent: "             + String(batteryCurrent));
   Serial.println("[bmv] instantaneousPower: "         + String(instantaneousPower));
   Serial.println("[bmv] relay: "                      + relay);
   Serial.println("[bmv] timeToGo: "                   + String(timeToGo));
   Serial.println("[bmv] mainVoltage: "                + String(mainVoltage));
   Serial.println("[bmv] alarmReason: "                + String(alarmReason));
}

/* helper function to dump all the collected data */
void MPPT::Dump()
{
   Serial.println("[mppt] stateOfOperation: "      + String(stateOfOperation));
   Serial.println("[mppt] yieldTotal: "            + String(yieldTotal));
   Serial.println("[mppt] yieldToday: "            + String(yieldToday));
   Serial.println("[mppt] maximumPowerToday: "     + String(maximumPowerToday));
   Serial.println("[mppt] yieldYesterday: "        + String(yieldYesterday));
   Serial.println("[mppt] maximumPowerYesterday: " + String(maximumPowerYesterday));
   Serial.println("[mppt] batteryCurrent: "        + String(batteryCurrent));
   Serial.println("[mppt] panelPower: "            + String(panelPower));
   Serial.println("[mppt] mainVoltage: "           + String(mainVoltage));
   Serial.println("[mppt] panelVoltage: "          + String(panelVoltage));
   Serial.println("[mppt] errorCode: "             + String(errorCode));
}

/* helper function to dump all the collected data */
void TasmotaElite::Dump()
{
   Serial.println("[elite] voltage: " + String(voltage));
   Serial.println("[elite] ampere: "  + String(ampere));
   Serial.println("[elite] power: "   + String(power));
}
