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

#define IOBROKER_QUERY     "/query/"
#define IOBROKER_GET       "/get/"
#define IOBROKER_GET_PLAIN "/getPlainValue/"

#define REQUEST_TIMEOUT    2000 // msec

class IoBrokerWifiClient; //!< Wifi connection class
class IoBrokerBase;       //!< Base class for IoBroker communication
class IoBrokerPlain;      //!< Plain value (double/string) request
class IoBrokerValue;      //!< get values request
class IoBrokerHistory;    //!< History request


/* ***************************************************************************** */
/* *** class IoBrokerWifiClient ************************************************ */
/* ***************************************************************************** */

/**
  * Helper class for the IoBroker connection.
  */
class IoBrokerWifiClient
{
public:
   WiFiClient client_; //!< wifi client

public:
   bool connect();
   void disconnect();
   bool connected(bool reconnect = true);
   void waitForAvailable();
   
public:
   IoBrokerWifiClient();
   ~IoBrokerWifiClient();
};

/* Constructor: Connect to the IoBroker server. */
IoBrokerWifiClient::IoBrokerWifiClient()
{
   connect();
}

/* Destructor: Disconnect. */
IoBrokerWifiClient::~IoBrokerWifiClient()
{
   if (client_.connected()) {
      client_.stop();
   }
}

/* Connect to the specific IoBroker server. */
bool IoBrokerWifiClient::connect()
{
   Serial.print("try to connect to IoBroker!");
   if (!client_.connect(IOBROKER_URL, IOBROKER_PORT)) {
      Serial.println(" -> connection failed!");
   } else {
      Serial.println(" -> connected!");
      delay(100);
   }
}

/* Disconnect the IoBroker server. */
void IoBrokerWifiClient::disconnect()
{
   if (client_.connected()) {
      client_.stop();
   }
}

/* Are we connected, reconnect if needed.. */
bool IoBrokerWifiClient::connected(bool reconnect /*= true*/)
{
   if (reconnect) {
      if (!client_.connected()) {
         connect();
      }
   }
   return client_.connected();
}

/* Wait until the server is available. */
void IoBrokerWifiClient::waitForAvailable()
{
   unsigned long _startMillis = millis();
   unsigned long _timeout     = client_.getTimeout();

   while (!client_.available()) {
      if (millis() - _startMillis > _timeout) {
         Serial.println("timeout!");          
         client_.stop(); 
         break;
      }
   }
}

/* ***************************************************************************** */
/* *** class IoBrokerBase ****************************************************** */
/* ***************************************************************************** */

/**
  * Base class for IoBroker communication.
  */
class IoBrokerBase
{
protected:
   IoBrokerWifiClient &wifiClient_; //!< Reference to the IoBroker wifiClient

protected:
   void parseContentLen(int &len, String line);

   virtual void onRequest ()       = 0;
   virtual void onChar    (char c) = 0;
   
public:
   IoBrokerBase(IoBrokerWifiClient &ioBrokerWifiClient)
      : wifiClient_(ioBrokerWifiClient)
   {
   }

   bool sendRequest(String method, String topic, String param = "");
};

/* Read the Content-Length header. */
void IoBrokerBase::parseContentLen(int &len, String line)
{
   String lenString = "Content-Length:";
   int    index     = line.indexOf(lenString);
   
   if (index != -1) {
      len = atoi(line.substring(index + lenString.length()).c_str());
   }
}

