#include "RPMGauge.h"

void RPMGaugeClass::Run()
{
  float tempRPM = 0.00;
  unsigned long timeA, timeB = millis();
  
  if (!VIRTUAL_ELM_ENABLED) {
    if (!ScanBLEDevices()) return;
  }

  InitELMPort();
  if (!ConnectELM() || !ELMInitialized)
  {
    M5m.Lcd.println("Unable to establish connection with ELM327");
  } else {
    M5m.Lcd.clearDisplay();
    ReadVIN();

    while(!M5m.BtnA.wasPressed()) {
      timeB = millis();

      if( (timeB - timeA) >= pollRate ) {
        if ( (ELMState != ELM_GETTING_MSG) && (ELMState != ELM_SUCCESS) ) {
          M5m.Lcd.setCursor((5)*20, (5)*2);
          M5m.Lcd.printf("STATE: %i", ELMState);
        }

        GetVehicleData();
        timeA = timeB;
      }

      // read the state of all our buttons
      M5m.update();
    }
  }

  return;
}

void RPMGaugeClass::InitELMPort()
{
  if (VIRTUAL_ELM_ENABLED) {
    VIRT_ELM_PORT.begin(115200);
    M5m.Lcd.println("VIRTUAL MODE - Serial2 Started at 115200 baud");
    ELMInitialized = true;
  }

  if(!ELMInitialized) {
    // Starts our module in Master mode with given broadcastName)
    if(!ELM_PORT.begin(broadcastName, true)) {
        M5m.Lcd.println("Failed to initiate Bluetooth");
        ELMInitialized = false;
    } else {
        ELM_PORT.setPin(pin);
        M5m.Lcd.println("Successfully initialized Bluetooth");
        ELMInitialized = true;
    }
  }

  return;
}

bool RPMGaugeClass::ScanBLEDevices()
{
  bool deviceFound = false;

  InitELMPort();

  if (!ELMInitialized) {
    M5m.Lcd.println("Serial communication is not ready.");
    return false;
  }

  M5m.Lcd.println("Discovering client devices - please stand by...");
  bleClientDevices = ELM_PORT.discover(10000);
  
  int16_t deviceCount = bleClientDevices->getCount();

  if (deviceCount > 0) {
    for (int16_t i = 0; i < deviceCount; i++) {
      myBLEDevice = bleClientDevices->getDevice(i);

      if ( myBLEDevice->haveName() ) {
        if (strcmp(myBLEDevice->getName().c_str(),clientName.c_str()) == 0) {
          deviceFound = true;
          M5m.Lcd.printf("\t%i:\t%s\t%s\n", i, myBLEDevice->getName().c_str(), myBLEDevice->getAddress().toString().c_str());
          break;
        } else {
          DEBUG_PORT.printf("MISS: %s\n", myBLEDevice->getName().c_str());
        }
      }
    }
  } else {
    M5m.Lcd.println("No Bluetooth devices found.");
    deviceFound = false;
  }

  ELM_PORT.discoverClear();
  ELM_PORT.discoverAsyncStop();
  ELM_PORT.end();

  return deviceFound;
}

bool RPMGaugeClass::ConnectELM()
{
  uint8_t retryCount, protocolID = 0;

  if (!VIRTUAL_ELM_ENABLED) {
    if (!ELMInitialized) {
      M5m.Lcd.println("Internal Bluetooth module is not ready.");
      return false;
    }

    M5m.Lcd.println("Connecting to ELM327...");
    ELMConnected = ELM_PORT.connect(clientName);

    while (!ELMConnected)
    {
      if (retryCount < 3) {
          ELMConnected = ELM_PORT.connected(10000);

          if (!ELMConnected) {
            M5m.Lcd.printf("Could not connect to scanner. Retrying...(%i)\n", retryCount);
            break;
          }
      } else {
        M5m.Lcd.println("Failed to establish connection with scanner.");
        ELM_PORT.disconnect();
        return false;
      }
      retryCount++;
    }
    M5m.Lcd.println("Bluetooth connection established!");
  }

  if (VIRTUAL_ELM_ENABLED) {
    for( protocolID = 0; protocolID < 10; protocolID++ ) {
      DEBUG_PORT.printf("Switched to Protocol ID %i - Attempting to connect...\n", protocolID);
      ELMConnected = myELM327.begin(VIRT_ELM_PORT, false, 1000, protocolID, 64, 0);
      
      if( ELMConnected ) {
        break;
      } else {
        continue;
      }
    }
  } else {
    ELMConnected = myELM327.begin(ELM_PORT, false, 2000);
  }
  
  if (!ELMConnected) {
    M5m.Lcd.println("Failed to establish communication with ELM327.");
    return false;
  } else {
    M5m.Lcd.println("Connected to ELM327!");
  }

  return true;
}

