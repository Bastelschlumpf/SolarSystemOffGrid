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
  * @file Display.h
  * 
  * Main class for drawing the content to the e-paper display.
  */
#pragma once
#include "Data.h"
#include "Icons.h"

#define GRAY_0 0
#define GRAY_1 1
#define GRAY_2 2
#define GRAY_3 3
#define GRAY_4 4
#define GRAY_5 5
#define GRAY_6 6
#define GRAY_7 7

#define X_FACTOR (1024.0 / 960.0)
#define Y_FACTOR ( 758.0 / 540.0)

Inkplate display(INKPLATE_3BIT); // Main display of the e-paper

/* Main class for drawing the content to the e-paper display. */
class SolarDisplay
{
protected:
   MyData &myData; //!< Reference to the global data
   int     maxX;   //!< Max width of the e-paper
   int     maxY;   //!< Max height of the e-paper

protected:
   void   DrawString            (String text, int x, int y);
   void   DrawCentreString      (String text, int x, int y);
   void   DrawRightString       (String text, int x, int y);
   void   DrawCircle            (int32_t x, int32_t y, int32_t r, uint32_t color, int32_t degFrom = 0, int32_t degTo = 360);
   void   DrawIcon              (int x, int y, const uint16_t *icon, int dx = 64, int dy = 64, bool highContrast = false);
   void   DrawGraph             (int x, int y, int dx, int dy, HistoryData &powerHistory, HistoryData &yieldHistory, HistoryData *chargeHistory = NULL);
   String FormatString          (String format, double data, int fillLen = 8);
   String StateOfOperation      (int value);
   String BmvAlarmReason        (int value);   
   String MpptErrorCode         (int value);

   void   DrawHead              (int x, int y, int dx, int dy);
   void   DrawHeadVersion       (int x, int y);
   void   DrawHeadUpdated       (int x, int y);
   void   DrawHeadRSSI          (int x, int y);
   void   DrawHeadBattery       (int x, int y);
   void   DrawBody              (int x, int y, int dx, int dy);
   void   DrawBatteryInfo       (int x, int y, int dx, int dy);
   void   DrawSolarSymbol       (int x, int y, int dx, int dy);
   void   DrawSolarArrow        (int x, int y, int dx, int dy);
   void   DrawGridInfo          (int x, int y, int dx, int dy);
   void   DrawBatterySymbol     (int x, int y, int dx, int dy);
   void   DrawBatteryArrow      (int x, int y, int dx, int dy);
   void   DrawInverterSymbol    (int x, int y, int dx, int dy);
   void   DrawInverterArrow     (int x, int y, int dx, int dy);
   void   DrawHouseSymbol       (int x, int y, int dx, int dy);
   void   DrawGridArrow         (int x, int y, int dx, int dy);
   void   DrawGridSymbol        (int x, int y, int dx, int dy);
   void   DrawSolarInfo         (int x, int y, int dx, int dy);

public:
   SolarDisplay(MyData &md, int x = 1024, int y = 758)
      : myData(md)
      , maxX(x)
      , maxY(y)
   {
   }

   void Show();
   void ShowWiFiError(String ssid);
};

/* Draw a text to a specific position. */
void SolarDisplay::DrawString(String text, int x, int y)
{
   display.setCursor(x, y);
   display.print(text);
}

/* Draw a text centered to a specific position. */
void SolarDisplay::DrawCentreString(String text, int x, int y)
{
   int16_t  x1 = 0;
   int16_t  y1 = 0;
   uint16_t w  = 0;
   uint16_t h  = 0;
   
   display.getTextBounds(text.c_str(), x, y, &x1, &y1, &w, &h);
   display.setCursor(x - w / 2, y);
   display.print(text);
}

/* Draw a text right aligned to a specific position. */
void SolarDisplay::DrawRightString(String text, int x, int y)
{
   int16_t  x1 = 0;
   int16_t  y1 = 0;
   uint16_t w  = 0;
   uint16_t h  = 0;
   
   display.getTextBounds(text.c_str(), x, y, &x1, &y1, &w, &h);
   display.setCursor(x - w, y);
   display.print(text);
}

/* Draw a circle with optional start and end point */
void SolarDisplay::DrawCircle(int32_t x, int32_t y, int32_t r, uint32_t color, int32_t degFrom /* = 0 */, int32_t degTo /* = 360 */)
{
   for (int i = degFrom; i < degTo; i++) {
      double radians = i * PI / 180;
      double px      = x + r * cos(radians);
      double py      = y + r * sin(radians);
      
      display.drawPixel(px, py, color);
   }
} 