/* Read a String from IoBroker. */
bool IoBrokerBase::sendRequest(String method, String topic, String param) 
{
   bool ret = false;

   Serial.print("sendRequest! ");
   if (wifiClient_.connected()) {
      int    ticks         = 0;
      int    readLength    = 0;
      int    contentLength = -1;
      String url           = method + topic + param;
            
      // This will send the request to the server
      onRequest();
      Serial.print(url);
      wifiClient_.client_.print("GET " + url + " HTTP/1.1\r\nConnection: keep-alive\r\n\r\n");      
      wifiClient_.client_.flush();

      wifiClient_.waitForAvailable();
      // Read http response head
      while (wifiClient_.client_.available()) {
         String line = wifiClient_.client_.readStringUntil('\n');
         
         parseContentLen(contentLength, line);
         if (line == "\r") break;
      }    

      // Read http response body
      ticks = millis();
      do {
         if (wifiClient_.client_.available()) {
            onChar((char) wifiClient_.client_.read());
            readLength++;
            ret = true;
         } else {
            if (millis() - ticks > REQUEST_TIMEOUT) {
               Serial.println("IoBrokerBase::sendRequest() -> timeout!");
               break;
            }
         }
      } while (contentLength == -1 || readLength < contentLength);
      wifiClient_.client_.flush();
      Serial.println(" -> ok");
   }
   return ret;
}

/* ***************************************************************************** */
/* *** class IoBrokerPlain ***************************************************** */
/* ***************************************************************************** */

/**
  * IoBroker plain value (double) request.
  */
class IoBrokerPlain : public IoBrokerBase
{
protected:
   String plainString_; //!< Holds the incoming wifi data

protected:
   virtual void onRequest ();
   virtual void onChar    (char c);
   
public:
   IoBrokerPlain(IoBrokerWifiClient &wifiClient)
      : IoBrokerBase(wifiClient)
   {
   }

   bool getPlainValue(String &value, String topic);
   bool getPlainValue(double &value, String topic);
};

/* The request was started. */
void IoBrokerPlain::onRequest()
{
   plainString_ = "";
}

/* one char comes in. */
void IoBrokerPlain::onChar(char c)
{
   plainString_ += c;
}

/* The incoming data are complete. */
bool IoBrokerPlain::getPlainValue(String &value, String topic)
{
   if (sendRequest(IOBROKER_GET_PLAIN, topic)) {
      value = plainString_;
      value = Trim(value, "\"");
      Serial.println("   plainValue: " + value);
      return true;
   }
   return false;
}

/* Convert the string request to double. */
bool IoBrokerPlain::getPlainValue(double &value, String topic)
{
   value = 0.0;
   if (sendRequest(IOBROKER_GET_PLAIN, topic)) {
      value = plainString_.toDouble();
      Serial.println("   plainValue: " + String(value));
      return true;
   }
   return false;
}

/* ***************************************************************************** */
/* *** class IoBrokerValue ***************************************************** */
/* ***************************************************************************** */

/**
  * IoBroker get values request.
  */
class IoBrokerValue : public IoBrokerBase
{
protected:
   String jsonData_; //!< Holds the incoming wifi data

protected:
   virtual void onRequest ();
   virtual void onChar    (char c);
   
public:
   IoBrokerValue(IoBrokerWifiClient &wifiClient)
      : IoBrokerBase(wifiClient)
   {
   }

   bool getLastChange(DateTime &dateTime, String topic);
};

/* The request was started. */
void IoBrokerValue::onRequest()
{
   jsonData_ = "";
}

/* one char comes in. */
void IoBrokerValue::onChar(char c)
{
   jsonData_ += c;
}

/* 
 *  Get the last change date informstion from the simple json result.. 
   { ... "lc": 1442431190, ... }
   */
bool IoBrokerValue::getLastChange(DateTime &dateTime, String topic)
{
   dateTime = EmptyDateTime;
   if (sendRequest(IOBROKER_GET, topic)) {
      String lcPart  = "\"ts\":";
      int    lcIndex = jsonData_.indexOf(lcPart);

      if (lcIndex >= 0) {
         int lcIndexEnd = jsonData_.indexOf(",", lcIndex + lcPart.length());

         if (lcIndexEnd >= 0) {
            String timestamp = jsonData_.substring(lcIndex + lcPart.length(), lcIndexEnd - 3); // no milliseconds

            dateTime = UtcToLocalTime(timestamp.toInt());
            Serial.printf("   get lc: %d-%d-%d %d:%d:%d\n", dateTime.year(), dateTime.month(), dateTime.day(), dateTime.hour(), dateTime.minute(), dateTime.second());
            return true;
         }
      }
   }
   return false;
}

