#pragma once
#include "SD.h"

class LittleFSClass {
public:
    bool begin(bool formatOnFail = false, const char * basePath = "/littlefs", uint8_t maxOpenFiles = 10, const char * partitionLabel = "spiffs") { return true; }
    bool exists(const char* path) { return false; }
    File open(const char* path, const char* mode = "r") { return File(); }
    bool remove(const char* path) { return true; }
};

extern LittleFSClass LittleFS;