/* Draw one icon from the binary data */
void SolarDisplay::DrawIcon(int x, int y, const uint16_t *icon, int dx /*= 64*/, int dy /*= 64*/, bool highContrast /*= false*/)
{
   for (int yi = 0; yi < dy; yi++) {
      for (int xi = 0; xi < dx; xi++) {
         uint16_t pixel = icon[yi * dx + xi];

         if (highContrast) {
            if (15 - (pixel / 4096) > 0) display.drawPixel(x + xi, y + yi, GRAY_7);
         } else {
            display.drawPixel(x + xi, y + yi, (pixel / 4096) / 2);
         }
      }
   }
}

/* Draw a graph with x- and y-axis and values */
void SolarDisplay::DrawGraph(int x, int y, int dx, int dy, HistoryData &powerHistory, HistoryData &yieldHistory, HistoryData *chargeHistory /*= NULL*/)
{
   String yMinString  = "0"                          + powerHistory.unitName_;
   String yMaxString1 = String(powerHistory.max_, 0) + powerHistory.unitName_;
   String yMaxString2 = String(yieldHistory.max_, 0) + yieldHistory.unitName_;
   int    textWidth   = 5 + max(yMaxString1.length() * 8, yMaxString2.length() * 8);
   int    graphX      = x + 5 + textWidth + 5;
   int    graphY      = y + 25;
   int    graphDX     = dx - textWidth - 20;
   int    graphDY     = dy - 25 - 20;

   display.setTextSize(2);
   DrawString      (yMaxString2, x + dx - 18, graphY - 5);   
   DrawRightString (yMaxString1, x + 53,      graphY - 5);
   DrawRightString (yMinString,  x + 53,      graphY + graphDY - 12);

   display.drawLine(x + dx + 11, graphY + 20, x + dx + 21, graphY + 20, GRAY_0);
   display.drawLine(x + dx + 11, graphY + 20, x + dx + 11, graphY + 30, GRAY_0);
   display.drawLine(x + dx +  1, graphY + 30, x + dx + 11, graphY + 30, GRAY_0);
   display.drawLine(x + dx + 21, graphY + 20, x + dx + 21, graphY + 25, GRAY_0);
   display.drawLine(x + dx + 21, graphY + 25, x + dx + 31, graphY + 25, GRAY_0);

   display.drawRect(graphX, graphY, graphDX, graphDY, GRAY_0);   

   if (powerHistory.size_ > 0) {
      String oldDay;
      float  xStep = (float) graphDX / (float) powerHistory.size_;
      
      display.setTextSize(1);
      for (int i = 0; i < powerHistory.size_; i++) {
         DateTime date = powerHistory.dates_[i];
         String   day  = String(date.day());
         
         if (oldDay != day && date.hour() >= 12) {
            int xPos = graphX - 3 + i * xStep;
            
            oldDay = day;
            if (xPos > graphX) {
               DrawCentreString(day, xPos, graphY + graphDY + 5);
            }
         }
      }
   }

   if (powerHistory.max_ > 0) {
      float yStep = (float) graphDY / (float) powerHistory.max_;
      
      for (int i = 0; i < powerHistory.size_; i++) {
         float yValue   = powerHistory.values_[i];
         float yValueDY = (float) graphDY / (float) powerHistory.max_;
         float xPos     = (float) graphX + graphDX / (float) powerHistory.size_ * i;
         float yPos     = (float) graphY + graphDY - (float) (yValue) * yValueDY;
   
         if (yPos > graphY + graphDY) yPos = graphY + graphDY;
         if (yPos < graphY)           yPos = graphY;
   
         if (i > 0) {
            display.drawLine(xPos, graphY + graphDY, xPos, yPos, GRAY_0);
            // Serial.printf("GraphLine: %d %f %d, %d\n", i, yValue, (int) xPos, (int) yPos);
         }
      }
   }

   if (chargeHistory) {
      float yStep = (float) graphDY / 1000.0;
      int   xLast = 0;
      int   yLast = 0;
      
      for (int i = 0; i < chargeHistory->size_; i++) {
         float yValue   = chargeHistory->values_[i];
         float yValueDY = (float) graphDY / (float) 1000.0;
         float xPos     = (float) graphX + graphDX / (float) chargeHistory->size_ * i;
         float yPos     = (float) graphY + graphDY - (float) (yValue) * yValueDY;
   
         if (yPos > graphY + graphDY) yPos = graphY + graphDY;
         if (yPos < graphY)           yPos = graphY;
   
         if (yValue && xPos && yPos && xLast && yLast) {
            display.drawLine(xLast, yLast - 0, xPos, yPos - 0, GRAY_0);
            display.drawLine(xLast, yLast - 1, xPos, yPos - 1, GRAY_0);
            //Serial.printf("GraphLine: %d %f %d %d %d %d\n", i, yValue, (int) xLast, (int) yLast, (int) xPos, (int) yPos);
         }
         if (yValue && xPos && yPos) {
            xLast = xPos;
            yLast = yPos;
         }
      }
   }
   
   if (yieldHistory.max_ > 0) {
      String oldDay;
      float  xOld      = graphX;
      float  yOld      = 0.0;
      float  xStep     = (float) graphDX / (float) yieldHistory.size_;
      float  yValueDY  = (float) graphDY / (float) yieldHistory.max_;
      float  yMaxValue = 0.0;

      for (int i = 0; i < yieldHistory.size_; i++) {
         DateTime date   = yieldHistory.dates_[i];
         String   day    = String(date.day());
         float    yValue = yieldHistory.values_[i];
         float    xPos   = graphX - 3 + i * xStep;

         if (yValue > yMaxValue) {
            yMaxValue = yValue;
         }

         if (oldDay != day) {
            float yPos = graphY + graphDY - yMaxValue * yValueDY;

            if (xOld > 0 && xPos > graphX) {
               if (yMaxValue > 0) {
                  int    xTextPos        = xOld + (xPos - xOld) / 2;
                  int    yTextPos        = yPos - 12;
                  String yMaxValueString = String(yMaxValue, 2);
                  
                  display.fillRect(xTextPos - 13, yTextPos - 1, 26, 9, GRAY_7);
                  DrawCentreString(yMaxValueString, xTextPos, yTextPos);
               }

               xPos = max((float) graphX + 1, xPos);
               xOld = max((float) graphX + 1, xOld);
               display.drawLine(xOld - 0, yOld - 0, xOld - 0, yPos - 0, GRAY_0);
               display.drawLine(xOld - 0, yPos - 0, xPos - 0, yPos - 0, GRAY_0);
               display.drawLine(xOld - 1, yOld - 1, xOld - 1, yPos - 1, GRAY_0);
               display.drawLine(xOld - 1, yPos - 1, xPos - 1, yPos - 1, GRAY_0);
            }
            oldDay    = day;
            yMaxValue = 0.0;
            xOld      = xPos;
            yOld      = yPos;
         }
      }
   }
}

