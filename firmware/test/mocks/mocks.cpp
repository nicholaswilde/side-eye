#include "Arduino.h"
#include "WiFi.h"
#include "SD.h"
#include "LittleFS.h"
#include "SPI.h"

unsigned long _mock_millis = 0;
int _mock_digitalRead_val = HIGH;
int _mock_analogWrite_val = 0;
uint8_t _mock_analogWrite_pin = 0;
SerialMock Serial;
WiFiClass WiFi;
ESPClass ESP;
SDClass SD;
LittleFSClass LittleFS;
SPIClass SPI;

std::map<std::string, std::string> _mock_sd_files;
std::map<std::string, std::string> _mock_lfs_files;
