#include "usart.h"
#include "MadgwickAHRS.h"
#include "sensors.h"
#include "types.h"
#include <SoftwareSerial.h>
SoftwareSerial mySoftSerial(2, 3); // RX, TX

//== GLOBAL STATIC VARIABLES =============
imu_t imuData;
Madgwick imuFilter;
//========================================

void setup()
{
  USART::usart_init(57600);       // USB-TTL 115200 чёт не работает. только через Serial (жрёт место)
  mySoftSerial.begin(9600);   // Bluetooth
}

void loop()
{
  mySoftSerial.println("AR_ACC");
      
  char buf[6] = {0, 0, 0, 0, 0, 0};
  //buf[0] = sizeof(imu_t);
  //buf[1] = sizeof(float);
  if (USART::data_available()) {
    USART::usart_read_bytes(buf, 8);

    USART::usart_send_str("Rx: ");
    USART::usart_send_str(buf);
    USART::usart_send_str("\r\n");
  }// else {
  //  USART::usart_send_str("Empty buf\r\n");
  //}

  getSensors(&imuData);
  imuFilter.updateIMU(imuData.ax, imuData.ay, imuData.az, imuData.gx, imuData.gy, imuData.gz);

  delay(500);
  
  //buf[0] = (uint8_t)imuFilter.getRoll();
  //buf[1] = (uint8_t)imuFilter.getPitch();
  //buf[2] = (uint8_t)imuFilter.getYaw();
  //USART::usart_send_str(buf);
  USART::usart_send_str("NL\r\n");
}
