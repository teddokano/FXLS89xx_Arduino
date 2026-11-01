/*
 * Copyright 2023 ryraki
 * All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */
/**
 * @file example_DRDY_read.ino
 * @brief This file can measure XYZ accelerometer values in WAKE mode
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

/*
 * Functions
 */
void setup() {
  // put your setup code here, to run once:
  // Initialize the device and take the WHO_AM_I parameter
  Serial.begin(115200);
  while (!Serial);
#ifdef ONBOARD_SENSOR
  Wire1.begin();
  fxls89xx.int1_pin = -1;   // INT1 is not available: init() polls instead
#else
  Wire.begin();
#endif

  uint8_t whoami = fxls89xx.init();
  Serial.print("WHO_AM_I: ");
  Serial.println(whoami, HEX);
  fxls89xx.wake_odr = FXLS89xx::_6_25HZ;
  fxls89xx.wake_pm  = FXLS89xx::_HPM;
  fxls89xx.sensor_range = FXLS89xx::_2G;
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
  Serial.print("x:");
  Serial.print(float_output[0]);
  Serial.print(", y:");
  Serial.print(float_output[1]);
  Serial.print(", z:");
  Serial.println(float_output[2]);
}
