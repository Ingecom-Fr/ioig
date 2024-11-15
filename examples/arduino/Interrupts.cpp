const byte outPin = 12;
const byte interruptPin = 13;
volatile byte state = LOW;

void blink();

void setup() {
  pinMode(outPin, OUTPUT);
  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), blink, CHANGE);
}

void loop() {
  state = 0;
  digitalWrite(outPin, state);   
  delay(1000);
  state = 1;
  digitalWrite(outPin, state);   
  delay(1000);
}

void blink() {
  printf("ISR callback, state=%d\n", state);
}