/* ***************************************************************************** */
/* *** class IoBrokerHistory ************************************************ */
/* ***************************************************************************** */

/**
  * IoBroker History request.
  */
class IoBrokerHistory : public IoBrokerBase
{
protected:
   HistoryData &historyData_;  //!< The history data of the last 4 weeks
   int          days_;         //!< How many days we will show
   DateTime     fromDate_;     //!< Start date for the request (4 weeks earlier)
   DateTime     toDate_;       //!< End date, tomorrow

   String       valueString_;   //!< Incommig data  
   bool         valueStart_;    //!< One incomming data '[xxx, xxx]' has startet

protected:
   virtual void onRequest ();
   virtual void onChar    (char c);

   void parsValue(String valueString);

public:
   enum HISTORY_TYPE { AVG, MAX } eHistoryType_;
   
public:
   IoBrokerHistory(IoBrokerWifiClient &wifiClient, HistoryData &historyData, int days, HISTORY_TYPE eHistoryType = AVG)
      : IoBrokerBase(wifiClient)
      , historyData_(historyData)
      , days_(days)
      , valueStart_(false)
      , eHistoryType_(eHistoryType)
   {
   }
   ~IoBrokerHistory()
   {
   }

   bool getHistoryValues(String topic, float factor = 1.0);
};

/* The request has started. */
void IoBrokerHistory::onRequest()
{
   valueString_ = "";
}

/* Add every char to the valueString and start parsing at the end of one data item. */
void IoBrokerHistory::onChar(char c)
{
   if (c == '[') {
      valueStart_  = true;
      valueString_ = "";
   } else if (c == ']') {
      valueStart_ = false;
      // Serial.println("valueString: " + valueString_);
      parsValue(valueString_);
      valueString_ = "";
   } else {
      valueString_ += c;
   }
}

/* Pars one history data string. 'value, timestamp' */
void IoBrokerHistory::parsValue(String valueString)
{
   if  (valueString.length() >= 0) {
      if (isDigit(valueString[0])) {
         int sep = valueString.indexOf(',');

         if (sep >= 0) {
            String value     = valueString.substring(0, sep);
            String timestamp = valueString.substring(sep + 1, valueString.length() - 3); // no milliseconds

            if (value != "null" && timestamp != "null") {
               DateTime jsonDate(timestamp.toInt());
   
               int historyIndex = (double) historyData_.size_ / (double) (toDate_.secondstime() - fromDate_.secondstime()) * (double) (jsonDate.secondstime() - fromDate_.secondstime());
   
               if (historyIndex >= 0 && historyIndex < historyData_.size_) {
                  if (historyData_.max_ < value.toFloat()) {
                     historyData_.max_ = value.toFloat();
                  }
                  if (eHistoryType_ == AVG) {
                     historyData_.values_[historyIndex] += value.toFloat();
                     historyData_.counts_[historyIndex]++;
                  } else { // MAX
                     if (value.toFloat() > historyData_.values_[historyIndex]) {
                        historyData_.values_[historyIndex] = value.toFloat();
                     }
                     historyData_.counts_[historyIndex] = 1;
                  }
               } else {
                  Serial.printf("\nWrong history index! [%d] [%f] Timestamp: %d-%d-%d %d:%d:%d\n", historyIndex, value.toFloat(), jsonDate.year(), jsonDate.month(), jsonDate.day(), jsonDate.hour(), jsonDate.minute(), jsonDate.second());
               }
               // Serial.printf("**** Index: %d Value: %f Timestamp: %d-%d-%d %d:%d:%d\n", historyIndex, value.toFloat(), jsonDate.year(), jsonDate.month(), jsonDate.day(), jsonDate.hour(), jsonDate.minute(), jsonDate.second());
            }
         }
      }
   }
}

