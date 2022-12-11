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


M5EPD_Canvas canvas(&M5.EPD); // Main canvas of the e-paper

/* Main class for drawing the content to the e-paper display. */
class SolarDisplay
{
protected:
   MyData &myData; //!< Reference to the global data
   int     maxX;   //!< Max width of the e-paper
   int     maxY;   //!< Max height of the e-paper

protected:
   void   DrawCircle            (int32_t x, int32_t y, int32_t r, uint32_t color, int32_t degFrom = 0, int32_t degTo = 360);
   void   DrawIcon              (int x, int y, const uint16_t *icon, int dx = 64, int dy = 64, bool highContrast = false);
   void   DrawGraph             (int x, int y, int dx, int dy, int xMin, int xMax, int yMin, int yMax, float values[]);
   String FormatString          (String format, double data, int fillLen = 8);
   String StateOfOperation      (double value);
   
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
   SolarDisplay(MyData &md, int x = 960, int y = 540)
      : myData(md)
      , maxX(x)
      , maxY(y)
   {
   }

   void Show();
};

/* Draw a circle with optional start and end point */
void SolarDisplay::DrawCircle(int32_t x, int32_t y, int32_t r, uint32_t color, int32_t degFrom /* = 0 */, int32_t degTo /* = 360 */)
{
   for (int i = degFrom; i < degTo; i++) {
      double radians = i * PI / 180;
      double px      = x + r * cos(radians);
      double py      = y + r * sin(radians);
      
      canvas.drawPixel(px, py, color);
   }
} 

/* Draw one icon from the binary data */
void SolarDisplay::DrawIcon(int x, int y, const uint16_t *icon, int dx /*= 64*/, int dy /*= 64*/, bool highContrast /*= false*/)
{
   for (int yi = 0; yi < dy; yi++) {
      for (int xi = 0; xi < dx; xi++) {
         uint16_t pixel = icon[yi * dx + xi];

         if (highContrast) {
            if (15 - (pixel / 4096) > 0) canvas.drawPixel(x + xi, y + yi, M5EPD_Canvas::G15);
         } else {
            canvas.drawPixel(x + xi, y + yi, 15 - (pixel / 4096));
         }
      }
   }
}

/* Draw a graph with x- and y-axis and values */
void SolarDisplay::DrawGraph(int x, int y, int dx, int dy, int xMin, int xMax, int yMin, int yMax, float values[])
{
   String yMinString = String(yMin);
   String yMaxString = String(yMax);
   int    textWidth  = 5 + max(yMinString.length() * 3.5, yMaxString.length() * 3.5);
   int    graphX     = x + 5 + textWidth + 5;
   int    graphY     = y + 35;
   int    graphDX    = dx - textWidth - 20;
   int    graphDY    = dy - 35 - 20;
   float  xStep      = graphDX / (xMax - xMin);
   float  yStep      = graphDY / (yMax - yMin);
   int    iOldX      = 0;
   int    iOldY      = 0;

   canvas.setTextSize(1);
   canvas.drawString(yMaxString, x + 5, graphY - 5);   
   canvas.drawString(yMinString, x + 5, graphY + graphDY - 3);   
   for (int i = 0; i <= (xMax - xMin); i++) {
      canvas.drawString(String(i), graphX + i * xStep, graphY + graphDY + 5);   
   }
   
   canvas.drawRect(graphX, graphY, graphDX, graphDY, M5EPD_Canvas::G15);   
   for (int i = xMin; i <= xMax; i++) {
      float yValue   = values[i - xMin];
      float yValueDY = (float) graphDY / (yMax - yMin);
      int   xPos     = graphX + graphDX / (xMax - xMin) * i;
      int   yPos     = graphY + graphDY - (yValue - yMin) * yValueDY;

      if (yPos > graphY + graphDY) yPos = graphY + graphDY;
      if (yPos < graphY)           yPos = graphY;

      canvas.fillCircle(xPos, yPos, 2, M5EPD_Canvas::G15);
      if (i > xMin) {
         canvas.drawLine(iOldX, iOldY, xPos, yPos, M5EPD_Canvas::G15);         
         Serial.printf("drawLine: %d, %f : %d, %d, %d, %d\n", i, yValue, iOldX, iOldY, xPos, yPos);
      }
      iOldX = xPos;
      iOldY = yPos;
   }
}

