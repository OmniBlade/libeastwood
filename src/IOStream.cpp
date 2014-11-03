#include "eastwood/StdDef.h"
#include "eastwood/IOStream.h"

namespace eastwood {

IOStream::IOStream(const std::iostream &stream) :
    std::iostream(stream.rdbuf())
{
}

template <typename T> inline
IOStream& IOStream::writeT(T &value)
{
    return reinterpret_cast<IOStream&>(write(reinterpret_cast<char*>(&value), sizeof(value)));
}

template <typename T> inline
IOStream& IOStream::putT(T value)
{
    return writeT<T>(value);
}

IOStream& IOStream::putU16BE(uint16_t value)
{
    return putT<uint16_t>(htobe16(value));
}

IOStream& IOStream::putU16LE(uint16_t value)
{
    return putT<uint16_t>(htole16(value));
}

IOStream& IOStream::putU32BE(uint32_t value)
{
    return putT<uint32_t>(htobe32(value));
}

IOStream& IOStream::putU32LE(uint32_t value)
{
    return putT<uint32_t>(htole32(value));
}

#if __BYTE_ORDER == __BIG_ENDIAN
IOStream& IOStream::writeU16BE(uint16_t *buf, size_t n)
{
    return reinterpret_cast<IOStream&>(write((char*)buf, n*sizeof(buf[0])));
}

IOStream& IOStream::writeU32BE(uint32_t *buf, size_t n)
{
    return reinterpret_cast<IOStream&>(write((char*)buf, n*sizeof(buf[0])));
}

IOStream& IOStream::writeU16LE(uint16_t *buf, size_t n)
{
    for(size_t i = 0; i < n; i++)
	putU16LE(buf[i]);
    return *this;
}

IOStream& IOStream::writeU32LE(uint32_t *buf, size_t n)
{
    for(size_t i = 0; i < n; i++)
	putU32LE(buf[i]);
    return *this;
}
#else
IOStream& IOStream::writeU16BE(uint16_t *buf, size_t n)
{
    for(size_t i = 0; i < n; i++)
	putU16BE(buf[i]);
    return *this;
}

IOStream& IOStream::writeU32BE(uint32_t *buf, size_t n)
{
    for(size_t i = 0; i < n; i++)
	putU32BE(buf[i]);
    return *this;
}

IOStream& IOStream::writeU16LE(uint16_t *buf, size_t n)
{
    return reinterpret_cast<IOStream&>(write(reinterpret_cast<char*>(buf), n*sizeof(buf[0])));
}

IOStream& IOStream::writeU32LE(uint32_t *buf, size_t n)
{
    return reinterpret_cast<IOStream&>(write(reinterpret_cast<char*>(buf), n*sizeof(buf[0])));
}
#endif

std::streamsize IOStream::sizep()
{
    std::streamsize size;
    std::streampos pos = tellp();
    seekp(0, std::ios::end);
    size = static_cast<std::streamsize>(tellp());
    seekp(pos);
    return size;
}

IOStream& IOStream::operator=(const std::iostream &stream)
{
    std::ios::init(stream.rdbuf());

    return *this;
}

IOStream& IOStream::operator=(const IOStream &stream)
{
    std::ios::init(stream.rdbuf());

    return *this;
}

template <typename T> inline
IOStream& IOStream::readT(T &value) {
    return reinterpret_cast<IOStream&>(read(reinterpret_cast<char*>(&value), sizeof(value)));
}

template <typename T> inline
T IOStream::getT() 
{
    T value;
    readT<T>(value);
    return value;
}

uint16_t IOStream::getU16BE() 
{
    return htobe16(getT<uint16_t>());
}

uint16_t IOStream::getU16LE() 
{
    return htole16(getT<uint16_t>());
}


uint32_t IOStream::getU32BE() 
{
    return htobe32(getT<uint32_t>());
}

uint32_t IOStream::getU32LE() 
{
    return htole32(getT<uint32_t>());
}

#if __BYTE_ORDER == __BIG_ENDIAN
IOStream& IOStream::readU16BE(uint16_t *buf, size_t n)
{
    return reinterpret_cast<IOStream&>(read((char*)buf, n*sizeof(buf[0])));
}

IOStream& IOStream::readU32BE(uint32_t *buf, size_t n)
{
    return reinterpret_cast<IOStream&>(read((char*)buf, n*sizeof(buf[0])));
}

IOStream& IOStream::readU16LE(uint16_t *buf, size_t n)
{
    for(size_t i = 0; i < n; i++)
	buf[i] = getU16LE();
    return *this;
}

IOStream& IOStream::readU32LE(uint32_t *buf, size_t n)
{
    for(size_t i = 0; i < n; i++)
	buf[i] = getU32LE();
    return *this;
}
#else
IOStream& IOStream::readU16BE(uint16_t *buf, size_t n)
{
    for(size_t i = 0; i < n; i++)
	buf[i] = getU16BE();
    return *this;
}

IOStream& IOStream::readU32BE(uint32_t *buf, size_t n)
{
    for(size_t i = 0; i < n; i++)
	buf[i] = getU32BE();
    return *this;
}

IOStream& IOStream::readU16LE(uint16_t *buf, size_t n)
{
    return reinterpret_cast<IOStream&>(read(reinterpret_cast<char*>(buf), n*sizeof(buf[0])));
}

IOStream& IOStream::readU32LE(uint32_t *buf, size_t n)
{
    return reinterpret_cast<IOStream&>(read(reinterpret_cast<char*>(buf), n*sizeof(buf[0])));
}
#endif

std::streamsize IOStream::sizeg()
{
    std::streamsize size;
    std::streampos pos = tellg();
    seekg(0, std::ios::end);
    size = static_cast<std::streamsize>(tellg());
    seekg(pos);
    return size;
}

}
