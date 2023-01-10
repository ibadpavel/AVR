#include "Wire.h"

#include "customMPU6050.h"
#include "MadgwickOptim.h"

//== GLOBAL STATIC VARIABLES =============
basicMPU6050 imu;
MadgwickOptim imuFilter;

float buf[3] = { 0, 0, 0 };  //roll pitch yaws

// Define Slave I2C Address
#define SLAVE_ADDR 0x55
//========================================

void setup() {
  //ATinny88 LED pin - D0
  pinMode(0, OUTPUT);

  // Set registers - Always required
  imu.setup();

  // Initial calibration of gyro
  imu.setBias();

  Wire.begin(SLAVE_ADDR);
  Wire.onRequest(requestEvent);
}

void requestEvent() {
  Wire.write((char*)&buf[0], sizeof(float) * 3);
}

void loop() {
  imuFilter.updateIMU(imu.ax(), imu.ay(), imu.az(), imu.gx(), imu.gy(), imu.gz());

  buf[0] = imuFilter.getRoll();
  buf[1] = imuFilter.getPitch();
  buf[2] = imuFilter.getYaw();
}