void RPMGaugeClass::ReadVIN()
{
  unsigned long timeA, timeB = millis();
  ELMState = myELM327.nb_rx_state;

  if ( (ELMState != ELM_GETTING_MSG) && (ELMState != ELM_SUCCESS) ) {
    DEBUG_PORT.printf("ELM not ready - could not read VIN. STATE: %i\n", ELMState);
    myELM327.printError();
    return;
  } else {
    ELMState = myELM327.get_vin_blocking(myVIN);
   
    if( (ELMState != ELM_SUCCESS) ) {
      DEBUG_PORT.printf("Unsuccessful attempt at getting VIN. STATE: %i", ELMState);
      myELM327.printError();
      return;
    }
  }

  DEBUG_PORT.printf("VIN: %s\n", myVIN);
  M5m.Lcd.drawString("VIN: ", (5)*19, 230);
  M5m.Lcd.drawString(myVIN, (5)*24, 230);
  return;
}

void RPMGaugeClass::GetVehicleData()
{
  uint32_t returnValue;
  char     *funcName;

  if( functionIndex >= 6 ) functionIndex = 0;

  ELMNamedFunction statusCall[] {
      { "RPM: ",  "[%i] %s:\t\t%u\n",   true, &ELM327::rpm },               // RPM
      { "DC V",   "[%i] %s:\t\t%.1\n",   false, &ELM327::batteryVoltage },    // DC Voltage, float
      { "RAD C",  "[%i] %s:\t\t%u\n",   false, &ELM327::engineCoolantTemp }, // Celcius
      { "LOAD %", "[%i] %s:\t\t%u\n",   false, &ELM327::engineLoad },        // Percentage
      { "IAT",    "[%i] %s:\t\t%i\n",   false, &ELM327::intakeAirTemp },      // Celcius
      { "MPH",    "[%i] %s:\t\t%u\n",   true, &ELM327::mph }
  };

  if (!ELMConnected) {
    M5m.Lcd.println("Not connected to ELM327 - Could not collect vehicle data");
    return;
  }

  funcName = statusCall[functionIndex].name;

  if( statusCall[functionIndex].enabled ) {
    returnValue = (myELM327.*statusCall[functionIndex].function)();
    ELMState    = myELM327.nb_rx_state;
  } else {
    //DEBUG_PORT.printf("Function %s disabled - skipping\n", funcName);
    functionIndex+=1;
    return;
  }

  if( (ELMState != ELM_SUCCESS) && (ELMState != ELM_GETTING_MSG) ) {
    myELM327.printError();
    functionIndex+=1;
  } else if ( ELMState == ELM_SUCCESS ) {
    DEBUG_PORT.printf(statusCall[functionIndex].format, millis(), funcName, returnValue);
    functionIndex+=1;
  }

  return;
}

String RPMGaugeClass::getBLEMac()
{
    esp_read_mac(bleMacAddress, ESP_MAC_BT);
    sprintf(bleMacChr, "%02X:%02X:%02X:%02X:%02X:%02X", bleMacAddress[0], bleMacAddress[1], bleMacAddress[2], bleMacAddress[3], bleMacAddress[4], bleMacAddress[5]);
    return String(bleMacChr);
}

RPMGaugeClass::RPMGaugeClass()
{
  myVIN = (char*)malloc(18*sizeof(char));

  rpm = 0;

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
  free(myVIN);

	M5m.Lcd.fillScreen(TFT_BLACK);
	M5m.show();
}