#include "sensors.h"

//#include <SoftwareSerial.h>
//SoftwareSerial mySoftSerial(2, 3); // RX, TX


void initializeSensors()
{
    //mySoftSerial.begin(9600);   // Bluetooth 
}

void getSensors(imu_t *imuData)
{
    //mySoftSerial.println("GET;GYRO;millis_");

    imuData->ax = 12;
    imuData->ay = 13;
    imuData->az = 14;

    imuData->gx = 15;
    imuData->gy = 16;
    imuData->gz = 17;
}
