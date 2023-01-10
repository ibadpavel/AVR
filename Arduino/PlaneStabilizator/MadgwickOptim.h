#ifndef MadgwickOPTIM_h
#define MadgwickOPTIM_h
#include <math.h>

//--------------------------------------------------------------------------------------------
// Variable declaration
class MadgwickOptim{
public:
    float b_x = 1, b_z = 0; // reference direction of flux in earth frame
    float w_bx = 0, w_by = 0, w_bz = 0; // estimate gyroscope biases error

    float SEq_1;
    float SEq_2;
    float SEq_3;
    float SEq_4;            // quaternion of sensor frame relative to auxiliary frame
    float invSampleFreq;    // sampling period in seconds
    float roll;
    float pitch;
    float yaw;
    char anglesComputed;

    void computeAngles();

//-------------------------------------------------------------------------------------------
// Function declarations
public:
    MadgwickOptim(void);
    void setTimeInterval(float samplingPeriod) { invSampleFreq = samplingPeriod; }
    void update(float w_x, float w_y, float w_z, float a_x, float a_y, float a_z, float m_x, float m_y, float m_z);
    void updateIMU(float w_x, float w_y, float w_z, float a_x, float a_y, float a_z);
    //float getPitch(){return atan2f(2.0f * q2 * q3 - 2.0f * q0 * q1, 2.0f * q0 * q0 + 2.0f * q3 * q3 - 1.0f);};
    //float getRoll(){return -1.0f * asinf(2.0f * q1 * q3 + 2.0f * q0 * q2);};
    //float getYaw(){return atan2f(2.0f * q1 * q2 - 2.0f * q0 * q3, 2.0f * q0 * q0 + 2.0f * q1 * q1 - 1.0f);};
    float getRoll() {
        if (!anglesComputed) computeAngles();
        return roll * 57.29578f;
    }
    float getPitch() {
        if (!anglesComputed) computeAngles();
        return pitch * 57.29578f;
    }
    float getYaw() {
        if (!anglesComputed) computeAngles();
        return yaw * 57.29578f + 180.0f;
    }
    float getRollRadians() {
        if (!anglesComputed) computeAngles();
        return roll;
    }
    float getPitchRadians() {
        if (!anglesComputed) computeAngles();
        return pitch;
    }
    float getYawRadians() {
        if (!anglesComputed) computeAngles();
        return yaw;
    }
};
#endif

