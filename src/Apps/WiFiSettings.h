#pragma once
#include "WiFiWps/WpsConnect.h"

class WifiSettingsClass
{
public:
    static char WIFI_CRYPT_KEY[];

    WifiSettingsClass();
    ~WifiSettingsClass();

    void Run();
    
private:
    void AP_Mode();
    void STA_Mode();
    void APSTA_Mode();
    void SmartConfig();
};