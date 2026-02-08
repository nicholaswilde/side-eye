#pragma once
#include <string>
#include <map>
#include <stdint.h>
#include <vector>

extern std::map<std::string, std::string> _mock_sd_files;
extern std::map<std::string, std::string> _mock_lfs_files;

class File {
public:
    File() : _path(""), _fs(nullptr), _content(""), _valid(false), _pos(0), _isDir(false), _nextCalled(false) {}
    
    // FS-backed constructor
    File(std::string path, std::map<std::string, std::string>* fs, bool valid = true, bool isDir = false) 
        : _path(path), _fs(fs), _content(""), _valid(valid), _pos(0), _isDir(isDir), _nextCalled(false) {
        if (_fs && _valid && !_isDir) {
            _content = (*_fs)[_path];
        }
    }
    
    // Content-backed constructor (for mocks/dummies)
    File(std::string content, bool valid = true) 
        : _path(""), _fs(nullptr), _content(content), _valid(valid), _pos(0), _isDir(false), _nextCalled(false) {}
        
    operator bool() const { return _valid; }
    
    size_t write(const uint8_t *buf, size_t size) { 
        if (!_valid) return 0;
        
        // Write to local buffer
        if (_pos > _content.length()) {
            _content.resize(_pos, '\0');
        }
        _content.replace(_pos, size, (char*)buf, size);
        // If replace didn't extend (replacing inside string)
        if (_pos + size > _content.length()) {
            // It should have extended if we resized or if it was at end
            // Just force resize if needed? string::replace extends if needed? 
            // Actually replace behaves differently.
            // Let's rely on explicit loop or resize.
        }
        
        // Let's use robust manual update
        if (_pos + size > _content.size()) {
            _content.resize(_pos + size);
        }
        for(size_t i=0; i<size; i++) {
            _content[_pos + i] = (char)buf[i];
        }
        _pos += size;
        
        // Write-through to FS
        if (_fs) {
            (*_fs)[_path] = _content;
        }
        return size; 
    }
    
    size_t write(uint8_t c) { return write(&c, 1); }
    
    void close() {}
    
    int read() { 
        if (!_valid) return -1;
        if (_pos < _content.length()) return (uint8_t)_content[_pos++];
        return -1;
    }
    
    bool isDirectory() { return _isDir; }
    
    File openNextFile() { 
        if (_isDir && !_nextCalled) {
            _nextCalled = true;
            return File("test", true); // Returns file with content "test"
        }
        return File(); 
    }
    
    const char* name() { return "test"; }
    
    size_t size() { return _content.length(); }
    
    void seek(size_t pos) { _pos = pos; }

    std::string _path;
    std::map<std::string, std::string>* _fs;
    std::string _content;
    bool _valid;
    size_t _pos;
    bool _isDir;
    bool _nextCalled;
};

class LittleFSClass {
public:
    bool begin(bool formatOnFail = false, const char * basePath = "/littlefs", uint8_t maxOpenFiles = 10, const char * partitionLabel = "spiffs") { return true; }
    
    bool exists(const char* path) { return _mock_lfs_files.count(path) > 0; }
    
    File open(const char* path, const char* mode = "r") { 
        if (_failNextOpen) {
            _failNextOpen = false;
            return File("", nullptr, false);
        }
        if (_mock_lfs_files.count(path)) return File(path, &_mock_lfs_files, true);
        if (std::string(mode) == "w") {
            _mock_lfs_files[path] = "";
            return File(path, &_mock_lfs_files, true); 
        }
        return File("", nullptr, false); 
    }
    
    bool remove(const char* path) { 
        _mock_lfs_files.erase(path);
        return true; 
    }
    
    void _setFile(std::string path, std::string content) {
        _mock_lfs_files[path] = content;
    }
    
    void _setFailNextOpen(bool f) { _failNextOpen = f; }
    
private:
    bool _failNextOpen = false;
};

extern LittleFSClass LittleFS;