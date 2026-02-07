#pragma once
#include <stdint.h>
#include "Arduino.h"
#include "SPI.h"
#include "LittleFS.h" // Reuse File class

#define FILE_READ "r"
#define FILE_WRITE "w"
#define FILE_APPEND "a"

class SDClass {
public:
    bool begin(uint8_t ssPin, SPIClass &spi) { return true; }
    File open(const char* path, const char* mode = FILE_READ) { 
        if (std::string(path) == "/") {
            File f("dir", true);
            f._isDir = true;
            return f;
        }
        return File("test content", true); 
    }
    uint64_t totalBytes() { return 1024*1024; }
    uint64_t usedBytes() { return 512*1024; }
};

extern SDClass SD;
