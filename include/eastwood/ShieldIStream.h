#ifndef EASTWOOD_ISHISTREAM_H
#define EASTWOOD_ISHISTREAM_H

#include "IStream.h"
#include "BlastStreamBuf.h"

#include "eastwood/StdDef.h"

namespace eastwood {

template < typename CharType, typename CharTraits = std::char_traits <CharType> >

class basic_shieldistream : public basic_eastistream <CharType, CharTraits>
{
public:
    typedef CharType                                      char_type;
    typedef CharTraits                                    traits_type;

    typedef basic_blaststream           <char_type, traits_type>  sbuf_type;
    typedef std::basic_streambuf        <char_type, traits_type>  bbuf_type;
    typedef basic_shieldistream         <char_type, traits_type>  this_type;
    typedef basic_eastistream           <char_type, traits_type>  base_type;
    
    basic_shieldistream(std::vector<char>* source = NULL, int size = 0) :
    base_type(new sbuf_type(source, size))
    {}  
    
    basic_shieldistream(bbuf_type* sb) :
    base_type(reinterpret_cast<sbuf_type*>(sb))
    {}
    
    basic_shieldistream(const base_type& sb) :
    base_type(reinterpret_cast<sbuf_type*>(sb.rdbuf()))
    {}
    
    basic_shieldistream(ArcFileInfo& fileinfo) :
    base_type(new sbuf_type(fileinfo))
    {}
    
    void open(ArcFileInfo& fileinfo) 
    {
        if(fileinfo.type == ARC_ISH){
            LOG_DEBUG("opening file from InstallShield Archive");
            sbuf_type* buf = static_cast<sbuf_type*>(this->rdbuf());
            if (!(buf->open(fileinfo)))
                this->setstate(std::ios_base::badbit);
        }
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
};

typedef basic_arcistream<char>  shieldistream;
typedef basic_arcistream<char>  ShieldIStream;

}

#endif // EASTWOOD_ISHISTREAM_H
