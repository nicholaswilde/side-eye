#pragma once
#include <stdint.h>
#include "Arduino.h"
#include "SPI.h"

#define FILE_READ "r"
#define FILE_WRITE "w"
#define FILE_APPEND "a"

class File {
public:
    File() : _valid(false) {}
    operator bool() const { return _valid; }
    bool isDirectory() { return false; }
    File openNextFile() { return File(); }
    const char* name() { return "test"; }
    size_t size() { return 0; }
    void seek(size_t pos) {}
    size_t write(const uint8_t *buf, size_t size) { return size; }
    size_t write(uint8_t c) { return 1; }
    int read() { return -1; }
    void close() {}
    bool _valid;
};

class SDClass {
public:
    bool begin(uint8_t ssPin, SPIClass &spi) { return true; }
    File open(const char* path, const char* mode = FILE_READ) { return File(); }
    uint64_t totalBytes() { return 1024*1024; }
    uint64_t usedBytes() { return 512*1024; }
};

extern SDClass SD;