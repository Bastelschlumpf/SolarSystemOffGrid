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
  * @file veReader.h
  *
  * Class for reading the ve-direct values
  */


#define MAX_KEYWORDS 60

/**
  * class to read the ve.direct serial port and store the values in Strings
  */
class VEDirectReader
{
public:
   HardwareSerial &serial_;                  //!< Hardware serial communication
   int             index_;                   //!< Index of the keyword value pair Strings
   bool            blockCompleted_;          //!< Is one block finished?
   String          keywords_[MAX_KEYWORDS];  //!< Keywords
   String          values_[MAX_KEYWORDS];    //!< Values
   
public:
   VEDirectReader(HardwareSerial &serial);

   void reset();
   int  getValueCount();
   bool isBlockCompleted();
   bool isCheckSumOk();
   
   bool readLine();
}; 

/* ******************************************** */

VEDirectReader::VEDirectReader(HardwareSerial &serial)
   : serial_(serial)
   , index_(0)
   , blockCompleted_(false)
{
}

/** Reset all the indexes and values. */
void VEDirectReader::reset()
{
   index_          = 0;
   blockCompleted_ = false;
}

/** Get the count of the keyword value pair.
  * Starts from 0, the last is the Checksum value so ignore this.
  */
int VEDirectReader::getValueCount()
{
   return index_;
}

/** Is the block completed, 'Checksum' sent. */
bool VEDirectReader::isBlockCompleted()
{
   return blockCompleted_;
}

/** Is checksum ok? */
bool VEDirectReader::isCheckSumOk()
{
   byte checksum = 0;

   for (int i = 0; i <= index_; i++) {
      String keyword = keywords_[i];
      String value   = values_[i];

      Serial.write(keyword.c_str());
      Serial.write("-");
      Serial.write(value.c_str());
      Serial.write("\n");
      
      for (int y = 0; y < keyword.length(); y++) {
         checksum += keyword[y];
      }
      for (int y = 0; y < value.length(); y++) {
         checksum += value[y];
      }
   }
   checksum += (index_ + 1) * ('\t' + '\r' + '\n');

   // Serial.printf("CheckSum: %d\n", checksum);
   return checksum == 0;
}

/** Read one line from the ve.direct bus.
  * On '\t' - change from keyword to value
  * On '\n' - check if the block is finished
  * Else add the char to the checksum and
  * add the char to keyword or value
  */
bool VEDirectReader::readLine()
{
   int  type       = 1;
   bool isStarting = blockCompleted_;

   if (blockCompleted_) {
      reset();
   }

   keywords_[index_] = "";
   values_[index_]   = "";
   
   while (serial_.available() > 0) {
      char rc = serial_.read();

      /* DEBUG
      Serial.printf("%c", isPrintable(rc) ? rc : '.');
      if (rc == '\n') {
         Serial.println(" ");
      } */

      // On start, we wait for the first carriage return
      if (isStarting && rc != '\r') {
         continue;
      }
      isStarting = false;

      if (rc == '\t') {
         type++;
      } else if (rc == '\r') {
         // ignore \r
      } else if (rc == '\n') {
         if (index_ < MAX_KEYWORDS - 1) {
            // The block starts with cr/nl so we skip
            // this empty line here.
            if (!keywords_[index_].isEmpty()) {
               index_++;
            }
         }
         break;
      } else { // key or value
         if (type == 1) {
            keywords_[index_] += rc;
         } else { // type = 2
            values_[index_] += rc;
            if (keywords_[index_] == "Checksum") {
              blockCompleted_ = true;
              break;
            }         
         }
      }
      yield();
   }

   return true;
}
