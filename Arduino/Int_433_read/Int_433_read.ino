volatile byte counter = 0;
volatile bool flag = false;

void setup() {
  Serial.begin(115200);

  pinMode(2, INPUT_PULLUP);
  attachInterrupt(0, isr, CHANGE);
}

void isr() {
  counter = digitalRead(2);
  flag = true;  
}

void loop() {
  if (flag) {
    Serial.print(millis());
    Serial.print(" ");
    Serial.println(counter);    
    flag = false;
  } 
}
