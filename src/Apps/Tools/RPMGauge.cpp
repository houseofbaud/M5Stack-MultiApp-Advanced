#include "BluetoothSerial.h"
#include "ELMduino.h"

#include "RPMGauge.h"

BluetoothSerial SerialBT;
#define ELM_PORT   SerialBT
#define DEBUG_PORT Serial


ELM327 myELM327;


uint32_t rpm = 0;

void RPMGaugeClass::Run()
{
    M5m.update();
}

RPMGaugeClass::RPMGaugeClass()
{
#if LED_BUILTIN
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
#endif

  DEBUG_PORT.begin(115200);
  //SerialBT.setPin("1234");
  ELM_PORT.begin("ArduHUD", true);
  
  if (!ELM_PORT.connect("OBDII"))
  {
    DEBUG_PORT.println("Couldn't connect to OBD scanner - Phase 1");
  }

  if (!myELM327.begin(ELM_PORT, true, 2000))
  {
    Serial.println("Couldn't connect to OBD scanner - Phase 2");
  }

  Serial.println("Connected to ELM327");    
}

RPMGaugeClass::~RPMGaugeClass()
{
}


void setup()
{

}


void loop()
{

}