#include "RPMGauge.h"

void RPMGaugeClass::Run()
{
  unsigned long timeA, timeB = millis();
  
  ScanBLEDevices();

  if (!ConnectELM())
  {
    M5m.Lcd.println("Unable to establish connection with ELM327");
  } else {
    M5m.Lcd.clearDisplay();
    M5m.Lcd.drawString("RPM:", 10, 40);

    while(!M5m.BtnA.wasPressed()) {
      timeB = millis();

      if( (timeB - timeA) >= pollRate ) {
        rpm = myELM327.rpm();
        if( myELM327.nb_rx_state == ELM_SUCCESS) {
          M5m.Lcd.setCursor(40,40);
          M5m.Lcd.printf("%5.0f", rpm);
        }
        timeA = timeB;
      }

      M5m.BtnA.read();
    }
  }

  return;
}

bool RPMGaugeClass::InitELMPort()
{
  if(!ELMInitialized) {
    // Starts our module in Master mode with given broadcastName)
    if(!ELM_PORT.begin(broadcastName, true)) {
        M5m.Lcd.println("Failed to initialize SerialBT");
        return false;
    } else {
        ELM_PORT.setPin(pin);
        M5m.Lcd.println("Initialized SerialBT");
        ELMInitialized = true;
        return true;
    }
  } else {
    return true;
  }
}

void RPMGaugeClass::ScanBLEDevices()
{
  if (!ELM_PORT.isReady()) {
    M5m.Lcd.println("Bluetooth Module is not ready.");

    if (!InitELMPort()) {
      return;
    }
  }

  if (ELM_PORT.connected()) {
    ELM_PORT.disconnect();  // disconnect our ELM to prep for scanning
    ELMConnected = false;
  }

  bleClientDevices = ELM_PORT.getScanResults();

  // Check to see if our scan resultset is empty, and rediscover if so
  if (bleClientDevices->getCount() == 0) {
    ELM_PORT.discoverClear();

    M5m.Lcd.println("Discovering BLE devices. Please standby...");
    bleClientDevices = ELM_PORT.discover(10000);
  }
  
  int16_t deviceCount = bleClientDevices->getCount();

  if (deviceCount > 0) {
    for (int16_t i = 0; i < deviceCount; i++) {
      myBLEDevice = bleClientDevices->getDevice(i);
      M5m.Lcd.printf("\t%i:\t%s\n", i, myBLEDevice->toString().c_str());
    }
  } else {
    M5m.Lcd.println("No Bluetooth devices found.");
  }

  ELM_PORT.end(); // stop our BT device
  ELMInitialized = false;
  return;
}



bool RPMGaugeClass::ConnectELM()
{
  retryCount = 0;

  if (!ELM_PORT.isReady()) {
    M5m.Lcd.println("Bluetooth Module is not ready.");

    if (!InitELMPort())
      return false;
  }

  M5m.Lcd.println("Connecting to ODBII Module via Bluetooth...");
  ELM_PORT.disconnect();
  ELMConnected = ELM_PORT.connect(clientName);

  while (!ELMConnected)
  {
    if (retryCount < 3) {
        ELMConnected = ELM_PORT.connected(10000);

        if (!ELMConnected)
          M5m.Lcd.printf("P1 - Couldn't connect to OBD scanner - Retrying...%i\n", retryCount);
    } else {
      M5m.Lcd.println("P1 - Failed to establish BLE connection with scanner.");
      ELM_PORT.disconnect();
      return false;
    }
    retryCount++;
  }
  M5m.Lcd.println("Bluetooth connection established!");

  if (!myELM327.begin(ELM_PORT, true, 2000))
  {
    M5m.Lcd.println("P2 - Failed to establish serial session with scanner firmware.");
    return false;
  }

  M5m.Lcd.println("Connected to ELM327");
  ELMConnected = true;
  return true;
}

String RPMGaugeClass::getBLEMac()
{
    esp_read_mac(bleMacAddress, ESP_MAC_BT);
    sprintf(bleMacChr, "%02X:%02X:%02X:%02X:%02X:%02X", bleMacAddress[0], bleMacAddress[1], bleMacAddress[2], bleMacAddress[3], bleMacAddress[4], bleMacAddress[5]);
    return String(bleMacChr);
}

RPMGaugeClass::RPMGaugeClass()
{
  M5m.Lcd.clearDisplay();

  M5m.Lcd.setCursor(0, 40);
  M5m.Lcd.printf("BLE MAC: %s\n", getBLEMac().c_str());

  M5m.update();
}

RPMGaugeClass::~RPMGaugeClass()
{
  DEBUG_PORT.println("Destroying RPMGaugeClass");
  ELM_PORT.disconnect();
  ELM_PORT.discoverClear();
  ELM_PORT.end();

  myBLEDevice = NULL;
  bleClientDevices = NULL;

	M5m.Lcd.fillScreen(TFT_BLACK);
	M5m.show();
}