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
    bool ConnectELM();
    bool IsELMConnected();

private:
    BluetoothSerial SerialBT;
    ELM327 myELM327;

    bool ELMConnected;
    uint8_t retryCount;
    uint32_t rpm;

    //String MACadd = "AA:BB:CC:11:22:33";
    //uint8_t address[6]  = {0xAA, 0xBB, 0xCC, 0x11, 0x22, 0x33};

    String name;  // the name of the Nonda ZUS adapter, ELM327 v1.5 firmware
    const char *pin = "1234"; //<- standard pin would be provided by default


};