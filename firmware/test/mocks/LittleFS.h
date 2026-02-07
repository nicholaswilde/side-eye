#pragma once
#include <string>
#include <map>
#include <stdint.h>

class File {
public:
    File() : _content(""), _valid(false), _pos(0), _isDir(false), _nextCalled(false) {}
    File(std::string content, bool valid = true) : _content(content), _valid(valid), _pos(0), _isDir(false), _nextCalled(false) {}
    operator bool() const { return _valid; }
    size_t write(const uint8_t *buf, size_t size) { return size; }
    size_t write(uint8_t c) { return 1; }
    void close() {}
    int read() { 
        if (_pos < _content.length()) return (uint8_t)_content[_pos++];
        return -1;
    }
    bool isDirectory() { return _isDir; }
    File openNextFile() { 
        if (_isDir && !_nextCalled) {
            _nextCalled = true;
            return File("test", true);
        }
        return File(); 
    }
    const char* name() { return "test"; }
    size_t size() { return _content.length(); }
    void seek(size_t pos) { _pos = pos; }

    std::string _content;
    bool _valid;
    size_t _pos;
    bool _isDir;
    bool _nextCalled;
};

class LittleFSClass {
public:
    bool begin(bool formatOnFail = false, const char * basePath = "/littlefs", uint8_t maxOpenFiles = 10, const char * partitionLabel = "spiffs") { return true; }
    bool exists(const char* path) { return _files.count(path) > 0; }
    File open(const char* path, const char* mode = "r") { 
        if (_failNextOpen) {
            _failNextOpen = false;
            return File("", false);
        }
        if (_files.count(path)) return File(_files[path], true);
        if (std::string(mode) == "w") return File("", true); // Allow creating new files
        return File("", false); 
    }
    bool remove(const char* path) { 
        _files.erase(path);
        return true; 
    }
    void _setFile(std::string path, std::string content) {
        _files[path] = content;
    }
    void _setFailNextOpen(bool f) { _failNextOpen = f; }
private:
    std::map<std::string, std::string> _files;
    bool _failNextOpen = false;
};

extern LittleFSClass LittleFS;