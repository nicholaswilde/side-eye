#include "Arduino.h"
#include "WiFi.h"
#include "SD.h"
#include "LittleFS.h"
#include "SPI.h"
#include "ElegantOTA.h"
#include "WebServer.h"
#include "HTTPClient.h"
#include "HTTPUpdate.h"
#include "TJpg_Decoder.h"

unsigned long _mock_millis = 0;
int _mock_digitalRead_val = HIGH;
int _mock_analogWrite_val = 0;
uint8_t _mock_analogWrite_pin = 0;
uint32_t _mock_sd_frequency = 0;
SerialMock Serial;
WiFiClass WiFi;
ESPClass ESP;
SDClass SD;
LittleFSClass LittleFS;
SPIClass SPI;
ElegantOTAClass ElegantOTA;
HTTPUpdate httpUpdate;
TJpg_Decoder TJpgDec;

std::map<std::string, std::string> _mock_sd_files;
std::map<std::string, std::string> _mock_lfs_files;
