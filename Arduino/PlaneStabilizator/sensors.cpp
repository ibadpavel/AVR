#include "sensors.h"

void initializeSensors()
{
     
}

void getSensors(imu_t *imuData)
{
    imuData->ax = 12;
    imuData->ay = 13;
    imuData->az = 14;

    imuData->gx = 15;
    imuData->gy = 16;
    imuData->gz = 17;
}
