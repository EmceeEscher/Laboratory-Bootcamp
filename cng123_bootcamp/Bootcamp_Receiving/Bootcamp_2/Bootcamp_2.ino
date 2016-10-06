#include <SPI.h>
#include <mcp_can.h>
#include <ubcsolar_can_ids.h>

const int SPI_CS_PIN = 9;
MCP_CAN CAN(SPI_CS_PIN);

#define LED 7
#define CAN_LED 7
#define HIGH 1
#define LOW 0

int LED_State = 0;

void setup() {
  Serial.begin(115200);

  int canSSOffset = 0;

  pinMode(LED, OUTPUT);

  START_INIT:

  if(CAN_OK == CAN.begin(CAN_125KBPS)) {
    Serial.println("CAN BUS Shield init ok.");
  }

  else {
    Serial.println("CAN BUS Shield init fail");
    Serial.println("Init CAN BUS Shield again with SS pin ");
    Serial.println(SPI_CS_PIN + canSSOffset);

    delay(100);

    canSSOffset ^= 1;
    CAN = MCP_CAN(SPI_CS_PIN + canSSOffset);
    goto START_INIT;
  }
}

void loop() {

  unsigned char len = 0, buf[8], canID;

  if(5 == CAN.checkReceive()) { //5 should be CAN_MSGAVAIL, but CAN_MSGAVAIL is not recognized, so 5 is placed here just to see if the rest compiles
    CAN.readMsgBuf(&len, buf);

    canID = CAN.getCanId();
    Serial.println("get data from ID:");
    Serial.println(canID);

    for(int i = 0; i < len; i++) {
      Serial.print(buf[i]);
      Serial.print("\t");
    }

    if (canID == CAN_LED) {
      if (buf[0] == 1) {
        LED_State = HIGH;
        Serial.println("LED should be on"); 
      }
      else if(buf[0] == 0) {
        LED_State = LOW;
        Serial.println("LED should be off");
      }
    }
  }

  digitalWrite(LED, LED_State);
}
