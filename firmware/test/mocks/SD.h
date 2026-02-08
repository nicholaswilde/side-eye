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
            return File("dir", nullptr, true, true);
        }
        
        bool writing = (std::string(mode) == FILE_WRITE || std::string(mode) == FILE_APPEND);
        if (_mock_sd_files.count(path)) {
             return File(path, &_mock_sd_files, true);
        }
        
        if (writing) {
            _mock_sd_files[path] = "";
            return File(path, &_mock_sd_files, true);
        }
        
        return File(path, &_mock_sd_files, false); 
    }
    
    bool exists(const char* path) {
        return _mock_sd_files.count(path) > 0;
    }
    
    bool mkdir(const char* path) {
        // Just verify we can call it. In mock, folders are implicit by file paths?
        // Or we can add a dummy entry
        _mock_sd_files[path] = "DIR"; 
        return true;
    }

    bool rmdir(const char* path) {
        _mock_sd_files.erase(path);
        return true;
    }
    
    uint64_t totalBytes() { return 1024*1024; }
    uint64_t usedBytes() { return 512*1024; }
};

extern SDClass SD;
