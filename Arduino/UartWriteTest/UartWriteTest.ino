int start_millis = 0;
volatile byte counter = 0;

void setup() {
  Serial.begin(115200);

  pinMode(2, INPUT_PULLUP);
  attachInterrupt(0, isr, CHANGE);
}

void isr() {
  counter = digitalRead(2);  
}

void loop() {
  start_millis = millis();
  Serial.print(millis() - start_millis);
  Serial.println(counter); 
}
