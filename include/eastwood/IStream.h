#ifndef EASTWOOD_ISTREAM_H
#define EASTWOOD_ISTREAM_H

#include <istream>

#include "eastwood/StdDef.h"

namespace eastwood {

//extension of std::basic_istream with endian handling for int reads
template < typename CharType, typename CharTraits = std::char_traits <CharType> >

class basic_eastistream : public std::basic_istream <CharType, CharTraits>
{
public:
    typedef CharType                                      char_type;
    typedef CharTraits                                    traits_type;

    typedef std::basic_streambuf    <char_type, traits_type>  bbuf_type;
    typedef basic_eastistream       <char_type, traits_type>  this_type;
    typedef std::basic_istream      <char_type, traits_type>  base_type;
    
    basic_eastistream() {}
    
    basic_eastistream(bbuf_type* sb) :
    base_type(sb)
    {}
    
    basic_eastistream(const base_type& sb) :
    base_type(sb.rdbuf())
    {}
    
    //this_type& operator=(const std::iostream &stream);
    //this_type& operator=(const IOStream &stream);
    
private:
    template <typename T> inline
    this_type& readT(T &value) 
    {
        return reinterpret_cast<this_type&>(this->read(reinterpret_cast<char_type*>(&value), sizeof(value)));
    }
    
    template <typename T> inline
    T getT()
    {
        T value;
        readT<T>(value);
        return value;
    }

public:
    uint16_t getU16BE() 
    {
        return htobe16(getT<uint16_t>());
    }

    uint16_t getU16LE() 
    {
        return htole16(getT<uint16_t>());
    }


    uint32_t getU32BE() 
    {
        return htobe32(getT<uint32_t>());
    }

    uint32_t getU32LE() 
    {
        return htole32(getT<uint32_t>());
    }

    #if __BYTE_ORDER == __BIG_ENDIAN
    this_type& readU16BE(uint16_t *buf, size_t n)
    {
        return reinterpret_cast<this_type&>(this->read((char*)buf, n*sizeof(buf[0])));
    }

    this_type& readU32BE(uint32_t *buf, size_t n)
    {
        return reinterpret_cast<this_type&>(this->read((char*)buf, n*sizeof(buf[0])));
    }

    this_type& readU16LE(uint16_t *buf, size_t n)
    {
        for(size_t i = 0; i < n; i++)
            buf[i] = getU16LE();
        return *this;
    }

    this_type& readU32LE(uint32_t *buf, size_t n)
    {
        for(size_t i = 0; i < n; i++)
            buf[i] = getU32LE();
        return *this;
    }
    #else
    this_type& readU16BE(uint16_t *buf, size_t n)
    {
        for(size_t i = 0; i < n; i++)
            buf[i] = getU16BE();
        return *this;
    }

    this_type& readU32BE(uint32_t *buf, size_t n)
    {
        for(size_t i = 0; i < n; i++)
            buf[i] = getU32BE();
        return *this;
    }

    this_type& readU16LE(uint16_t *buf, size_t n)
    {
        return reinterpret_cast<this_type&>(this->read(reinterpret_cast<char*>(buf), n*sizeof(buf[0])));
    }

    this_type& readU32LE(uint32_t *buf, size_t n)
    {
        return reinterpret_cast<this_type&>(this->read(reinterpret_cast<char*>(buf), n*sizeof(buf[0])));
    }
    #endif

    virtual std::streamsize sizeg()
    {
        std::streamsize size;
        std::streampos pos = this->tellg();
        this->seekg(0, std::ios::end);
        size = static_cast<std::streamsize>(this->tellg());
        this->seekg(pos);
        return size;
    }
 
};

typedef basic_eastistream<char>  istream;
typedef basic_eastistream<char>  IStream;
    
}

#endif // EASTWOOD_ISTREAM_H
