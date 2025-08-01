#include <SPI.h>
#include <mcp_can.h>
#include <Arduino.h>
#include "actuatorController.h"

// Define which microcontroller you are using 
// 1: Controls Leg Lengths 6 (on left) and 1 (on right)
// 2: Controls Leg Lengths 2 (on left) and 3 (on right)
// 3: Controls Leg Lengths 4 (on left) and 5 (on right)
// NOTE: THIS MUST BE UPDATED FOR EACH MICROCONTROLLER
#define MICROCONTROLLER_ID 1

// Chip select pin
const int SPI_CS_PIN = 7;

// Create an instance of the MCP_CAN class
MCP_CAN CAN(SPI_CS_PIN);

// Define the Pin Numbers

// Right Actuator Pins (from the Perspective of the MCU)
int RILI = 0; // Right Internal Logic Input
int RELI = 1; // Right External Logic Input
int RPotPin = 3; // Right Potentiometer Feedback Pin

// Left Actuator Pins (from the Perspective of the MCU)
int LILI = 4; // Left Internal Logic Input
int LELI = 5; // Left External Logic Input
int LPotPin= 6; // Left Potentiometer Feedback Pin

// Create Instances of the Actuator Controller Class for the Left and Right Actuators
ActuatorController leftActuatorController(LPotPin, LILI, LELI);
ActuatorController rightActuatorController(RPotPin, RILI, RELI);

// Buffer for leg lengths
float legLengths[6] = {0};  // leg 1 to leg 6

void setup() {
  if (CAN.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ) == CAN_OK) {
  } else {
    while (1);
  }

  CAN.setMode(MCP_NORMAL); // Normal mode

  // Set the Command Output Pins

    // Left Actuator Pins (from the Perspective of the MCU)
    pinMode(LILI, OUTPUT);
    pinMode(LELI, OUTPUT);
    pinMode(LPotPin, INPUT);

    // Right Actuator Pins (from the Perspective of the MCU)
    pinMode(RILI, OUTPUT);
    pinMode(RELI, OUTPUT);
    pinMode(RPotPin, INPUT);
}

void loop() {
  if (CAN.checkReceive() == CAN_MSGAVAIL) {
    long unsigned int rxId;
    unsigned char len = 0;
    unsigned char rxBuf[8];

    CAN.readMsgBuf(&rxId, &len, rxBuf);

    // Only process expected message IDs (0x130 - 0x135)
    if (rxId >= 0x130 && rxId <= 0x135 && len == 4) {
      int legIndex = rxId - 0x130;

      // Reconstruct float from 4 bytes (big-endian from Python)
        union {
        byte b[4];
        float f;
        } converter;

        converter.b[0] = rxBuf[3];  // LSB
        converter.b[1] = rxBuf[2];
        converter.b[2] = rxBuf[1];
        converter.b[3] = rxBuf[0];  // MSB
      legLengths[legIndex] = converter.f;

      // Serial.print("Leg ");
      // Serial.print(legIndex + 1);
      // Serial.print(": ");
      // Serial.println(legLengths[legIndex], 6); // Print with precision
    }

    // Update the actuator controllers based on the microcontroller ID
    if (MICROCONTROLLER_ID == 1) {
      // Update the left and right actuator controllers with the new leg lengths
      leftActuatorController.setTargetPosition(legLengths[5]); // Left Leg Length 6
      rightActuatorController.setTargetPosition(legLengths[0]); // Right Leg Length 1
      // Serial.print("Left Leg Length 6 Command: ");
      // Serial.println(legLengths[5]);
      // Serial.print("Right Leg Length 1 Command: ");
      // Serial.println(legLengths[0]);
    } else if (MICROCONTROLLER_ID == 2) {
      leftActuatorController.setTargetPosition(legLengths[1]); // Left Leg Length 2
      rightActuatorController.setTargetPosition(legLengths[2]); // Right Leg Length 3
      // Serial.print("Left Leg Length 2 Command: ");
      // Serial.println(legLengths[1]);
      // Serial.print("Right Leg Length 3 Command: ");
      // Serial.println(legLengths[2]); 
    } else if (MICROCONTROLLER_ID == 3) {
      leftActuatorController.setTargetPosition(legLengths[3]); // Left Leg Length 4
      rightActuatorController.setTargetPosition(legLengths[4]); // Right Leg Length 5
      // Serial.print("Left Leg Length 4 Command: ");
      // Serial.println(legLengths[3]);
      // Serial.print("Right Leg Length 5 Command: ");
      // Serial.println(legLengths[4]);
    }

    
  }
  // Provide control commands and update actuator controllers
  leftActuatorController.update();
  rightActuatorController.update();

  // Removed delay to try to reduce jitters, but add it back if necessary
}
