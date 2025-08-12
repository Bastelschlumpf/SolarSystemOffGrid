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
  * @file EPD.h
  * 
  * Helper functions for initializing and shutdown of the M5Paper.
  */
#pragma once

#include "driver/gpio.h"
#include "esp_wifi.h"
#include "esp_bt.h"

#define GRAYSCALE_0           0  // Black
#define DISPLAY_POWER_PIN    23  // Power Supply for IT8951 and GT911
#define M5EPD_MAIN_PWR_PIN    2  // Main power supply (MOSFET)
#define BMI270_ADDRESS     0x69  // BMI

/* Initialize the M5Paper */
void InitEPD(bool clearDisplay = true)
{
   Serial.println("InitEPD: Initializing M5PaperS3");

   Serial.begin(115200);
   auto cfg = M5.config();

   cfg.fallback_board = m5::board_t::board_M5PaperS3;
   cfg.clear_display          = false;
   cfg.internal_imu           = false;
   cfg.external_imu           = false;
   cfg.internal_rtc           = true;
   cfg.external_rtc           = false;
   cfg.internal_spk           = false;
   cfg.internal_mic           = false;
   cfg.output_power           = false;
   cfg.serial_baudrate        = 0;
   cfg.led_brightness         = 0;
   cfg.external_display_value = 0x00;
   cfg.external_speaker_value = 0x00;
   M5.begin(cfg);

   M5.Lcd.setRotation(1);
   if (clearDisplay) {
      M5.Lcd.fillScreen(GRAYSCALE_0);
   }
   // disableCore0WDT();
}

/* 
 *  Shutdown the M5Paper with lowest possible power consumption
 *  NOTE: the M5Paper could not shutdown while on usb connection.
 *        In this case use the esp_deep_sleep_start() function.
*/
void ShutdownEPD(int sec) 
{
   Serial.println("Shutdown (" + String((int)(sec / 60)) + " min)");
   Serial.println("Battery Voltage: " + String(M5.Power.getBatteryVoltage()) + " mV");

   // Power down E-Ink display completely
   pinMode((gpio_num_t)DISPLAY_POWER_PIN, OUTPUT);
   digitalWrite(DISPLAY_POWER_PIN, LOW);
   gpio_hold_en((gpio_num_t)DISPLAY_POWER_PIN);
   
   M5.Display.powerSaveOn();
   M5.Display.sleep();
   M5.Display.waitDisplay();

   // Completely disable all radio functions
   WiFi.disconnect(true, true);
   WiFi.mode(WIFI_OFF);
   esp_wifi_stop();
   btStop();
   esp_bt_controller_disable();
   esp_bt_mem_release(ESP_BT_MODE_BTDM);

   // Disable main power rail
   pinMode((gpio_num_t)M5EPD_MAIN_PWR_PIN, OUTPUT);
   digitalWrite(M5EPD_MAIN_PWR_PIN, LOW);
   gpio_hold_en((gpio_num_t)M5EPD_MAIN_PWR_PIN);
   
   // Switch off Serial
   Serial.end();

   // Configure power domains for deepest sleep
   gpio_deep_sleep_hold_en();
   esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
   esp_sleep_pd_config(ESP_PD_DOMAIN_XTAL, ESP_PD_OPTION_OFF);
   esp_sleep_pd_config(ESP_PD_DOMAIN_VDDSDIO, ESP_PD_OPTION_OFF);

   M5.Rtc.setAlarmIRQ(sec);
   M5.Power.powerOff();
}