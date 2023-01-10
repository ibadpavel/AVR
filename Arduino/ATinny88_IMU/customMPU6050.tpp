//----------------- I2C communication -----------------
void basicMPU6050::setRegister( uint8_t reg, uint8_t mode ) {
  Wire.beginTransmission(MPU_ADDRESS);
  Wire.write(reg);
  Wire.write(mode);
  Wire.endTransmission();
}

void basicMPU6050::readRegister( uint8_t reg ) {
  Wire.beginTransmission(MPU_ADDRESS);
  Wire.write(reg);
  Wire.endTransmission();
  Wire.requestFrom(MPU_ADDRESS, 2, true);
}

int basicMPU6050::readWire() {
  return int( Wire.read()<<8|Wire.read() );
}

void basicMPU6050::setup() {
  // Turn on communication
  Wire.begin();

  // Enable sensor
  setRegister( 0x6B, 0x00 );

  // Low pass filter
  setRegister( 0x1A, DLPF_CFG <= 6 ? DLPF_CFG : 0x00 );

  // Gyro sensitivity
  setRegister( 0x1B, FS_SEL == 1 ? 0x08   :
                     FS_SEL == 2 ? 0x10   :
                     FS_SEL == 3 ? 0x18   :
                                   0x00   );
  // Accel sensitivity
  setRegister( 0x1C, AFS_SEL == 1 ? 0x08  :
                     AFS_SEL == 2 ? 0x10  :
                     AFS_SEL == 3 ? 0x18  :
                                    0x00  );
}

//---------------- Raw measurements ------------------

// Acceleration
int basicMPU6050::rawAx() {
  readRegister( 0x3B );
  return readWire();
}

int basicMPU6050::rawAy() {
  readRegister( 0x3D );
  return readWire();
}

int basicMPU6050::rawAz() {
  readRegister( 0x3F );
  return readWire();
}

// Temperature
int basicMPU6050::rawTemp() {
  readRegister( 0x41 );
  return readWire();
}

// Gyro
int basicMPU6050::rawGx() {
  readRegister( 0x43 );
  return readWire();
}

int basicMPU6050::rawGy() {
  readRegister( 0x45 );
  return readWire();
}

int basicMPU6050::rawGz() {
  readRegister( 0x47 );
  return readWire();
}

//--------------- Scaled measurements ----------------

// Acceleration
float basicMPU6050::ax() {
  const float SCALE  = (AX_S) * ACCEL_LBS;
  const float OFFSET = (AX_S) * float(AX_OFS)/ACCEL_LBS_0;

  return float(rawAx()) * SCALE - OFFSET;
}

float basicMPU6050::ay() {
  const float SCALE  = (AY_S) * ACCEL_LBS;
  const float OFFSET = (AY_S) * float(AY_OFS)/ACCEL_LBS_0;

  return float(rawAy()) * SCALE - OFFSET;
}

float basicMPU6050::az() {
  const float SCALE  = (AZ_S) * ACCEL_LBS;
  const float OFFSET = (AZ_S) * float(AZ_OFS)/ACCEL_LBS_0;

  return float(rawAz()) * SCALE - OFFSET;
}

// Temperature
float basicMPU6050::temp() {
  return rawTemp() * TEMP_MUL + 36.53;
}

// Gyro
float basicMPU6050::gx() {
  const float SCALE = (GX_S) * GYRO_LBS;
  return ( float( rawGx() ) - mean[0] )*SCALE;
}

float basicMPU6050::gy() {
  const float SCALE = (GY_S) * GYRO_LBS;
  return ( float( rawGy() ) - mean[1] )*SCALE;
}

float basicMPU6050::gz() {
  const float SCALE = (GZ_S) * GYRO_LBS;
  return ( float( rawGz() ) - mean[2] )*SCALE;
}

//--------------- Gyro bias estimate -----------------
void basicMPU6050::setBias() {
  for( int count = 0; count < N_BIAS; count += 1 ) {
    int gyro[] = { rawGx(), rawGy(), rawGz() };

    // Sum all samples
    for( int index = 0; index < N_AXIS; index += 1 ) {
      mean[index] += float( gyro[index] );
    }
  }

  // Divide sums by sample count
  for( int index = 0; index < N_AXIS; index += 1 ) {
    mean[index] *= MEAN;
  }
}

void basicMPU6050::updateBias() {
  // Error in reading
  float dw[N_AXIS] = { rawGx() - mean[0] ,
                       rawGy() - mean[1] ,
                       rawGz() - mean[2] };

  // Calculate kalman gain
  float mag = dw[0] * dw[0] + dw[1] * dw[1] + dw[2] * dw[2];
  float gain = BAND_SQ / (BAND_SQ + var + mag);

  var += mag + (gain - 1) * var;                    // covariance in the magnitude of gyro signal

  // Update mean with gain
  for( int index = 0; index < N_AXIS; index += 1 ) {
    mean[index] += dw[index] * gain * MEAN;
  }
}
