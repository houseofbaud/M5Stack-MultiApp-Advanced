#pragma once
#include "M5StackMod.h"

#if !defined(CONFIG_BT_SPP_ENABLED)
#error Serial Bluetooth not available or not enabled. It is only available for the ESP32 chip.
#endif

#define DEBUG_PORT      Serial
#define VIRT_ELM_PORT   Serial2
#define ELM_PORT        SerialBT

#define VIRTUAL_ELM_ENABLED true

typedef float (ELM327::*VehicleQueryFunction)(void);

typedef struct {
    char*   name;
    char*   format;
    bool    enabled;
    VehicleQueryFunction function;
} ELMNamedFunction;

class RPMGaugeClass
{
public:
    RPMGaugeClass();
    ~RPMGaugeClass();

    void Run();
    void GetVehicleData();
    void ReadVIN();

    bool ScanBLEDevices();
    void InitELMPort();
    bool ConnectELM();

private:
    BluetoothSerial SerialBT;
    String getBLEMac();

    uint8_t bleMacAddress[6];
       char bleMacChr[18] = {0};

    BTScanResults      *bleClientDevices;
    BTAdvertisedDevice *myBLEDevice;

    const String   clientName       = "VHM-classic";    // the name of the Nonda ZUS adapter, ELM327 v1.5 firmware
    const String   broadcastName    = "MELODYBOX";      // our announce name

    const   char   *pin             = "1234";           // Default PIN for ELM327 products is 1234

    const uint16_t pollRate         = 1;    // poll rate in ms between ELM327 reads
 
    // ELMDuino configuration and state
    ELM327  myELM327;
    uint8_t ELMState        = ELM_GETTING_MSG;
    bool    ELMInitialized  = false;
    bool    ELMConnected    = false;

    // Vehicle Properties
    char    *myVIN;
    float   rpm;
    
    uint8_t functionIndex = 0;
};