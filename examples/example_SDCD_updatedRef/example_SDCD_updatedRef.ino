/*
 * Copyright 2023 ryraki
 * All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */
/**
 * @file example_SDCD_updatedRef.ino
 * @brief SDCD function is utilized with sleep mode. Please check with Serial Plotter. When Z crosses upper or lower threshold, the device wakes from SLEEP mode. When 5 measurements are done without crossing thresholds, the device goes to SLEEP.
 */
#include <Wire.h>
#include "FXLS89xx_Arduino.h"

/*
 * Definition for test functions
 */

/*
 * FRDM-MCXC444 / FRDM-MCXN236 (mcx-arduino-core):
 * Use the on-board FXLS8974CF on Wire1. Its INT pins are not on the headers,
 * so the data-ready flag is polled from the INT_STATUS register.
 * Other boards: use a FXLS89xx shield on the Arduino header (INT1: D2, INT2: A0).
 */
#if defined(TARGET_C444) || defined(TARGET_N236)
#define ONBOARD_SENSOR
FXLS89xx fxls89xx(Wire1);
#else
FXLS89xx fxls89xx;
#endif
float upper_threshold=100;
float lower_threshold=-100;
float previousZ;

/*
 * Functions
 */
void setup() {
  // put your setup code here, to run once:
  // Initialize the device and take the WHO_AM_I parameter
  Serial.begin(115200);
  while (!Serial);
#ifndef ONBOARD_SENSOR
  pinMode(A0, INPUT);
#endif
#ifdef ONBOARD_SENSOR
  Wire1.begin();
  fxls89xx.int1_pin = -1;   // INT1 is not available: init() polls instead
#else
  Wire.begin();
#endif
  
  uint8_t whoami = fxls89xx.init();
  Serial.print("WHO_AM_I: ");
  Serial.println(whoami, HEX);
  fxls89xx.wake_odr = FXLS89xx::_12_5HZ;
  fxls89xx.wake_pm  = FXLS89xx::_HPM;
  fxls89xx.sleep_odr = FXLS89xx::_1_563HZ;
  fxls89xx.sleep_pm  = FXLS89xx::_LPM;
  fxls89xx.sensor_range = FXLS89xx::_2G;
  fxls89xx.sdcd_wt = false;
  fxls89xx.sdcd(FXLS89xx::_DIFFPREV, FXLS89xx::___Z, upper_threshold, lower_threshold);
  fxls89xx.enable_sleep(5);
  fxls89xx.run();
}

void loop() {
#ifdef ONBOARD_SENSOR
  uint8_t int_status;
  do {
    int_status = fxls89xx.read_r8(FXLS89xx::_INT_STATUS);
  } while (!(int_status & FXLS89xx::SRC_DRDY_H));   // Wait until DRDY is set
#else
  while (!digitalRead(2));      // Wait until D2 (INT1) rises
#endif
  float float_output[3];
  fxls89xx.read_XYZ(float_output); // Read and calculate XYZ G Data
  Serial.print(",Z:");
  Serial.print(float_output[2]);
  Serial.print(",Upper_Threshold:");
  Serial.print(previousZ+upper_threshold);
  Serial.print(",Lower_Threshold:");
  Serial.print(previousZ+lower_threshold);
  Serial.print(",SDCD_INT(Z):");
#ifdef ONBOARD_SENSOR
  Serial.println((int_status & FXLS89xx::SRC_SDCD_OT_H) ? 1000 : 0);
#else
  Serial.println(digitalRead(A0)?1000:0);
#endif
  previousZ=float_output[2];
}
