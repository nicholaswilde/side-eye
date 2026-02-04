#ifndef HISTORY_BUFFER_H
#define HISTORY_BUFFER_H

#include <stddef.h>

template <typename T, size_t Size>
class HistoryBuffer {
public:
    HistoryBuffer() : _head(0), _count(0) {}

    void push(T value) {
        _buffer[_head] = value;
        _head = (_head + 1) % Size;
        if (_count < Size) {
            _count++;
        }
    }

    T get(size_t index) const {
        if (index >= _count) return T();
        size_t pos = (_head + Size - _count + index) % Size;
        return _buffer[pos];
    }

    size_t count() const {
        return _count;
    }

    size_t size() const {
        return Size;
    }

    T max() const {
        if (_count == 0) return T();
        T m = _buffer[0];
        for (size_t i = 1; i < _count; i++) {
            if (_buffer[i] > m) m = _buffer[i];
        }
        return m;
    }

private:
    T _buffer[Size];
    size_t _head;
    size_t _count;
};

#endif
