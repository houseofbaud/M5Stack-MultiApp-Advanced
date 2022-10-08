#include "BluetoothSerial.h"
#include "ELMduino.h"


BluetoothSerial SerialBT;
#define ELM_PORT   SerialBT
#define DEBUG_PORT Serial


ELM327 myELM327;


uint32_t rpm = 0;

