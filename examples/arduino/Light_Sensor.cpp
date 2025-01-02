#include <Arduino.h>

const int sensorPin = A0; // Pin connected to the light sensor
int sensorValue = 0;      // Variable to store the sensor value

void setup() {
    Serial2.begin(9600); // Initialize serial communication at 9600 bits per second
    pinMode(sensorPin, INPUT); // Set the sensor pin as an input
}

void loop() {
    sensorValue = analogRead(sensorPin); // Read the value from the sensor
    Serial2.print("Light Sensor Value: ");
    Serial2.println(sensorValue); // Print the sensor value to the serial monitor
    delay(1000); // Wait for 1 second before reading again
}