/* Tranlate the state of operation to readable string. */
String SolarDisplay::StateOfOperation(int value) 
{
   String ret;

        if (value == 0) ret = "Off";
   else if (value == 1) ret = "Low power";
   else if (value == 2) ret = "Fault";
   else if (value == 3) ret = "Bulk";
   else if (value == 4) ret = "Absoption";
   else if (value == 5) ret = "Float";
   else if (value == 9) ret = "Inverting";

   return ret;
}

/* Tranlate the mppt error code to readable string. */
String SolarDisplay::BmvAlarmReason(int value) 
{
   String ret;

   if (value &    1) ret += "[Low Voltage]";
   if (value &    2) ret += "[High Voltage]";
   if (value &    4) ret += "[Low SOC]";
   if (value &    8) ret += "[Low Starter Voltage]";
   if (value &   16) ret += "[High Starter Voltage]";
   if (value &   32) ret += "[Low Temperature]";
   if (value &   64) ret += "[High Temperature]";
   if (value &  128) ret += "[Mid Voltage]";
   if (value &  256) ret += "[Overload]";
   if (value &  512) ret += "[DC-ripple]";
   if (value & 1024) ret += "[Low V AC out]";
   if (value & 2048) ret += "[High V AC out]";

   return ret;
}

/* Tranlate the mppt error code to readable string. */
String SolarDisplay::MpptErrorCode(int value) 
{
   String ret;

        if (value ==   0) ret = "No error";
   else if (value ==   2) ret = "Battery voltage too high";
   else if (value ==  17) ret = "Charger temperature too high";
   else if (value ==  18) ret = "Charger over current";
   else if (value ==  19) ret = "Charger current reversed";
   else if (value ==  20) ret = "Bulk time limit exceeded";
   else if (value ==  21) ret = "Current sensor issue (sensor bias/sensor broken)";
   else if (value ==  26) ret = "Terminals overheated";
   else if (value ==  33) ret = "Input voltage too high (solar panel)";
   else if (value ==  34) ret = "Input current too high (solar panel)";
   else if (value ==  38) ret = "Input shutdown (due to excessive battery voltage)";
   else if (value == 116) ret = "Factory calibration data lost";
   else if (value == 117) ret = "Invalid/incompatible firmware";
   else if (value == 119) ret = "User settings invalid";
   else ret = "ErrorCode: " + String(value, 0);

   return ret;
}

