#pragma once
#include "M5StackMod.h"

#define ELM_PORT   SerialBT
#define DEBUG_PORT Serial

class RPMGaugeClass
{
public:
    RPMGaugeClass();
    ~RPMGaugeClass();

    void Run();

private:
    BluetoothSerial SerialBT;
    ELM327 myELM327;

    uint32_t rpm = 0;
};