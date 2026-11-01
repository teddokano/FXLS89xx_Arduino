/*
 * Copyright 2023 ryraki
 * All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */
/**
 * @file example_EXT_TRIG.ino
 * @brief This file can measure G value by using EXT_TRIG function
 */
#include <Wire.h>
#include <FXLS89xx_Arduino.h>

/*
 * Definition for test functions
 */

#if defined(TARGET_C444) || defined(TARGET_N236)
#error "EXT_TRIG needs the INT2 pin, which the on-board sensor of FRDM-MCXC444/MCXN236 does not bring out. Use a FXLS89xx shield on the Arduino header."
#endif

FXLS89xx fxls89xx;

/*
 * Functions
 */
void setup() {
  // put your setup code here, to run once:
  // Initialize the device and take the WHO_AM_I parameter
  // put your setup code here, to run once:
  // Initialize the device and take the WHO_AM_I parameter
  Serial.begin(115200);
  while (!Serial);
  pinMode(A0, INPUT);
  Wire.begin();
  
  uint8_t whoami = fxls89xx.init();
  Serial.print("WHO_AM_I: ");
  Serial.println(whoami, HEX);
  fxls89xx.wake_odr = FXLS89xx::_6_25HZ;
  fxls89xx.wake_pm  = FXLS89xx::_HPM;
  fxls89xx.sensor_range = FXLS89xx::_2G;
  fxls89xx.EXT_TRIG_init();
}

void loop() {
  // put your main code here, to run repeatedly:
  // Repeated every 1 sec
  fxls89xx.EXT_TRIG_Trigger();  // Triggers EXT_TRIG
  while (!digitalRead(2));      // Wait until D2 (INT1) rises                                    
  float float_output[3];
  fxls89xx.read_XYZ(float_output); // Read and calculate XYZ G Data
  Serial.print("x:");
  Serial.print(float_output[0]);
  Serial.print(", y:");
  Serial.print(float_output[1]);
  Serial.print(", z:");
  Serial.println(float_output[2]);
  delay(500);
}