/* Printf operation as String. */
String SolarDisplay::FormatString(String format, double data, int fillLen /*= 9*/)
{
   char buff[100];

   memset(buff, 0, sizeof(buff));
   sprintf(buff, format.c_str(), data);

   String fill;
   int    padLen = (int) fillLen - (int) strlen(buff);

   for (int i = 0; i < padLen; i++) {
      fill += ' ';
   }

   return fill + buff;
}

/* ********************************************************************************************* */

/* Draw the version text.  */
void SolarDisplay::DrawHeadVersion(int x, int y)
{
   DrawString(VERSION, x, y);
}

/* Draw the information when are these data updated. */
void SolarDisplay::DrawHeadUpdated(int x, int y)
{
   String updatedString = "Updated " + getDateTimeString(GetRTCTime());
   
   DrawCentreString(updatedString, x, y);
}

/* Draw the wifi reception strength */
void SolarDisplay::DrawHeadRSSI(int x, int y)
{
   int iQuality = WifiGetRssiAsQualityInt(myData.wifiRSSI);

   DrawRightString(WifiGetRssiAsQuality(myData.wifiRSSI) + "%", x - 2, y - 14);
   if (iQuality >= 80) DrawCircle(x + 12, y, 16, GRAY_0, 225, 315); 
   if (iQuality >= 40) DrawCircle(x + 12, y, 12, GRAY_0, 225, 315); 
   if (iQuality >= 20) DrawCircle(x + 12, y,  8, GRAY_0, 225, 315); 
   if (iQuality >= 10) DrawCircle(x + 12, y,  4, GRAY_0, 225, 315); 
   if (iQuality >=  0) DrawCircle(x + 12, y,  2, GRAY_0, 225, 315); 
}

/* Draw the state of charge. */
void SolarDisplay::DrawHeadBattery(int x, int y)
{
   display.drawRect(x, y, 40, 16, GRAY_0);
   display.drawRect(x + 40, y + 3, 4, 10, GRAY_0);
   for (int i = x; i < x + 40; i++) {
      display.drawLine(i, y, i, y + 15, GRAY_0);
      if ((i - x) * 100.0 / 40.0 > myData.batteryCapacity) {
         break;
      }
   }
   DrawRightString(String(myData.batteryCapacity) + "%", x - 2, y + 1);
}

