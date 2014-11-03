#ifndef EASTWOOD_ARCOSTREAM_H
#define EASTWOOD_ARCOSTREAM_H

#include "OStream.h"
#include "ArcStreamBuf.h"

#include "eastwood/StdDef.h"

namespace eastwood {

template < typename CharType, typename CharTraits = std::char_traits <CharType> >

class basic_arcostream : public basic_eastostream <CharType, CharTraits>
{
public:
    typedef CharType                                      char_type;
    typedef CharTraits                                    traits_type;

    typedef basic_arcstream         <char_type, traits_type>  sbuf_type;
    typedef std::basic_streambuf    <char_type, traits_type>  bbuf_type;
    typedef basic_arcostream        <char_type, traits_type>  this_type;
    typedef basic_eastostream       <char_type, traits_type>  base_type;
    
    basic_arcostream(FILE* fp = NULL) :
    base_type(new sbuf_type(fp))
    {}
    
    basic_arcostream(bbuf_type* sb) :
    base_type(reinterpret_cast<sbuf_type*>(sb))
    {}
    
    basic_arcostream(const base_type& sb) :
    base_type(reinterpret_cast<sbuf_type*>(sb.rdbuf()))
    {}
    
    basic_arcostream(const char* filename, std::ios_base::openmode mode = std::ios_base::out) :
    base_type((new sbuf_type)->open(filename, mode))
    {}
    
    basic_arcostream(ArcFileInfo& fileinfo) :
    base_type(new sbuf_type(fileinfo))
    {}
    
    void open(const char* filename, std::ios_base::openmode mode = std::ios_base::out) 
    {
        sbuf_type* buf = static_cast<sbuf_type*>(this->rdbuf());
        if(!(buf->open(filename, mode))) this->setstate(std::ios_base::badbit);
    }
    
    void open(ArcFileInfo& fileinfo) 
    {
        sbuf_type* buf = static_cast<sbuf_type*>(this->rdbuf());
        if(!(buf->open(fileinfo))) this->setstate(std::ios_base::badbit);
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
    
    std::streamsize sizep()
    {
        sbuf_type* buf = static_cast<sbuf_type*>(this->rdbuf());
        return static_cast<std::streamsize>(buf->size());
    }
};

typedef basic_arcostream<char>  arcostream;
typedef basic_arcostream<char>  ArcOStream;

}

#endif // EASTWOOD_ARCOSTREAM_H

