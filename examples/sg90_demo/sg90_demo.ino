/*
 * Copyright Tedd OKANO
 * All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */
/**
 * @file sg90_demo.ino
 * @brief Two SG90 servo motors follow the tilt of the on-board FXLS8974CF.
 *        For FRDM-MCXN236 and FRDM-MCXC444 (mcx-arduino-core)
 *
 * Connect the servo signal lines to A5 (X-Y tilt) and A4 (X-Z tilt).
 * These are not PWM pins: the pulses come from a timer interrupt (Servo library),
 * which can tremble a little. Use PWM pins (D3/D9 etc.) for steadier pulses.
 * Power the servos from 5V, not from the board's 3.3V pin.
 */
#include <Wire.h>
#include <Servo.h>
#include "FXLS89xx_Arduino.h"

#if !defined(TARGET_C444) && !defined(TARGET_N236)
#error "This example uses the on-board sensor of FRDM-MCXC444 / FRDM-MCXN236"
#endif

constexpr int   SERVO_PIN_0   = A5;
constexpr int   SERVO_PIN_1   = A4;

constexpr int   PULSE_MIN     = 500;  // [us]
constexpr int   PULSE_MAX     = 2400; // [us]
constexpr int   DEAD_BAND     = 10;    // [us] ignore smaller changes so the servo doesn't hunt
constexpr int   FILTER_LENGTH = 10;
constexpr float RAD_TO_DEGREE = 180.0 / 3.1415926535897;

class Filter {
public:
  Filter(int length) : data(new float[length]), len(length), index(0) {
    for (int i = 0; i < len; i++)
      data[i] = 0.0;
  }

  float calc(float v) {
    data[index++] = v;
    index %= len;

    float sum = 0;
    for (int i = 0; i < len; i++)
      sum += data[i];

    return sum / (float)len;
  }

private:
  float *data;
  int    len;
  int    index;
};

FXLS89xx fxls89xx(Wire1);   // the accelerometer is on Wire1
Servo    srv[2];
Filter   fx(FILTER_LENGTH), fy(FILTER_LENGTH), fz(FILTER_LENGTH);

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println("*** ServoMotor demo ***");

  Wire1.begin();
  fxls89xx.int1_pin = -1;   // INT1 is not available: init() polls instead

  srv[0].attach(SERVO_PIN_0, PULSE_MIN, PULSE_MAX);
  srv[1].attach(SERVO_PIN_1, PULSE_MIN, PULSE_MAX);

  uint8_t whoami = fxls89xx.init();
  Serial.print("WHO_AM_I: ");
  Serial.println(whoami, HEX);
  fxls89xx.wake_odr     = FXLS89xx::_100HZ;
  fxls89xx.wake_pm      = FXLS89xx::_HPM;
  fxls89xx.sensor_range = FXLS89xx::_2G;
  fxls89xx.run();
}

// angle -90..+90 degrees -> pulse width, in 1us steps (Servo.write() has 1 degree steps)
void move(int ch, float angle) {
  static int last[2] = {0, 0};
  float a  = constrain(angle, -90.0, 90.0);
  int   us = PULSE_MIN + (int)((a + 90.0) / 180.0 * (PULSE_MAX - PULSE_MIN));

  if (abs(us - last[ch]) >= DEAD_BAND) {
    srv[ch].writeMicroseconds(us);
    last[ch] = us;
  }
}

void loop() {
  while (!(fxls89xx.read_r8(FXLS89xx::_INT_STATUS) & FXLS89xx::SRC_DRDY_H));   // Wait until DRDY is set

  float sensor_data[3];
  fxls89xx.read_XYZ(sensor_data);

  // Filter the acceleration, then calculate the angles. (Averaging the angles
  // themselves jumps where atan2() wraps around at +-180 degrees)
  float x = fx.calc(sensor_data[0]);
  float y = fy.calc(sensor_data[1]);
  float z = fz.calc(sensor_data[2]);

  move(0, atan2(x, y) * RAD_TO_DEGREE - 90.0);
  move(1, atan2(x, z) * RAD_TO_DEGREE - 90.0);
}
