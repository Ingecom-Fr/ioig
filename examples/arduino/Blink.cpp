void setup()
{
  pinMode(13, OUTPUT);          // sets the digital pin 13 as output
}

void loop()
{
  digitalWrite(13, HIGH);       // sets the digital pin 13 on
  delay(5000);                  // waits for a second
  digitalWrite(13, LOW);        // sets the digital pin 13 off
  delay(5000);                  // waits for a second
}
