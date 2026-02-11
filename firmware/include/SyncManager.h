#ifndef SYNC_MANAGER_H
#define SYNC_MANAGER_H

#include <Arduino.h>
#include <SD.h>
#include <SPI.h>
#include <ArduinoJson.h>
#include <mbedtls/base64.h>

#define SD_SCK 19
#define SD_MOSI 18
#define SD_MISO 20
#define SD_CS 23

class SyncManager {
public:
    SyncManager() {}

    void begin() {
        // Use a dedicated SPI instance for the SD card as per Waveshare demo
        static SPIClass sdSPI(FSPI);
        pinMode(SD_MISO, INPUT_PULLUP);
        sdSPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
        
        if (!SD.begin(SD_CS, sdSPI, 1000000)) {
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
            file.close();
            file = root.openNextFile();
        }
        root.close();
        
        String output;
        serializeJson(doc, output);
        return output;
    }

    bool writeChunk(const char* path, size_t offset, uint8_t* data, size_t len) {
        if (offset == 0) {
            ensureDirectory(path);
            if (SD.exists(path)) {
                SD.remove(path); // Ensure fresh file
            }
        }
        
        File file = SD.open(path, offset == 0 ? FILE_WRITE : FILE_APPEND);
        if (!file) return false;
        file.seek(offset);
        size_t written = file.write(data, len);
        file.close();
        return written == len;
    }

    void ensureDirectory(const char* path) {
        String p = String(path);
        for (int i = 0; i < p.length(); i++) {
            if (p[i] == '/') {
                if (i == 0) continue; 
                String dir = p.substring(0, i);
                if (!SD.exists(dir.c_str())) {
                    SD.mkdir(dir.c_str());
                }
            }
        }
    }

    bool handleWriteChunk(JsonObject data) {
        String path = data["path"];
        size_t offset = data["offset"];
        String b64data = data["data"];
        
        size_t b64len = b64data.length();
        size_t decodedLen = (b64len * 3) / 4 + 1; 
        uint8_t* decoded = static_cast<uint8_t*>(malloc(decodedLen));
        if (!decoded) return false;
        
        size_t actualLen = 0;
        
        int res = mbedtls_base64_decode(decoded, decodedLen, &actualLen, reinterpret_cast<const unsigned char*>(b64data.c_str()), b64len);
        
        bool success = (res == 0) && writeChunk(path.c_str(), offset, decoded, actualLen);
        free(decoded);
        return success;
    }

private:
};

#endif
