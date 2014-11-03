#ifndef EASTWOOD_IOSTREAM_H
#define EASTWOOD_IOSTREAM_H

#include <iostream>

namespace eastwood {

class IOStream : public std::iostream
{
public:
    IOStream() : std::iostream(NULL){}
    IOStream(std::streambuf *sb) : std::iostream(sb) {}
    IOStream(const std::iostream &stream);

    IOStream& operator=(const std::iostream &stream);
    IOStream& operator=(const IOStream &stream);

    uint16_t getU16BE();
    uint16_t getU16LE();
    uint32_t getU32BE();
    uint32_t getU32LE();

    IOStream& readU16BE(uint16_t *buf, size_t n);
    IOStream& readU16LE(uint16_t *buf, size_t n);
    IOStream& readU32BE(uint32_t *buf, size_t n);
    IOStream& readU32LE(uint32_t *buf, size_t n);

    std::streamsize sizeg();

    IOStream& putU16BE(uint16_t value);
    IOStream& putU16LE(uint16_t value);
    IOStream& putU32BE(uint32_t value);
    IOStream& putU32LE(uint32_t value);

    IOStream& writeU16BE(uint16_t *buf, size_t n);
    IOStream& writeU16LE(uint16_t *buf, size_t n);
    IOStream& writeU32BE(uint32_t *buf, size_t n);
    IOStream& writeU32LE(uint32_t *buf, size_t n);

    std::streamsize sizep();

private:

    template <typename T> inline
        IOStream& readT(T &value);
    template <typename T> inline
        T getT();
    template <typename T> inline
        IOStream& writeT(T &value);
    template <typename T> inline
        IOStream& putT(T value);
};

}

#endif // EASTWOOD_IOSTREAM_H
