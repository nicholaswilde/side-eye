#pragma once
class SDClass {
public:
    uint64_t totalBytes() { return 1024 * 1024; }
    uint64_t usedBytes() { return 512 * 1024; }
};
extern SDClass SD;
