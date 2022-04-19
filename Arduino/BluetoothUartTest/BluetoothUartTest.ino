#include <SoftwareSerial.h>

SoftwareSerial mySoftSerial(2, 3); // RX, TX
int val;

void setup()
{
  Serial.begin(115200);       // USB-TTL
  mySoftSerial.begin(9600);   // Bluetooth
  
  //pinMode(LED, OUTPUT);
  //digitalWrite(LED, HIGH);
}

void loop()
{  
  while (mySoftSerial.available()) {
    Serial.write(mySoftSerial.read());
  }
  
  if (Serial.available()) {
    val = Serial.read();
    // При символе "1" включаем светодиод
    if (val == '1') {
      //digitalWrite(LED, HIGH);
      mySoftSerial.println("HIGH");
    }
    // При символе "0" выключаем светодиод
    if ( val == '0') {
      //digitalWrite(LED, LOW);
      mySoftSerial.println("LOW");
    }
  }
}
