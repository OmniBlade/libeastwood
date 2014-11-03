#ifndef EASTWOOD_OSTREAM_H
#define EASTWOOD_OSTREAM_H

#include <ostream>

#include "eastwood/StdDef.h"

namespace eastwood {

//extension of std::basic_ostream with endian handling for int writes
template < typename CharType, typename CharTraits = std::char_traits <CharType> >

class basic_eastostream : public std::basic_ostream <CharType, CharTraits>
{
public:
    typedef CharType                                      char_type;
    typedef CharTraits                                    traits_type;

    typedef std::basic_streambuf    <char_type, traits_type>  bbuf_type;
    typedef basic_eastostream       <char_type, traits_type>  this_type;
    typedef std::basic_ostream      <char_type, traits_type>  base_type;
    
    basic_eastostream(bbuf_type* sb) :
    base_type(sb)
    {}
    
    basic_eastostream(const base_type& sb) :
    base_type(sb.rdbuf())
    {}
    
    //this_type& operator=(const std::ostream &stream);
    //this_type& operator=(const OStream &stream);
    
private:
    template <typename T> inline
    this_type& writeT(T &value)
    {
        return reinterpret_cast<this_type&>(this->write(reinterpret_cast<char_type*>(&value), sizeof(value)));
    }

    template <typename T> inline
    this_type& putT(T value)
    {
        return writeT<T>(value);
    }

public:
    this_type& putU16BE(uint16_t value)
    {
        return putT<uint16_t>(htobe16(value));
    }

    this_type& putU16LE(uint16_t value)
    {
        return putT<uint16_t>(htole16(value));
    }

    this_type& putU32BE(uint32_t value)
    {
        return putT<uint32_t>(htobe32(value));
    }

    this_type& putU32LE(uint32_t value)
    {
        return putT<uint32_t>(htole32(value));
    }

    #if __BYTE_ORDER == __BIG_ENDIAN
    this_type& writeU16BE(uint16_t *buf, size_t n)
    {
        return reinterpret_cast<this_type&>(this->write((char*)buf, n*sizeof(buf[0])));
    }

    this_type& writeU32BE(uint32_t *buf, size_t n)
    {
        return reinterpret_cast<this_type&>(this->write((char*)buf, n*sizeof(buf[0])));
    }

    this_type& writeU16LE(uint16_t *buf, size_t n)
    {
        for(size_t i = 0; i < n; i++)
            putU16LE(buf[i]);
        return *this;
    }

    this_type& writeU32LE(uint32_t *buf, size_t n)
    {
        for(size_t i = 0; i < n; i++)
            putU32LE(buf[i]);
        return *this;
    }
    #else
    this_type& writeU16BE(uint16_t *buf, size_t n)
    {
        for(size_t i = 0; i < n; i++)
            putU16BE(buf[i]);
        return *this;
    }

    this_type& writeU32BE(uint32_t *buf, size_t n)
    {
        for(size_t i = 0; i < n; i++)
            putU32BE(buf[i]);
        return *this;
    }

    this_type& writeU16LE(uint16_t *buf, size_t n)
    {
        return reinterpret_cast<this_type&>(this->write(reinterpret_cast<char*>(buf), n*sizeof(buf[0])));
    }

    this_type& writeU32LE(uint32_t *buf, size_t n)
    {
        return reinterpret_cast<this_type&>(this->write(reinterpret_cast<char*>(buf), n*sizeof(buf[0])));
    }
    #endif

    virtual std::streamsize sizep()
    {
        std::streamsize size;
        std::streampos pos = this->tellp();
        this->seekp(0, std::ios::end);
        size = static_cast<std::streamsize>(this->tellp());
        this->seekp(pos);
        return size;
    }
 
};

typedef basic_eastostream<char>  ostream;
typedef basic_eastostream<char>  OStream;

}

#endif // EASTWOOD_OSTREAM_H

