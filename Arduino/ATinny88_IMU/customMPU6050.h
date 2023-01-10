#include "Arduino.h"
#include <Wire.h>

#ifndef customMPU6050_h
#define customMPU6050_h

//------------------ Common terms --------------------

// Sensor values
#define             MPU_ADDRESS_LOW     0x68
#define             MPU_ADDRESS_HIGH    0x69

#ifdef ADDRESS_A0_HIGH
#define MPU_ADDRESS MPU_ADDRESS_HIGH
#else
#define MPU_ADDRESS MPU_ADDRESS_LOW
#endif

#define             ACCEL_LBS_0         16384.0
#define             N_AXIS              3

//---------------- Default settings ------------------
#define DLPF_CFG     6                 /* Low pass filter setting - 0 to 6         */
#define FS_SEL       0                 /* Gyro sensitivity - 0 to 3                */
#define AFS_SEL      0                 /* Accelerometer sensitivity - 0 to 3       */
#define GYRO_BAND    16                /* Standard deviation of gyro signals       */
#define N_BIAS       10000             /* Samples of average used to calibrate gyro*/

//---------------- Offsets ---------------------------
#define AX_OFS       0                 /* Accel X offset                           */
#define AY_OFS       0                 /* Accel Y offset                           */
#define AZ_OFS       0                 /* Accel Z offset                           */

//---------------- Multiplier ------------------------
#define AX_S         1                 /* Accel X Multiplier                       */
#define AY_S         1                 /* Accel Y Multiplier                       */
#define AZ_S         1                 /* Accel Z Multiplier                       */
#define GX_S         1                 /* Gyro X Multiplier                        */
#define GY_S         1                 /* Gyro Y Multiplier                        */
#define GZ_S         1                 /* Gyro Z Multiplier                        */

const float BAND_SQ = GYRO_BAND * GYRO_BAND;
const float TEMP_MUL = 1.0 / 340.0;
const float MEAN = 1.0/float(N_BIAS);                                 // Inverse of sample count

const float ACCEL_LBS =
              AFS_SEL < 0 || AFS_SEL >  3 ? 1         :     // Scaling factor for accelerometer. Depends on sensitivity setting.
                       1.0/( AFS_SEL == 0 ? 16384.0   :     // Output is in: g
                             AFS_SEL == 1 ? 8192.0    :
                             AFS_SEL == 2 ? 4096.0    :
                                            2048.0    );
const float GYRO_LBS =
             FS_SEL < 0 || FS_SEL >  3 ? 1      :           // Scaling factor for gyro. Depends on sensitivity setting.
              (PI/180.0)/( FS_SEL == 0 ? 131.0  :           // Output is in: rad/s
                           FS_SEL == 1 ? 65.5   :
                           FS_SEL == 2 ? 32.8   :
                                         16.4   );

//---------------- Class definition ------------------
class basicMPU6050 {
   private:
    float mean[N_AXIS] = {0};
    float var = 0;

    // I2C communication
    void setRegister(uint8_t, uint8_t);
    void readRegister( uint8_t );
    int readWire();

   public:
    void setup();

    //-- Raw measurements

    // Accel
    int rawAx();
    int rawAy();
    int rawAz();

    // Temp
    int rawTemp();

    // Gyro
    int rawGx();
    int rawGy();
    int rawGz();

    //-- Scaled measurements

    // Accel
    float ax();
    float ay();
    float az();

    // Temp
    float temp();

    // Gyro
    float gx();
    float gy();
    float gz();

    //-- Gyro bias estimate
    void setBias();
    void updateBias();
};

#include "customMPU6050.tpp"

#endif
