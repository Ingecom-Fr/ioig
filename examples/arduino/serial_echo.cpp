#include <Arduino.h>


void setup() {
  // Initialize serial communication at 115200 baud rate
  Serial2.begin(115200);
}

void loop() {
  // Generate a random sensor value (simulating sensor data)
  int randValue = random(0, 1024);
  
  //stdout print
  printf("Sent message....: ");

  // Create a message to send
  String message = "UART data [" + String(randValue) + "]";
  
  // Send the message via Serial2
  Serial2.println(message);

  // Check if data is available to read
  if (Serial2.available() > 0) {
    
    // Read the incoming string
    String receivedMessage = Serial2.readStringUntil('\n');
    
    //stdout print
    printf("Received message: %s\n" , receivedMessage.c_str());

  }  
  
  // Wait for a second before sending next reading
  delay(1000);
}