/* Tranlate the state of operation to readable string. */
String SolarDisplay::StateOfOperation(double value) 
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
   canvas.drawString(VERSION, x, y);
}

/* Draw the information when are these data updated. */
void SolarDisplay::DrawHeadUpdated(int x, int y)
{
   String updatedString = "Updated " + getRTCDateString() + " " + getRTCTimeString();
   
   canvas.drawCentreString(updatedString, x, y, 1);
}

/* Draw the wifi reception strength */
void SolarDisplay::DrawHeadRSSI(int x, int y)
{
   int iQuality = WifiGetRssiAsQualityInt(myData.wifiRSSI);

   canvas.drawString(WifiGetRssiAsQuality(myData.wifiRSSI) + "%", x - 43, y - 15);
   if (iQuality >= 80) DrawCircle(x + 12, y, 16, M5EPD_Canvas::G15, 225, 315); 
   if (iQuality >= 40) DrawCircle(x + 12, y, 12, M5EPD_Canvas::G15, 225, 315); 
   if (iQuality >= 20) DrawCircle(x + 12, y,  8, M5EPD_Canvas::G15, 225, 315); 
   if (iQuality >= 10) DrawCircle(x + 12, y,  4, M5EPD_Canvas::G15, 225, 315); 
   if (iQuality >=  0) DrawCircle(x + 12, y,  2, M5EPD_Canvas::G15, 225, 315); 
}

/* Draw the state of charge. */
void SolarDisplay::DrawHeadBattery(int x, int y)
{
   canvas.drawRect(x, y, 40, 16, M5EPD_Canvas::G15);
   canvas.drawRect(x + 40, y + 3, 4, 10, M5EPD_Canvas::G15);
   for (int i = x; i < x + 40; i++) {
      canvas.drawLine(i, y, i, y + 15, M5EPD_Canvas::G15);
      if ((i - x) * 100.0 / 40.0 > myData.batteryCapacity) {
         break;
      }
   }
   canvas.drawString(String(myData.batteryCapacity) + "%", x - 45, y + 1);
}

/* Draw all the information about the battery status. */
void SolarDisplay::DrawBatteryInfo(int x, int y, int dx, int dy)
{
   String mainVoltageInfo                = FormatString("%.2f V     ",  myData.bmv.mainVoltage / 1000,                 13);
   String batteryCurrenInfot             = FormatString("%.2f A     ",  myData.bmv.batteryCurrent / 1000,              13);
   String midPointDeviationInfo          = FormatString("%.2f %%MP  ", myData.bmv.midPointDeviation / 10,              12);
   String stateOfOperationInfo           = FormatString(StateOfOperation(myData.mppt.stateOfOperation), 10);
   String numberOfChargeCyclesInfo       = FormatString("%.0f Cycles",  myData.bmv.numberOfChargeCycles,               10);
   String secondsSinceLastFullChargeInfo = FormatString("%.0f Days  ",  myData.bmv.secondsSinceLastFullCharge / 86400, 10);

   canvas.drawRect(x, y, dx, dy, M5EPD_Canvas::G15);

   canvas.drawString(mainVoltageInfo,                x + 14, y +  14);
   canvas.drawString(batteryCurrenInfot,             x + 14, y +  34);
   canvas.drawString(midPointDeviationInfo,          x + 14, y +  54);
   canvas.drawString(numberOfChargeCyclesInfo,       x + 14, y +  84);
   canvas.drawString(secondsSinceLastFullChargeInfo, x + 14, y + 104);
   canvas.drawString(stateOfOperationInfo,           x + 14, y + 134);

   DrawIcon(x + dx - 23, y + dy - 34, (uint16_t *) image_data_BatteryIconSmall, 13, 24);
}

