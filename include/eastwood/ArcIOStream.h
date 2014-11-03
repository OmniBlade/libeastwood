#ifndef EASTWOOD_ARCIOSTREAM_H
#define EASTWOOD_ARCIOSTREAM_H

#include "ArcStreamBuf.h"
#include "IOStream.h"

namespace eastwood {

template < typename CharType, typename CharTraits = std::char_traits <CharType> >

class basic_arciostream : public basic_eastiostream <CharType, CharTraits>
{
public:
    typedef CharType                                      char_type;
    typedef CharTraits                                    traits_type;

    typedef basic_arcstream     <char_type, traits_type>  sbuf_type;
    typedef basic_arciostream   <char_type, traits_type>  this_type;
    typedef basic_eastiostream  <char_type, traits_type>  base_type;
    
    basic_arciostream(FILE* fp = NULL) :
    base_type(new sbuf_type(fp))
    {}
    
    basic_arciostream(const char* filename, std::ios_base::openmode mode) :
    base_type((new sbuf_type)->open(filename, mode))
    {}
    
    basic_arciostream(ArcFileInfo& fileinfo) :
    base_type(new sbuf_type(fileinfo))
    {}
    
    void open(const char* filename, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out) 
    {
        sbuf_type* buf = static_cast<sbuf_type*>(this->rdbuf());
        if (!(buf->open(filename, mode)))
        this->setstate(std::ios_base::badbit);
    }
    
    void open(ArcFileInfo& fileinfo) 
    {
        sbuf_type* buf = static_cast<sbuf_type*>(this->rdbuf());
        if (!(buf->open(fileinfo)))
        this->setstate(std::ios_base::badbit);
    }
    
    void close()
    {
        sbuf_type* buf = static_cast<sbuf_type*>(this->rdbuf());
        buf->close();
    }
    
    bool is_open()
    {
        sbuf_type* buf = static_cast<sbuf_type*>(this->rdbuf());
        return buf->is_open();
    }
    
    std::streamsize sizeg()
    {
        sbuf_type* buf = static_cast<sbuf_type*>(this->rdbuf());
        return static_cast<std::streamsize>(buf->size());
    }
    
    std::streamsize sizep()
    {
        sbuf_type* buf = static_cast<sbuf_type*>(this->rdbuf());
        return static_cast<std::streamsize>(buf->size());
    } 
};

typedef basic_arciostream<char>  arciostream;
typedef basic_arciostream<char>  ArcIOStream;

}

#endif // EASTWOOD_ISTREAM_H
