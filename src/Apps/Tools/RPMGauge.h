#pragma once
#include "M5StackMod.h"

#if !defined(CONFIG_BT_SPP_ENABLED)
#error Serial Bluetooth not available or not enabled. It is only available for the ESP32 chip.
#endif

#define ELM_PORT   SerialBT
#define DEBUG_PORT Serial

class RPMGaugeClass
{
public:
    RPMGaugeClass();
    ~RPMGaugeClass();

    void Run();
    void ScanBLEDevices();

    bool InitELMPort();
    bool ConnectELM();

private:
    BluetoothSerial SerialBT;
    ELM327 myELM327;

    uint8_t retryCount;
    uint16_t pollRate = 1000; // poll rate in ms between ELM327 reads
    uint16_t rpm;

    bool ELMInitialized = false;
    bool ELMConnected   = false;

    String getBLEMac();
    uint8_t bleMacAddress[6];
    char bleMacChr[18] = {0};

    BTScanResults *bleClientDevices;
    BTAdvertisedDevice *myBLEDevice;

    String clientName       = "VHM-classic";    // the name of the Nonda ZUS adapter, ELM327 v1.5 firmware
    String broadcastName    = "MELODYBOX";      // our announce name

    const char *pin         = "1234";           // Default PIN for ELM327 products is 1234
};