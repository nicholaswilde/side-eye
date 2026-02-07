#include "Arduino.h"
#include "WiFi.h"
#include "SD.h"
#include "LittleFS.h"
#include "SPI.h"

unsigned long _mock_millis = 0;
int _mock_digitalRead_val = HIGH;
SerialMock Serial;
WiFiClass WiFi;
ESPClass ESP;
SDClass SD;
LittleFSClass LittleFS;
SPIClass SPI;
