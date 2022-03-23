// the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(57600);
}

// the loop routine runs over and over again forever:
void loop() {
  Serial.println("UART (57600) ready!");
  delay(500);        // delay in between reads for stability
}
