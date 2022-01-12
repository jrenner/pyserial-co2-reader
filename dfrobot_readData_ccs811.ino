/*!
 * @file drrobot_readData_ccs811.ino
 * @brief Read the concentration of carbon dioxide and TVOC 
 * @copyright	Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com), Copyright (c) 2021 Alex Jokela
 * @license     The MIT License (MIT)
 * @version  V0.2
 * @date  2021-12-24
 */

#include "DFRobot_CCS811.h"
#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

#include "SparkFun_External_EEPROM.h" // Click here to get the library: http://librarymanager/All#SparkFun_External_EEPROM

#define BUFF_SIZE 14

ExternalEEPROM mem;
U8G2_SSD1327_EA_W128128_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);


/*
 * IIC address default 0x5A, the address becomes 0x5B if the ADDR_SEL is soldered.
 */
//DFRobot_CCS811 CCS811(&Wire, /*IIC_ADDRESS=*/0x5A);
DFRobot_CCS811 CCS811;

#define EEPROM_SAVE_COUNT 10

char line0[BUFF_SIZE];
char line1[BUFF_SIZE];
char line2[BUFF_SIZE];
char line3[BUFF_SIZE];
char line4[BUFF_SIZE];

uint32_t counter = 0;
float    avg_co2 = 0.0;
float    avg_voc = 0.0;
float    NaN;
float    intraVals[EEPROM_SAVE_COUNT];
uint16_t intraCnt= 0;

void setup(void)
{
  Serial.begin(115200);
  /*Wait for the chip to be initialized completely, and then exit*/
  while(CCS811.begin() != 0){
      Serial.println("ERR: failed to init chip, please check if the chip connection is fine");
      delay(1000);
  }

  u8g2.begin();
  sprintf(line0, "..");
  sprintf(line1, "Warming...");
  sprintf(line2, "....");
  sprintf(line3, "......");
  sprintf(line4, "........");

  mem.get(0, counter);
  mem.get(4, avg_co2);
  mem.get(8, avg_voc);

  /*
   * Reset EEPROM values
   * -----------------------------------
   */
  /*
  counter = 0;
  avg_co2 = NaN;
  avg_voc = NaN;
  */
  /*
   * -----------------------------------
   */

  while(mem.begin() == false) {
    Serial.println("ERR: No memory detected");
    delay(1000);
  }

  if(counter == ULONG_MAX) {
    Serial.println("ERR: counter reset");
    counter = 0;
  }

  if(avg_co2 != avg_co2) { // NaN check
    Serial.println("ERR: avg_co2 reset");
    avg_co2 = 0.0;
  }

  if(avg_voc != avg_voc) { // NaN check
    Serial.println("ERR: avg_voc reset");
    avg_voc = 0.0;
  }

  mem.put(0, counter);
  mem.put(4, avg_co2);
  mem.put(8, avg_voc);
  
}
void loop() {

  u8g2.setFont(u8g2_font_profont17_mr);
  u8g2.firstPage();

  do {

    u8g2.setCursor(0, 13);
    u8g2.print(line0);
    
    u8g2.setCursor(0, 32);
    u8g2.print(line1);
    
    u8g2.setCursor(0, 51);
    u8g2.print(line2);

    u8g2.setCursor(0, 70);
    u8g2.print(line3);

    u8g2.setCursor(0, 89);
    u8g2.print(line4);
    
    delay(250);
    
  } while ( u8g2.nextPage() );
  
  if(CCS811.checkDataReady() == true){

    char *dots;
    
    uint16_t co2 = CCS811.getCO2PPM();
    uint16_t voc = CCS811.getTVOCPPB();

    Serial.print("CO2: ");
    Serial.print(co2);
    Serial.print(",TVOC: ");
    Serial.println(voc);

    sprintf(line0, "");
    sprintf(line1, "co2: %d ppm", co2);
    sprintf(line3, "voc: %d ppb", voc);

    if( counter % 10 == 0) {
      dots = "..........";
    } else if ( counter % 8 == 0 ) {
      dots = "........";
    } else if ( counter % 6 == 0 ) {
      dots = "......";
    } else if ( counter % 4 == 0 ) {
      dots = "...";
    } else if ( counter % 3 == 0 ) {
      dots = ".";
    }

    sprintf(line4, "%s", dots);

    intraVals[intraCnt] = co2;

    if( counter % EEPROM_SAVE_COUNT == 0 && counter >= EEPROM_SAVE_COUNT ) {
      mem.put(0, counter);
      ////////////////
      avg_co2 = approx_average(co2, average(intraVals, EEPROM_SAVE_COUNT), counter);

      /*
      Serial.println("----------------------------");
      Serial.print("avg co2: ");
      Serial.println(avg_co2);
      Serial.println("----------------------------");
      */
      
      mem.put(4, avg_co2);

      sprintf(line2, "avg: %d.%01d ppm", (int)avg_co2, (int)(avg_co2*10)%10);

      intraCnt = 0;
    }
 
    counter++;
    intraCnt++;
    
  } else {
    Serial.println("ERR: Data is not ready!");
  }
  /*!
   * @brief Set baseline
   * @param get from getBaseline.ino
   */
  CCS811.writeBaseLine(0x447B);
  //delay cannot be less than measurement cycle
  delay(1000);
}

float approx_average(float new_value, float old_average, uint32_t n) {
  if(old_average <= 0.0) {
    old_average = new_value;
  }
  float new_average = (old_average * (((float)n)-1.0) + new_value) / ((float)n);
  return new_average;
}

float average(float values[], uint16_t n) {
  float tmp = 0.0;
  for(uint16_t i=0; i<n; ++i) {
    tmp += values[i];
  }

  return tmp/(float(n));
}
