#ifndef EASTWOOD_BLASTSTREAM_H
#define	EASTWOOD_BLASTSTREAM_H

#include "eastwood/ArcFileInfo.h"
#include "eastwood/Log.h"
#include "eastwood/StdDef.h"
#include "eastwood/codec/blast.h"
#include "eastwood/ArcIStream.h"

#include <streambuf>
//#include <vector>
#include <string>
//#include <cstdio>
//#include <iostream>

namespace eastwood {

/**
 * @Brief Cached stream buffer for implode compressed data.
 */

const uint32_t BLAST_CHUNK = 16384;
    
template < typename CharType, typename CharTraits = std::char_traits <CharType> >
    
class basic_blaststream : public std::basic_streambuf <CharType, CharTraits>
{
private:
    struct blastinfo {
        unsigned inputsize;
        char* inputbuf;
    };
    
public:
    typedef CharType                                char_type;
    typedef CharTraits                              traits_type;
    typedef typename traits_type::int_type          int_type;
    typedef typename traits_type::pos_type          pos_type;
    typedef typename traits_type::off_type          off_type;
    typedef std::ios_base::seekdir                  seekdir;
    typedef std::ios_base::openmode                 openmode;
    
    typedef basic_blaststream <char_type, traits_type> this_type;
    
    basic_blaststream(std::vector<char>& source, int size) 
        :_buffer(NULL), _bufsize(size), _inputinfo(source.size(), NULL)
    {
        _buffer = new char[_bufsize];
        char* end = _buffer + _bufsize;
        char* decomp = _buffer;
        _inputinfo.inputbuf = &source.at(0);
        int rv = codec::blast(infbuf, &_inputinfo, outf, &decomp);
        
        //handle if decompression fails.
        if(rv != 0) {
            delete[] _buffer;
            _buffer = NULL;
            _bufsize = 0;
        } else {
            this->setg(_buffer, _buffer, end);
        }
    }
    
    basic_blaststream(ArcFileInfo& fileinfo) 
        :_buffer(NULL), _bufsize(fileinfo.size), _inputinfo(fileinfo.cmpsize, fileinfo.cache)
    {
        ArcIStream stream(fileinfo);
        _buffer = new char[_bufsize];
        char* end = _buffer + _bufsize;
        //this pointer is for the blast implementation to use to write to _buffer
        char* decomp = _buffer;
        int rv = codec::blast(infstream, &stream, outf, &decomp);
        
        //handle if decompression fails.
        if(rv != 0) {
            delete[] _buffer;
            _buffer = NULL;
            _bufsize = 0;
        } else {
            this->setg(_buffer, _buffer, end);
        }
    }
    
    ~basic_blaststream() { this->close(); }
    
    this_type* open(ArcFileInfo& fileinfo)
    {
        if(!fileinfo.cmpsize) return NULL;
        
        return this;
    }
    
    this_type* close()
    {
        _bufsize = 0;
        if(_buffer != NULL)
        {
            delete[] _buffer;
            _buffer = NULL;
        }
        return this;
    }
    
    bool is_open() { return _buffer != NULL; }
    
    int size() { return _bufsize; }
    
protected:
    virtual int_type underflow()
    {
        if(this->gptr() < this->egptr()){
            return traits_type::to_int_type(*(this->gptr()));
        } else {
            return traits_type::eof();
        }
    }
    
    virtual int_type pbackfail(int_type c)
    {
        if (!is_open() || c != *(this->gptr()))
          return traits_type::eof();
        return c;
    }
    
    virtual std::streamsize xsgetn(char* dest, std::streamsize n)
    {
        std::streamsize nread = 0;

        if(this->gptr() >= this->egptr()) return 0;

        if(this->gptr() + n < this->egptr()) {
            nread = n;
        } else {
            nread = n - ((this->gptr() + n) - this->egptr());
        }
        
        memcpy(dest, *(this->gptr()), nread);
        this->gbump(nread);
        
        return nread;
    }
    
    virtual pos_type seekoff(off_type offset, seekdir direction, openmode mode = std::ios_base::in | std::ios_base::out)
    {
        if(direction == std::ios_base::beg) {
            this->setg(_buffer, _buffer + offset, this->egptr());
        } else if (direction == std::ios_base::cur) {
            this->gbump(offset);
        } else {
            this->setg(_buffer, this->egptr() + offset, this->egptr());
        }
        
        return this->gptr() - _buffer;
    }
    
    virtual pos_type seekpos(pos_type offset, openmode mode = std::ios_base::in | std::ios_base::out)
    {
        return seekoff(offset, std::ios_base::beg, mode);
    }
    
private:
    basic_blaststream(const basic_blaststream&);
    basic_blaststream& operator=(const basic_blaststream&);
    
    char* _buffer;
    int _bufsize;
    blastinfo _inputinfo;
    
    //how will be passed the pointer to our input buffer
    static unsigned infbuf(void *how, unsigned char **buf)
    {
        blastinfo* info = static_cast<blastinfo*>(how);
        *buf = info->inputbuf;
        return info->inputbuf != NULL ? info->inputsize : 0;
    }
    
    static unsigned infstream(void *how, unsigned char **buf)
    {
        unsigned count;
        unsigned char hold[BLAST_CHUNK];
        *buf = hold;
        ArcIStream* stream = static_cast<ArcIStream*>(how);
        
        for(count = 0; count < BLAST_CHUNK; count++) {
            if(stream->eof()) return count;
            hold[count] = stream->get();
        }
        
        return BLAST_CHUNK;
        //return fread(hold, 1, BLAST_CHUNK, (FILE *)how);
    }
    
    //how will be passed a pointer to the pointer to our output buffer
    static int outf(void *how, unsigned char *buf, unsigned len)
    {
        char** outbuf = static_cast<char**>(how);
        bool rv = memcpy(*outbuf, buf, len) != NULL;
        *outbuf += len;
        return rv;
    }
    
};
    
}//eastwood

#endif	/* EASTWOOD_BLASTSTREAM_H */

