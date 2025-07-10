#include <SPI.h>
#include <mcp_can.h>

// Chip select pin
const int SPI_CS_PIN = 7;

MCP_CAN CAN(SPI_CS_PIN);

// Buffer for leg lengths
float legLengths[6] = {0};  // leg 1 to leg 6

void setup() {
  Serial.begin(115200);
  while (!Serial) {}

  if (CAN.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ) == CAN_OK) {
    Serial.println("CAN init OK");
  } else {
    Serial.println("CAN init FAILED");
    while (1);
  }

  CAN.setMode(MCP_NORMAL); // Normal mode
  Serial.println("CAN set to normal mode");
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

      Serial.print("Leg ");
      Serial.print(legIndex + 1);
      Serial.print(": ");
      Serial.println(legLengths[legIndex], 6); // Print with precision
    }
  }
}
