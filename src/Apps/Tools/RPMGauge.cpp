#include "RPMGauge.h"

void RPMGaugeClass::Run()
{
  // Loop until the ESC button is pressed
  while (!M5m.BtnA.wasPressed()) {
    if (!ConnectELM())
    {
      M5m.Lcd.drawString(":(", 0, 40);
      sleep(1000);
    } else {
      M5m.Lcd.drawString("OH SHIT IT WORKED", 0, 40);
    }
  }

  return;
}

bool RPMGaugeClass::ConnectELM()
{
  DEBUG_PORT.println("Connecting to ODBII Module via Bluetooth...");

  retryCount = 0;
  ELM_PORT.begin("ESP32TEST", true); // Starts our module in Master mode with Broadcast name "ESP32TEST"

  while (!ELMConnected)
  {
    if (retryCount < 3) {
      ELMConnected = ELM_PORT.connect(name);
    
      if (!ELMConnected) {
        ELMConnected = ELM_PORT.connected(5000);

        if (!ELMConnected) {
          DEBUG_PORT.println("Couldn't connect to OBD scanner - Phase 1");
          DEBUG_PORT.printf("Retrying...%i\n", retryCount);
        }
      }
    } else {
      DEBUG_PORT.println("All connection attempts exhausted. Try again soon!");
      ELM_PORT.disconnect();
      return false;
    }
    retryCount++;
  }
  DEBUG_PORT.println("Bluetooth connection established.");

  if (!myELM327.begin(ELM_PORT, true, 2000))
  {
    DEBUG_PORT.println("Couldn't connect to OBD scanner - Phase 2");
    return false;
  }

  DEBUG_PORT.println("Connected to ELM327");
  return true;
}

RPMGaugeClass::RPMGaugeClass()
{
  // Make sure we're not stalled by a previous failed run
  ELMConnected = false;
  name = "VHM-ble";
  
  ELM_PORT.disconnect();

  M5m.drawAppMenu(F("RPM GAUGE"), F("ESC"), F("A"), F("B"));
  M5m.update();
}

RPMGaugeClass::~RPMGaugeClass()
{
  ELM_PORT.disconnect();

	M5m.Lcd.fillScreen(TFT_BLACK);
	M5m.show();
}