/* Draw a solar symbol. */
void SolarDisplay::DrawSolarSymbol(int x, int y, int dx, int dy)
{
   DrawIcon(x, y, (uint16_t *) image_data_SolarIcon, 100, 100);

   canvas.setTextSize(3);
   canvas.drawCentreString(String(myData.mppt.panelPower, 0)      + "W",  x + 75, y - 10, 1);
   canvas.drawCentreString(String(myData.mppt.yieldToday * 10, 0) + "Wh", x + 85, y + 22, 1);
   canvas.setTextSize(2);
}

/* Draw the Solar arror if the mppt current flows. */
void SolarDisplay::DrawSolarArrow(int x, int y, int dx, int dy)
{
   DrawIcon(x, y, (uint16_t *) image_data_LineUpDown, 4, 47);
   if (myData.mppt.batteryCurrent > 0.0) {
      DrawIcon(x + 14, y + 5, (uint16_t *) image_data_ArrowDown, 20, 39);
   }
}

/* Draw all the Grid information. */
void SolarDisplay::DrawGridInfo(int x, int y, int dx, int dy)
{
   String powerInfo   = FormatString("%.0f W", myData.tasmotaElite.power,   5);
   String voltageInfo = FormatString("%.2f V", myData.tasmotaElite.voltage, 8);
   String ampereInfo  = FormatString("%.2f A", myData.tasmotaElite.ampere,  8);

   canvas.drawRect(x, y, dx, dy, M5EPD_Canvas::G15);

   canvas.drawString(powerInfo,   x + 14, y + 14);
   canvas.drawString(voltageInfo, x + 14, y + 44);
   canvas.drawString(ampereInfo,  x + 14, y + 64);
   
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
      canvas.drawLine(x + 6, i, x + dx - 7, i, M5EPD_Canvas::G15);
      if (i < state) {
         break;
      }
   }

   if (stateOfCharge < 40) {   
      canvas.drawString(String(myData.bmv.stateOfCharge / 10.0, 0) + "%", x + 6, y + (dy / 2) - 8);
   } else if (stateOfCharge > 65) {   
      canvas.setTextColor(BLACK, WHITE);
      canvas.drawString(String(myData.bmv.stateOfCharge / 10.0, 0) + "%", x + 6, y + (dy / 2) - 8);
   } else {
      canvas.setTextColor(BLACK, WHITE);
      canvas.drawString(String(myData.bmv.stateOfCharge / 10.0, 0) + "%", x + 6, y + dy - 30);
   }
   canvas.setTextColor(WHITE, BLACK);
}

/* Draw the battery inverter connection. */
void SolarDisplay::DrawBatteryArrow(int x, int y, int dx, int dy)
{
   canvas.setTextSize(3);
   canvas.drawCentreString(String(myData.bmv.instantaneousPower, 0) + "W", x + (dx / 2) - 5, y + 13, 1);
   canvas.setTextSize(2);
   
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
   canvas.setTextSize(3);
   if (myData.bmv.relay == "ON") {
      DrawIcon(x + 16, y + 68, (uint16_t *) image_data_ArrowRight, 70, 15);
      canvas.drawString("ON", x + 35, y + 13);
   } else {
      canvas.drawString("OFF", x + 35, y + 13);
   }
   canvas.setTextSize(2);
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
   canvas.setTextSize(3);
   if (myData.bmv.relay == "OFF" || myData.tasmotaElite.voltage == 0) {
      DrawIcon(x + 16, y + 68, (uint16_t *) image_data_ArrowLeft, 70, 15);
      canvas.drawString("ON", x + 35, y + 13);
   } else {
      canvas.drawString("OFF", x + 35, y + 13);
   }
   canvas.setTextSize(2);
}