/* Read all the history data of one mqtt type. */
bool IoBrokerHistory::getHistoryValues(String topic, float factor /*= 1.0*/)
{
   String   param;
   DateTime toDay = GetRTCTime();

   historyData_.clear();

   // Calculate the from and to dates (4 weeks ago and tomorrow) and format these as a query param.
   toDate_   = toDay   + TimeSpan(    0, 3, 0, 0);
   fromDate_ = toDate_ - TimeSpan(days_, 0, 0, 0);
   param     = "?dateFrom=" + getIoBrokerDateTimeString(fromDate_) +
               "&dateTo="   + getIoBrokerDateTimeString(toDate_)   +
               "&count="    + String(200000);

   // Initialize the right time to the array positions.
   int fromUnixTime = fromDate_.unixtime();                    
   int toUnixTime   = toDate_.unixtime();                    
   for (int i = 0; i < historyData_.size_; i++) {
      historyData_.dates_[i] = DateTime((int) ((float) fromUnixTime + i * (float) (toUnixTime - fromUnixTime) / (float) historyData_.size_));
   }

   // Send request, pars on every date item internaly
   if (sendRequest(IOBROKER_QUERY, topic, param)) {
      // average on every data
      for (int i = 0; i < historyData_.size_; i++) {
         if (historyData_.counts_[i] > 0) {
            historyData_.values_[i] = factor * historyData_.values_[i] / historyData_.counts_[i];
         }
      }
      return true;
   }
   
   return false;
}

/* ***************************************************************************** */
/* *** GetIoBrokerValues() ***************************************************** */
/* ***************************************************************************** */

