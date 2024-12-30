#include <Arduino.h>
// Need to set the USER_SETUP_ID in .pio/libdeps/TFT_eSPI/User_Setup.h
#include <TFT_eSPI.h>
#include <SPI.h>
#include <CAN.h>
#include <iostream>
#include <vector>

// TODO: add credit where credit is due
// GMLAN CANBUS header defines
#include "GMLAN_11bit.h"
#include "GMLAN_29bit.h"


// Defining Canbus pins
// Defines for dev board
// #define TX_GPIO 5
// #define RX_GPIO 4
// Defines for yellow board
#define TX_GPIO 27
#define RX_GPIO 22

// Defining TFT size in pixels

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define FONT_SIZE 4

#define SPRITE_WIDTH 200
#define SPRITE_HEIGHT 40

using namespace std; 

// Create TFT obj
TFT_eSPI tft = TFT_eSPI();

//Create sprite object that points to tft obj

TFT_eSprite spr = TFT_eSprite(&tft);

uint64_t g_packetId = 0;
uint16_t vehicleSpeed = 0;
uint64_t engineRPM = 0;

vector<int> canBusData;
vector<int> canBusData2;
// TFT global packet
int packetSize;

void setup() {
  // Serial Setup
  Serial.begin(115200);
  // Start the tft display
  tft.init();
  // Set the TFT display rotation in landscape mode
  tft.setRotation(1);

  // Clear the screen before writing to it
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(FONT_SIZE);
  tft.setPivot(tft.width() / 2, tft.height() / 2);
  tft.println("Hello World!");
  tft.fillScreen(TFT_BLACK);
  // CANBUS setup
  tft.println("Setting up canbus..."); 
  CAN.setPins(RX_GPIO,TX_GPIO);
  // start the CAN bus at 500kbps
  if (!CAN.begin(500E3)) {
    Serial.println("Starting CAN failed!");
    tft.fillScreen(TFT_BLACK);
    tft.println("Starting CAN failed");
    while (1);
  }
  else
  {
    tft.fillScreen(TFT_BLACK);
    tft.println("Canbus init successful");
  }
  delay(1000);
  tft.fillScreen(TFT_BLACK);
  
  // Sprite init setup
    // Create the sprite with a defined size
  spr.setColorDepth(8);
  // spr.setRotation(1);
  spr.createSprite(SPRITE_WIDTH,SPRITE_HEIGHT);
  spr.fillSprite(TFT_BLACK);
  spr.setTextSize(FONT_SIZE);
  spr.setTextColor(TFT_WHITE);
  spr.setTextDatum(ML_DATUM);

}
void doSprite(int x, int y, int data)
{
  // Clear the sprite to write data to sprite
  spr.fillSprite(TFT_BLACK);

  // Draw text inside of the defined sprite area
  // Using center of sprite due to middle left aligned text set in setup
  spr.drawString((String)data, 0, 20);

  // Draw sprite at defined x,y location, all black pixels will not be drawn
  spr.pushSprite(x,y);
}

void parseCanPacket(uint64_t canHeaderRead)
{
  // Maybe map headers to something
  // Switch case?
  // Do something like get header, when header got get byte information position, then do conversions?
  // Need to actually get the full header, not just the arbitrary ID header
  // To get that info probably need to send 0x7DF and then read that header.
  switch (canHeaderRead)
  {
    case GMLAN_ARBID_VEHICLE_SPEED_INFORMATION:
      // Convert the vehicle speed from hex to decimal
      // TODO: Combine
      vehicleSpeed = canBusData[1] << 8; 
      vehicleSpeed = canBusData[2] | vehicleSpeed; 
      vehicleSpeed /= 10;
      // Convert the engine rpm from hex to decimal
      // TODO: Combine
      engineRPM = canBusData[3] << 8; 
      engineRPM = canBusData[4] | engineRPM;
      engineRPM *= 4;
      break;
    
    default:
      break;
  }

}

void getCANPacket()
{
  packetSize = CAN.parsePacket();
  
  if(packetSize)
  {
    // Serial.print("PacketID: ");
    g_packetId = CAN.packetId();
    // Serial.println(g_packetId,HEX);
    while(CAN.available())
    {
      canBusData.push_back(CAN.read());
    }
    // for(int i = 0; i < canBusData.size(); i++)
    // {
    //   Serial.print(canBusData[i]);
    // }
    // Serial.println(); 
    // Clears the CAN buffer after a packet read
    // canBusData2 = canBusData;
    parseCanPacket(g_packetId);
    canBusData.clear();
  }
}
void drawOnScreen()
 {

  // Display the packet ID 
  tft.setCursor(0,0);
  tft.println("PacketID:");
  doSprite(0,35,g_packetId);

  // Display the packet Data / Second line on display
  tft.setCursor(0,80);
  tft.println("Speed:");
  doSprite(0,115,vehicleSpeed);

  // Display the packet Data / Third line on display
  tft.setCursor(0,160);
  tft.println("RPM:");
  doSprite(0,195,engineRPM);

 }

void loop() {
  // put your main code here, to run repeatedly:
  getCANPacket();
  drawOnScreen();
}