/* Draw all the information about the battery status. */
void SolarDisplay::DrawBatteryInfo(int x, int y, int dx, int dy)
{
   DateTime toDay                          = GetRTCTime();
   TimeSpan timeSpan                       = toDay - myData.bmv.lastChange;
   String   mainVoltageInfo                = FormatString("%.2f V     ",  myData.bmv.mainVoltage / 1000,                 13);
   String   batteryCurrenInfot             = FormatString("%.2f A     ",  myData.bmv.batteryCurrent / 1000,              13);
   String   midPointDeviationInfo          = FormatString("%.2f %%MP  ",  myData.bmv.midPointDeviation / 10,              12);
   String   stateOfOperationInfo           = FormatString(StateOfOperation(myData.mppt.stateOfOperation), 0,              7);
   String   numberOfChargeCyclesInfo       = FormatString("%.0f Cycles",  myData.bmv.numberOfChargeCycles,               10);
   String   secondsSinceLastFullChargeInfo = FormatString("%.0f Days  ",  myData.bmv.secondsSinceLastFullCharge / 86400, 10);
   String   chargedEnergyInfo              = FormatString("+%.1f kWh",    myData.bmv.chargedEnergy / 100,                 5);
   String   dischargedEnergyInfo           = FormatString("-%.1f kWh",    myData.bmv.dischargedEnergy / 100,              5);

   display.drawRect(x, y, dx, dy, GRAY_0);

   if (myData.bmv.alarmReason > 0.0) {
      display.setTextSize(3);
      DrawString(BmvAlarmReason(myData.bmv.alarmReason), x + 300, y + 70);
   } else if (myData.bmv.lastChange == EmptyDateTime) {
      display.setTextSize(3);
      DrawString("no data", x + 45, y + 70);
   } else if (timeSpan.totalseconds() > 60 * 60) {
      display.setTextSize(3);
      DrawString("no update", x + 45, y + 70);
   } else {
      DrawString(mainVoltageInfo,                x +   2, y +  14);
      DrawString(batteryCurrenInfot,             x +   2, y +  34);
      DrawString(midPointDeviationInfo,          x +   2, y +  54);
      DrawString(numberOfChargeCyclesInfo,       x +   2, y +  84);
      DrawString(secondsSinceLastFullChargeInfo, x +   2, y + 104);
      DrawString(stateOfOperationInfo,           x +   2, y + 134);
   
      DrawString(chargedEnergyInfo,              x + 130, y +  14);
      DrawString(dischargedEnergyInfo,           x + 130, y +  34);
   }
   display.setTextSize(2);

   DrawIcon(x + dx - 23, y + dy - 34, (uint16_t *) image_data_BatteryIconSmall, 13, 24);
}

/* Draw a solar symbol. */
void SolarDisplay::DrawSolarSymbol(int x, int y, int dx, int dy)
{
   DrawIcon(x, y, (uint16_t *) image_data_SolarIcon, 150, 150);

   display.setTextSize(3);
   DrawRightString(String(myData.mppt.panelPower, 0)      + "W ", x + dx, y + 13);
   DrawRightString(String(myData.mppt.yieldToday * 10, 0) + "Wh", x + dx, y + 45);
   display.setTextSize(2);
}

/* Draw the Solar arror if the mppt current flows. */
void SolarDisplay::DrawSolarArrow(int x, int y, int dx, int dy)
{
   DrawIcon(x, y - 10, (uint16_t *) image_data_LineUpDown, 4, 47);
   DrawIcon(x, y + 10, (uint16_t *) image_data_LineUpDown, 4, 47);
   if (myData.mppt.batteryCurrent > 0.0) {
      DrawIcon(x + 14, y + 5, (uint16_t *) image_data_ArrowDown, 20, 39);
   }
}

/* Draw all the Grid information. */
void SolarDisplay::DrawGridInfo(int x, int y, int dx, int dy)
{
   DateTime toDay       = GetRTCTime();
   TimeSpan timeSpan    = toDay - myData.tasmotaElite.lastChange;
   String   voltageInfo = FormatString("%.2f V", myData.tasmotaElite.voltage, 8);
   String   ampereInfo  = FormatString("%.2f A", myData.tasmotaElite.ampere,  8);

   display.drawRect(x, y, dx, dy, GRAY_0);

   if (myData.tasmotaElite.alive == "false") {
      display.setTextSize(3);
      DrawString("switched off", x + 100, y + 70);
   } else if (timeSpan.totalseconds() > 60 * 60) {
      display.setTextSize(3);
      DrawString("no update", x + 100, y + 70);
   } else {
      DrawString(voltageInfo, x + 14, y + 14);
      DrawString(ampereInfo,  x + 14, y + 34);

      DrawGraph(x + 120, y - 5, myData.tasmotaElite.powerHistory.size_, dy, myData.tasmotaElite.powerHistory, myData.tasmotaElite.yieldHistory);
   }
   display.setTextSize(2);
   DrawIcon(x + dx - 40, y + dy - 40, (uint16_t *) image_data_HouseIconSmall, 30, 30);
}

