#include <mcp_can.h>
#include <SPI.h>
#include <ubcsolar_can_ids.h>

#define ON 1
#define OFF 0
#define CAN_LED 7

const int SPI_CS_PIN = 9;

MCP_CAN CAN(SPI_CS_PIN);

void setup() {
  Serial.begin(115200);
  int canSSOffset = 0;

  START_INIT:

    if(CAN_OK == CAN.begin(CAN_125KBPS)) {
      Serial.println("CAN BUS Shield init ok.");
    }

    else {
      Serial.println("CAN BUS Shield init fail");
      Serial.print("Init CAN BUS Shield again with SS pin");
      Serial.println(SPI_CS_PIN + canSSOffset);

      delay(100);

      canSSOffset ^= 1;
      CAN = MCP_CAN(SPI_CS_PIN + canSSOffset);
      goto START_INIT;
    }

}

unsigned char stmp[1] = {0};

void loop() {
  char c;

  if(Serial.available()) {
    c = Serial.read();

    if (c == '1') {
      Serial.println("1");
      Serial.println("Turn LED ON");
      stmp[0] = ON;
      CAN.sendMsgBuf(CAN_LED, 0, 1, stmp);
    }

    else if (c == '0') {
      Serial.println("0");
      Serial.println("Turn LED OFF");
      stmp[0] = OFF;
      CAN.sendMsgBuf(CAN_LED, 0, 1, stmp);
    }
  }
}
