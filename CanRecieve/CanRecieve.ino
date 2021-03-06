//Code by Patrick Cruce(pcruce_at_igpp.ucla.edu) 
//Uses CAN extended library, designed for Arduino with 16 Mhz oscillator
//Library found at: http://code.google.com/p/canduino/source/browse/trunk/#trunk%2FLibrary%2FCAN
//This runs a simple test of the hardware with the MCP 2515 CAN Controller in loopback mode.
//If using over physical bus rather than loopback, and you have high bus 
//utilization, you'll want to turn the baud of the serial port up or log
//to SD card, because frame drops can occur due to the reader code being
//occupied with writing to the port.
//In our testing over a 40 foot cable, we didn't have any problems with
//synchronization or frame drops due to SPI,controller, or propagation delays
//even at 1 Megabit.  But we didn't do any tests that required arbitration
//with multiple nodes.

#include <SoftwareSerial.h>
#include <SPI.h>
#include <CAN.h>

#define BUS_SPEED 125

//global variable used to determine whether loop should
//be in Tx or Rx mode.
int state;

void setup()
{                
  state = 1; 
  Serial.begin(9600);
  Serial.println("starting CanRecieve...");
  
  // initialize CAN bus class
  // this class initializes SPI communications with MCP2515
  CAN.begin();
  CAN.setMode(CONFIGURATION);
  CAN.baudConfig(BUS_SPEED);
 
  //Wait 10 seconds so that I can still upload even
  //if the previous iteration spams the serial port
  delay(1000);

  CAN.setMode(NORMAL);  // set to "NORMAL" for standard com
  CAN.toggleRxBuffer0Acceptance(false, false); //set to true,true to disable filtering
  CAN.toggleRxBuffer1Acceptance(true, true);
  
/* set mask bit to 1 to turn on filtering for that bit
                                                   un       DATA        DATA
                                  ID MSB   ID LSB used     BYTE 0      BYTE 1
                                  [             ][   ]    [      ]    [      ]  */                                
  //CAN.setMaskOrFilter(MASK_0,   0b00000000, 0b00000000, 0b00000000, 0b00000000); //mask 0 controls filters 0 and 1 
  //CAN.setMaskOrFilter(FILTER_0, 0b00000000, 0b00000000, 0b00000000, 0b00000000); //to pass a filter, all bits in the msg id that are "masked" must be the same as in the filter.
  //CAN.setMaskOrFilter(FILTER_1, 0b00000000, 0b00000000, 0b00000000, 0b00000000); //a message will pass if at least one of the filters pass it. 
  //CAN.setMaskOrFilter(MASK_1,   0b00000000, 0b00000000, 0b00000000, 0b00000000); //mask 1 control filters 2 to 5
  //CAN.setMaskOrFilter(FILTER_2, 0b00000000, 0b00000000, 0b00000000, 0b00000000);
  //CAN.setMaskOrFilter(FILTER_3, 0b00000000, 0b00000000, 0b00000000, 0b00000000);
  //CAN.setMaskOrFilter(FILTER_4, 0b00000000, 0b00000000, 0b00000000, 0b00000000); //shows up as 0 on printBuf
  //CAN.setMaskOrFilter(FILTER_5, 0b00000000, 0b00000000, 0b00000000, 0b00000000); //shows up as 1 on printBuf
}


void printBuf(byte rx_status, byte length, uint32_t frame_id, byte filter, byte buffer, byte *frame_data, byte ext)
{       
  Serial.print("[Rx] Status:");
  Serial.print(rx_status,HEX);
      
  Serial.print(" Len:");
  Serial.print(length,HEX);
      
  Serial.print(" Frame:");
  Serial.print(frame_id,HEX);

  Serial.print(" EXT?:");
  Serial.print(ext==1,HEX);
       
  Serial.print(" Filter:");
  Serial.print(filter,HEX);

  Serial.print(" Buffer:");
  Serial.print(buffer,HEX);
      
  Serial.print(" Data:[");
  for (int i=0;i<length;i++)
  {
    if (i>0) Serial.print(" ");
    Serial.print(frame_data[i],HEX);
  }
  Serial.println("]"); 
}

void msgHandler(byte rx_status, byte length, uint32_t frame_id, byte filter, byte buffer, byte *frame_data, byte ext)
{
  //current sensor
  if(frame_id==0x7FF)
  {
    union u_tag
    {
      byte b[4];
      float fval;
    } u;

    Serial.print("Current Sensor:");
      
    u.b[0] = frame_data[0];
    u.b[1] = frame_data[1];
    u.b[2] = frame_data[2];
    u.b[3] = frame_data[3];      
    Serial.print("I=");
    Serial.print(u.fval);
      
    u.b[0] = frame_data[4];
    u.b[1] = frame_data[5];
    u.b[2] = frame_data[6];
    u.b[3] = frame_data[7];
    Serial.print(", Q=");
    Serial.println(u.fval);
  }
  else
  {
    Serial.print("unknown msg ");
    Serial.println(frame_id, HEX);
  }  
}

void loop()
{  
  byte length,rx_status,filter,ext;
  uint32_t frame_id;
  byte frame_data[8];
  
  //Rx
  //clear receive buffers, just in case.
  for(int i=0; i<8; i++) frame_data[i] = 0x00;
  frame_id = 0x0000;
  length = 0;
  rx_status = CAN.readStatus();

  if ((rx_status & 0x80) == 0x80)
  {
    Serial.println("Reading frame in buffer 0");
    CAN.readDATA_ff_0(&length,frame_data,&frame_id, &ext, &filter);
    printBuf(rx_status, length, frame_id, filter, 0, frame_data, ext);
    //msgHandler(rx_status, length, frame_id, filter, 0, frame_data, ext);
    CAN.clearRX0Status();
    rx_status = CAN.readStatus();
    Serial.println(rx_status,HEX);
  }
      
  if ((rx_status & 0x40) == 0x40)
  {
    Serial.println("Reading frame in buffer 1...");
    CAN.readDATA_ff_1(&length,frame_data,&frame_id, &ext, &filter);
    printBuf(rx_status, length, frame_id, filter, 1, frame_data, ext);       
    //msgHandler(rx_status, length, frame_id, filter, 0, frame_data, ext);
    CAN.clearRX1Status();
    rx_status = CAN.readStatus();
    Serial.println(rx_status,HEX);
  }    
  
  delay(100);  
}

