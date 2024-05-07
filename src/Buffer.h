#ifndef CNC_3018_BUFFER_H
#define CNC_3018_BUFFER_H

#include <Arduino.h>

template<const size_t SIZE, typename Storage_Type = char>
class Buffer {
public:
    using value_type = char;

    const size_t MAX_SIZE = SIZE / sizeof(Storage_Type);

    template<typename T>
    struct Iter {
        friend class Buffer<SIZE>;

        using value_type = T;
        using pointer = value_type*;  // or also value_type*
        using reference = value_type&;  // or also value_type&
        using const_reference = const value_type&;  // or also value_type&
        size_t BASE = SIZE / sizeof(T);

        size_t bufferIndex;
        Buffer<SIZE, Storage_Type>& bufferRef;

        explicit Iter(size_t index, Buffer<SIZE, Storage_Type>& _buf) : bufferRef{_buf} {
            bufferIndex = index;
        }

        const_reference operator*() {
            size_t index = bufferIndex % BASE;
            return reinterpret_cast<const_reference>( bufferRef.buffer[index * sizeof(T)]);
        }

        Iter& operator++() {
            bufferIndex++;
            return *this;
        }

        friend bool operator!=(const Iter& a, const Iter& b) {
            return a.bufferIndex != b.bufferIndex;
        };

    };

    explicit Buffer() : _writeIndex{0}, size{0}, _tailIndex{0} {}

    template<typename G, size_t MAX_SIZE = SIZE / sizeof(G)>
    void push(const G& value) {
        assert_param(size < MAX_SIZE);
        if (size < MAX_SIZE) {
            value_type* p = ((value_type*) buffer) + _writeIndex * sizeof(G);
            memcpy(p, &value, sizeof(G));
            ++_writeIndex;
            _writeIndex = _writeIndex % MAX_SIZE;
            size++;
            _tailIndex = _tailIndex < MAX_SIZE ? ++_tailIndex : _tailIndex;
        }
    }

    void operator--() {
        size = size <= 0 ? 0 : --size;
    }

    template<typename G = Storage_Type>
    inline bool full() const {
        return size >= (MAX_SIZE / sizeof(G));
    }

    inline bool empty() const {
        return size == 0;
    }

    void clear() {
        size = 0;
        _writeIndex = 0;
        _tailIndex = 0;
    }

    template<typename Iter_Access_Type=Storage_Type>
    Iter<Iter_Access_Type> tail(uint8_t diff) {
        return (tailSize() >= diff) ? Iter<Iter_Access_Type>(MAX_SIZE + _writeIndex - diff, *this)
                                    : end<Iter_Access_Type>();
    }

    template<typename Iter_Access_Type=Storage_Type>
    Iter<Iter_Access_Type> readEnd() {
        return Iter<Iter_Access_Type>(MAX_SIZE + _writeIndex - size, *this);
    }

    template<typename Iter_Access_Type=Storage_Type>
    Iter<Iter_Access_Type> end() {
        return Iter<Iter_Access_Type>(MAX_SIZE + _writeIndex, *this);
    }

private:
    inline int8_t tailSize() const {
        return _tailIndex % (MAX_SIZE + 1);
    }

    value_type buffer[SIZE];

    size_t _writeIndex, size, _tailIndex;
};

#endif //CNC_3018_BUFFER_H