/* Draw a grid symbol. */
void SolarDisplay::DrawGridSymbol(int x, int y, int dx, int dy)
{
   DrawIcon(x, y, (uint16_t *) image_data_GridIcon, 60, 100);
}

/* Draw all solar panel data. */
void SolarDisplay::DrawSolarInfo(int x, int y, int dx, int dy)
{
   double panelAmpere        = myData.mppt.panelPower / (myData.mppt.panelVoltage / 1000);
   String panelVoltageInfo   = FormatString("%.2f V ", myData.mppt.panelVoltage / 1000,   10);
   String panelAmpereInfo    = FormatString("%.2f A ", panelAmpere,                       10);
   String mainVoltageInfo    = FormatString("%.2f V ", myData.mppt.mainVoltage / 1000.0,  10);
   String batteryCurrentInfo = FormatString("%.2f A ", myData.mppt.batteryCurrent / 1000, 10);

   canvas.drawRect(x, y, dx, dy, M5EPD_Canvas::G15);

   canvas.drawString("Panel",            x + 14, y +  14);
   canvas.drawString(panelVoltageInfo,   x + 14, y +  39);
   canvas.drawString(panelAmpereInfo,    x + 14, y +  59);
   canvas.drawString("Battery",          x + 14, y +  89);
   canvas.drawString(mainVoltageInfo,    x + 14, y + 114);
   canvas.drawString(batteryCurrentInfo, x + 14, y + 134);

   DrawGraph(x + 150, y - 5, PPV_HISTORY_SIZE, dy, 0, PPV_HISTORY_SIZE, 0, myData.ppvMax, myData.ppvHistory);

   Serial.printf("HistoryMax: %f\n", myData.ppvMax);
   for (int i = 0; i < PPV_HISTORY_SIZE; i++) {
      Serial.printf("History: %d - %f\n", i, myData.ppvHistory[i]);
   }
   
   DrawIcon(x + dx - 40, y + dy - 40, (uint16_t *) image_data_SolarIconSmall, 30, 30);
}

/* Draw the complete head. */
void SolarDisplay::DrawHead(int x, int y, int dx, int dy)
{
   DrawHeadVersion  (x + 5,         y + 13);
   DrawHeadUpdated( (x + (dx / 2)), y + 13);
   DrawHeadRSSI     (x + dx - 133,  y + 26);
   DrawHeadBattery  (x + dx -  49,  y + 11);
}

/* Draw the whole solar information body. */
void SolarDisplay::DrawBody(int x, int y, int dx, int dy)
{
   canvas.drawRect(x, y, dx, dy, M5EPD_Canvas::G15);

   DrawBatteryInfo    (x +  10, y +  10, 250, 166);
   DrawSolarSymbol    (x + 300, y +  35, 100, 100);
   DrawSolarArrow     (x + 346, y + 140,  50,  50);
   DrawGridInfo       (x + 436, y +  10, 486, 166);
   DrawBatterySymbol  (x +  96, y + 196,  60, 100);
   DrawBatteryArrow   (x + 171, y + 196, 110, 100);
   DrawInverterSymbol (x + 296, y + 196, 100, 100);
   DrawInverterArrow  (x + 411, y + 196, 110, 100);
   DrawHouseSymbol    (x + 536, y + 196, 100, 100);
   DrawGridArrow      (x + 651, y + 196, 110, 100);
   DrawGridSymbol     (x + 776, y + 196,  60, 100);
   DrawSolarInfo      (x +  10, y + 316, 912, 168);
}

/* Fill the screen. */
void SolarDisplay::Show()
{
   Serial.println("SolarDisplay::DrawSolarInfo");

   canvas.setTextSize(2);
   canvas.setTextColor(WHITE, BLACK);
   canvas.setTextDatum(TL_DATUM);
   canvas.createCanvas(960, 540);

   DrawHead(14,  0, maxX - 28, 33);
   DrawBody(14, 34, maxX - 28, maxY - 45);

   canvas.pushCanvas(0, 0, UPDATE_MODE_GC16);
   delay(2000);
}
