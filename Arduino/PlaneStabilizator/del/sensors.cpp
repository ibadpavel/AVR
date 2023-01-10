#include <Arduino.h>
#include "sensors.h"

void initializeSensors()
{
     randomSeed(analogRead(0));
}

void getSensors(imu_t *imuData)
{
    imuData->ax = 0.1 * random(10, 20);
    imuData->ay = 0.1 * random(11, 21);
    imuData->az = 0.1 * random(12, 22);

    imuData->gx = 0.1 * random(13, 23);
    imuData->gy = 0.1 * random(14, 24);
    imuData->gz = 0.1 * random(15, 25);
}