/* Draw a battery symbol. */
void SolarDisplay::DrawBatterySymbol(int x, int y, int dx, int dy)
{
   DrawIcon(x, y, (uint16_t *) image_data_BatteryIcon, 60, 100);

   int zero          = y + dy - 7;
   int full          = y + 14;
   int stateOfCharge = (myData.bmv.stateOfCharge / 10.0);
   int state         = zero - (zero - full) / 100.0 * stateOfCharge;

   for (int i = zero; i > full; i--) {
      display.drawLine(x + 6, i, x + dx - 7, i, GRAY_0);
      if (i < state) {
         break;
      }
   }

   if (stateOfCharge < 40) {   
      DrawCentreString(String(myData.bmv.stateOfCharge / 10.0, 0) + "%", x + 30, y + (dy / 2) - 8);
   } else if (stateOfCharge > 65) {   
      display.setTextColor(7, 0);
      DrawCentreString(String(myData.bmv.stateOfCharge / 10.0, 0) + "%", x + 30, y + (dy / 2) - 8);
   } else {
      display.setTextColor(0, 7);
      DrawCentreString(String(myData.bmv.stateOfCharge / 10.0, 0) + "%", x + 30, y + dy - 30);
   }
   display.setTextColor(0, 7);
}

/* Draw the battery inverter connection. */
void SolarDisplay::DrawBatteryArrow(int x, int y, int dx, int dy)
{
   display.setTextSize(3);
   DrawCentreString(String(myData.bmv.instantaneousPower, 0) + "W", x + (dx / 2) - 5, y + 13);
   display.setTextSize(2);
   
   DrawIcon(x, y + 48, (uint16_t *) image_data_LineLeftRight, 110, 4);
   
   if (myData.bmv.batteryCurrent > 0.0) {
      DrawIcon(x + 16, y + 68, (uint16_t *) image_data_ArrowLeft, 70, 15);
   } else if (myData.bmv.batteryCurrent < 0.0) {
      DrawIcon(x + 16, y + 68, (uint16_t *) image_data_ArrowRight, 70, 15);
   }
}

/* Draw a inverter symbol. */
void SolarDisplay::DrawInverterSymbol(int x, int y, int dx, int dy)
{
   DrawIcon(x, y, (uint16_t *) image_data_InverterIcon, 100, 100);
}

/* Draw the inverter house connection. */
void SolarDisplay::DrawInverterArrow(int x, int y, int dx, int dy)
{
   DrawIcon(x, y + 48, (uint16_t *) image_data_LineLeftRight, 110, 4);

   display.setTextSize(3);

   if (myData.bmv.relay == "ON" && myData.tasmotaElite.voltage > 0.0) {
      display.setTextSize(3);
      DrawCentreString(String(myData.tasmotaElite.power, 0) + "W", x + (dx / 2) - 5, y + 13);
      display.setTextSize(2);
      DrawIcon(x + 16, y + 68, (uint16_t *) image_data_ArrowRight, 70, 15);
    } else {
      DrawString("OFF", x + 35, y + 13);
   }
   display.setTextSize(2);
}

/* Draw a house symbol. */
void SolarDisplay::DrawHouseSymbol(int x, int y, int dx, int dy)
{
   DrawIcon(x, y, (uint16_t *) image_data_HouseIcon, 100, 100);
}

/* Draw the house grid connection. */
void SolarDisplay::DrawGridArrow(int x, int y, int dx, int dy)
{
   DrawIcon(x, y + 48, (uint16_t *) image_data_LineLeftRight, 110, 4);

   display.setTextSize(3);
   if (myData.bmv.relay == "OFF" || myData.tasmotaElite.alive == "false") {
      DrawString("ON", x + 35, y + 13);
      DrawIcon(x + 16, y + 68, (uint16_t *) image_data_ArrowLeft, 70, 15);
   } else {
      DrawString("OFF", x + 35, y + 13);
   }
   display.setTextSize(2);
}

/* Draw a grid symbol. */
void SolarDisplay::DrawGridSymbol(int x, int y, int dx, int dy)
{
   DrawIcon(x, y, (uint16_t *) image_data_GridIcon, 60, 100);
}