/* Helper Funktion to read all the IoBroker data into the data object. */
void GetIoBrokerValues(MyData &myData)
{
   IoBrokerWifiClient  ioBrokerWifiClient;
   IoBrokerPlain       ioBrokerPlain            (ioBrokerWifiClient);
   IoBrokerValue       ioBrokerValue            (ioBrokerWifiClient);
   IoBrokerHistory     ioBrokerChargeHistory    (ioBrokerWifiClient, myData.bmv.chargeHistory,         21);                       // 21 days
   IoBrokerHistory     ioBrokerPPVHistory       (ioBrokerWifiClient, myData.mppt.ppvHistory,           21);                       // 21 days
   IoBrokerHistory     ioBrokerPPVYieldHistory  (ioBrokerWifiClient, myData.mppt.yieldHistory,         21, IoBrokerHistory::MAX); // 21 days
   IoBrokerHistory     ioBrokerGridHistory      (ioBrokerWifiClient, myData.tasmotaElite.powerHistory,  7, IoBrokerHistory::MAX); //  7 days
   IoBrokerHistory     ioBrokerGridYieldHistory (ioBrokerWifiClient, myData.tasmotaElite.yieldHistory,  7, IoBrokerHistory::MAX); //  7 days

   ioBrokerPlain.getPlainValue(myData.bmv.consumedAmpHours,           "mqtt.0.bmv.CE");
   ioBrokerPlain.getPlainValue(myData.bmv.stateOfCharge,              "mqtt.0.bmv.SOC");
   ioBrokerPlain.getPlainValue(myData.bmv.midPointDeviation,          "mqtt.0.bmv.DM");
   ioBrokerPlain.getPlainValue(myData.bmv.numberOfChargeCycles,       "mqtt.0.bmv.H4");
   ioBrokerPlain.getPlainValue(myData.bmv.dischargedEnergy,           "mqtt.0.bmv.H17");
   ioBrokerPlain.getPlainValue(myData.bmv.chargedEnergy,              "mqtt.0.bmv.H18");
   ioBrokerPlain.getPlainValue(myData.bmv.cumulativeAmpHoursDrawn,    "mqtt.0.bmv.H6");
   ioBrokerPlain.getPlainValue(myData.bmv.secondsSinceLastFullCharge, "mqtt.0.bmv.H9");
   ioBrokerPlain.getPlainValue(myData.bmv.batteryCurrent,             "mqtt.0.bmv.I");
   ioBrokerPlain.getPlainValue(myData.bmv.instantaneousPower,         "mqtt.0.bmv.P");
   ioBrokerPlain.getPlainValue(myData.bmv.relay,                      "mqtt.0.bmv.Relay");
   ioBrokerPlain.getPlainValue(myData.bmv.timeToGo,                   "mqtt.0.bmv.TTG");
   ioBrokerPlain.getPlainValue(myData.bmv.mainVoltage,                "mqtt.0.bmv.V");
   ioBrokerValue.getLastChange(myData.bmv.lastChange,                 "mqtt.0.bmv.V");

   ioBrokerChargeHistory.getHistoryValues("mqtt.0.bmv.SOC");

   ioBrokerPlain.getPlainValue(myData.mppt.stateOfOperation,          "mqtt.0.mppt.CS");
   ioBrokerPlain.getPlainValue(myData.mppt.yieldTotal,                "mqtt.0.mppt.H19");
   ioBrokerPlain.getPlainValue(myData.mppt.yieldToday,                "mqtt.0.mppt.H20");
   ioBrokerPlain.getPlainValue(myData.mppt.maximumPowerToday,         "mqtt.0.mppt.H21");
   ioBrokerPlain.getPlainValue(myData.mppt.yieldYesterday,            "mqtt.0.mppt.H22");
   ioBrokerPlain.getPlainValue(myData.mppt.maximumPowerYesterday,     "mqtt.0.mppt.H23");
   ioBrokerPlain.getPlainValue(myData.mppt.batteryCurrent,            "mqtt.0.mppt.I");
   ioBrokerPlain.getPlainValue(myData.mppt.panelPower,                "mqtt.0.mppt.PPV");
   ioBrokerPlain.getPlainValue(myData.mppt.mainVoltage,               "mqtt.0.mppt.V");
   ioBrokerPlain.getPlainValue(myData.mppt.panelVoltage,              "mqtt.0.mppt.VPV");
   ioBrokerValue.getLastChange(myData.mppt.lastChange,                "mqtt.0.mppt.V");

   ioBrokerPPVHistory.getHistoryValues      ("mqtt.0.mppt.PPV");
   ioBrokerPPVYieldHistory.getHistoryValues ("mqtt.0.mppt.H22", 0.01);
   myData.mppt.yieldHistory.max_ = 3.0; // Max yield to 3 kWh

   ioBrokerPlain.getPlainValue(myData.tasmotaElite.voltage,           "sonoff.0.TasmotaElite.ENERGY_Voltage");
   ioBrokerPlain.getPlainValue(myData.tasmotaElite.ampere,            "sonoff.0.TasmotaElite.ENERGY_Current");
   ioBrokerPlain.getPlainValue(myData.tasmotaElite.power,             "sonoff.0.TasmotaElite.ENERGY_Power");
   ioBrokerPlain.getPlainValue(myData.tasmotaElite.alive,             "sonoff.0.TasmotaElite.alive");
   ioBrokerValue.getLastChange(myData.tasmotaElite.lastChange,        "sonoff.0.TasmotaElite.ENERGY_Voltage");

   ioBrokerGridHistory.getHistoryValues      ("sonoff.0.TasmotaElite.ENERGY_Power");
   ioBrokerGridYieldHistory.getHistoryValues ("sonoff.0.TasmotaElite.ENERGY_Yesterday");
   myData.tasmotaElite.yieldHistory.max_ = 2.0; // Max consumption to 2 kWh
}
