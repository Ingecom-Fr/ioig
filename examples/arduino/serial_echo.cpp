#include <Arduino.h>


void setup() {
  // Initialize serial communication at 115200 baud rate
  Serial2.begin(115200);
}

void loop() {
  // Generate a random sensor value (simulating sensor data)
  int sensorValue = random(0, 1024);
  
  // Create a message to send
  String message = "Sensor value: " + String(sensorValue);
  
  // Send the message via Serial2
  Serial2.println(message);

  // // Check if data is available to read
  // if (Serial2.available() > 0) {
  //   // Read the incoming string
  //   String receivedMessage = Serial2.readStringUntil('\n');
    
  //   // Process and display the received message
  //   Serial2.print("Received: ");
  //   Serial2.println(receivedMessage);
    
  //   // Optional: You could add further processing here
  //   // For example, extracting the numeric value
  //   int sensorReading = receivedMessage.substring(15).toInt();
    
  //   // Do something with the sensor reading if needed
  //   if (sensorReading > 500) {
  //     Serial2.println("High reading detected!");
  //   }
  // }  
  
  // Wait for a second before sending next reading
  delay(1000);
}