/* Draw all solar panel data. */
void SolarDisplay::DrawSolarInfo(int x, int y, int dx, int dy)
{
   DateTime toDay              = GetRTCTime();
   TimeSpan timeSpan           = toDay - myData.mppt.lastChange;
   double   panelAmpere        = myData.mppt.panelPower / (myData.mppt.panelVoltage / 1000);
   String   panelVoltageInfo   = FormatString("%.2f V ", myData.mppt.panelVoltage / 1000,   10);
   String   panelAmpereInfo    = FormatString("%.2f A ", panelAmpere,                       10);
   String   mainVoltageInfo    = FormatString("%.2f V ", myData.mppt.mainVoltage / 1000.0,  10);
   String   batteryCurrentInfo = FormatString("%.2f A ", myData.mppt.batteryCurrent / 1000, 10);

   display.drawRect(x, y, dx, dy, GRAY_0);

   DrawString("Panel",            x + 14, y +  14);
   DrawString(panelVoltageInfo,   x + 14, y +  39);
   DrawString(panelAmpereInfo,    x + 14, y +  59);
   DrawString("Battery",          x + 14, y +  89);
   DrawString(mainVoltageInfo,    x + 14, y + 114);
   DrawString(batteryCurrentInfo, x + 14, y + 134);

   if (myData.mppt.errorCode > 0.0) {
      display.setTextSize(3);
      DrawString(MpptErrorCode(myData.mppt.errorCode), x + 300, y + 70);
   } else if (myData.mppt.lastChange == EmptyDateTime) {
      display.setTextSize(3);
      DrawString("no data", x + 300, y + 70);
   } else if (timeSpan.totalseconds() > 60 * 60) {
      display.setTextSize(3);
      DrawString("no update", x + 300, y + 70);
   } else {
      DrawGraph(x + 140, y - 5, myData.mppt.ppvHistory.size_, dy, myData.mppt.ppvHistory, myData.mppt.yieldHistory, &myData.bmv.chargeHistory);
   }
   display.setTextSize(2);
   DrawIcon(x + dx - 40, y + dy - 40, (uint16_t *) image_data_SolarIconSmall, 30, 30);
}

/* Draw the complete head. */
void SolarDisplay::DrawHead(int x, int y, int dx, int dy)
{
   DrawHeadVersion  (x + 5,         y + 13);
   DrawHeadUpdated( (x + (dx / 2)), y + 13);
   DrawHeadRSSI     (x + dx - 130,  y + 26);
   DrawHeadBattery  (x + dx -  49,  y + 11);
}

/* Draw the whole solar information body. */
void SolarDisplay::DrawBody(int x, int y, int dx, int dy)
{
   display.drawRect(x, y, dx, dy, GRAY_0);

   DrawBatteryInfo    (x +  10 * X_FACTOR, y +  10 * Y_FACTOR, 250 * X_FACTOR, 186 * Y_FACTOR);
   DrawSolarSymbol    (x + 276 * X_FACTOR, y +  50 * Y_FACTOR, 150, 150);
   DrawSolarArrow     (x + 344 * X_FACTOR, y + 160 * Y_FACTOR,  50,  50);
   DrawGridInfo       (x + 438 * X_FACTOR, y +  10 * Y_FACTOR, 486 * X_FACTOR, 186 * Y_FACTOR);
   DrawBatterySymbol  (x +  96 * X_FACTOR, y + 223 * Y_FACTOR,  60, 100);
   DrawBatteryArrow   (x + 171 * X_FACTOR, y + 223 * Y_FACTOR, 110, 100);
   DrawInverterSymbol (x + 296 * X_FACTOR, y + 223 * Y_FACTOR, 100, 100);
   DrawInverterArrow  (x + 411 * X_FACTOR, y + 223 * Y_FACTOR, 110, 100);
   DrawHouseSymbol    (x + 536 * X_FACTOR, y + 223 * Y_FACTOR, 100, 100);
   DrawGridArrow      (x + 651 * X_FACTOR, y + 223 * Y_FACTOR, 110, 100);
   DrawGridSymbol     (x + 776 * X_FACTOR, y + 223 * Y_FACTOR,  60, 100);
   DrawSolarInfo      (x +  12 * X_FACTOR, y + 313 * Y_FACTOR, 912 * X_FACTOR, 186 * Y_FACTOR);
}

/* Fill the screen. */
void SolarDisplay::Show()
{
   Serial.println("SolarDisplay::DrawSolarInfo");

   display.setTextSize(2);
   display.setTextColor(0, 7);

   DrawHead(14,  0, maxX - 28, 33);
   DrawBody(14, 34, maxX - 28, maxY - 45);

   display.display();
   delay(1000);
}

/* Show WiFi connewction error. */
void SolarDisplay::ShowWiFiError(String ssid)
{
   Serial.println("SolarDisplay::ShowWiFiError");

   display.setTextSize(4);
   display.setTextColor(0, 7);

   String errMsg = "WiFi error: [" + ssid + "]";
   DrawCentreString(errMsg, maxX / 2, maxY / 2);

   display.display();
   delay(1000);
}
