#include "RPMGauge.h"

void RPMGaugeClass::Run()
{
    M5m.update();
    M5m.drawAppMenu(F("RPM GAUGE"), F("ESC"), F("A"), F("B"));
}

RPMGaugeClass::RPMGaugeClass()
{
  DEBUG_PORT.begin(115200);
  //SerialBT.setPin("1234");
  ELM_PORT.begin("MELODY", true);
  
  if (!ELM_PORT.connect("OBDII"))
  {
    DEBUG_PORT.println("Couldn't connect to OBD scanner - Phase 1");
    return;
  }

  if (!myELM327.begin(ELM_PORT, true, 2000))
  {
    Serial.println("Couldn't connect to OBD scanner - Phase 2");
    return;
  }

  Serial.println("Connected to ELM327");
}

RPMGaugeClass::~RPMGaugeClass()
{
	M5m.Lcd.fillScreen(TFT_BLACK);
	M5m.show();
}