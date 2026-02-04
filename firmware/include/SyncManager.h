#ifndef SYNC_MANAGER_H
#define SYNC_MANAGER_H

#include <Arduino.h>
#include <SD.h>
#include <SPI.h>
#include <ArduinoJson.h>
#include <mbedtls/base64.h>

#define SD_SCK 1
#define SD_MOSI 2
#define SD_MISO 0
#define SD_CS 7

class SyncManager {
public:
    SyncManager() : _sdSPI(HSPI) {}

    void begin() {
        _sdSPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
        if (!SD.begin(SD_CS, _sdSPI)) {
            Serial.println("SD Initialization failed!");
        } else {
            Serial.println("SD Initialized.");
        }
    }

    String listFiles(const char * dirname) {
        JsonDocument doc;
        JsonArray array = doc.to<JsonArray>();
        
        File root = SD.open(dirname);
        if(!root || !root.isDirectory()){
            return "[]";
        }

        File file = root.openNextFile();
        while(file){
            JsonObject obj = array.add<JsonObject>();
            obj["n"] = String(file.name());
            obj["s"] = file.size();
            obj["d"] = file.isDirectory();
            file = root.openNextFile();
        }
        
        String output;
        serializeJson(doc, output);
        return output;
    }

    bool writeChunk(const char* path, size_t offset, uint8_t* data, size_t len) {
        File file = SD.open(path, offset == 0 ? FILE_WRITE : FILE_APPEND);
        if (!file) return false;
        file.seek(offset);
        size_t written = file.write(data, len);
        file.close();
        return written == len;
    }

    bool handleWriteChunk(JsonObject data) {
        String path = data["path"];
        size_t offset = data["offset"];
        String b64data = data["data"];
        
        size_t b64len = b64data.length();
        size_t decodedLen = (b64len * 3) / 4 + 1; 
        uint8_t* decoded = (uint8_t*)malloc(decodedLen);
        size_t actualLen = 0;
        
        int res = mbedtls_base64_decode(decoded, decodedLen, &actualLen, (const unsigned char*)b64data.c_str(), b64len);
        
        bool success = (res == 0) && writeChunk(path.c_str(), offset, decoded, actualLen);
        free(decoded);
        return success;
    }

private:
    SPIClass _sdSPI;
};

#endif
