#include "eastwood/StdDef.h"

#include "eastwood/Exception.h"
#include "eastwood/Log.h"
#include "eastwood/MixFile.h"

namespace eastwood {

MixFile::MixFile(std::iostream& stream) :
    _mode(std::ios::in), _currentFile(), _stream(reinterpret_cast<IOStream&>(stream)),
    _fileEntries()
{
    readIndex();
}

MixFile::~MixFile()
{
    close();
}

void MixFile::close()
{
    if(is_open()) {
        delete rdbuf();
        std::ios::init(NULL);
        _mode = std::ios::in;
    }
}

inline int32_t MixFile::idGen(std::string &fileName)
{
    std::transform(fileName.begin(), fileName.end(), fileName.begin(),
            (int(*)(int)) toupper); // convert to uppercase
    int i = 0;
    uint32_t id = 0;
    int l = fileName.length(); // length of the filename
    while (i < l) {
        uint32_t a = 0;
        for (int j = 0; j < 4; j++) {
            a >>= 8;
            if (i < l)
                a += static_cast<uint32_t> (fileName[i]) << 24;
            i++;
        }
        id = (id << 1 | id >> 31) + a;
    }
    return id;
}

void MixFile::open(std::string fileName, std::ios::openmode mode)
{
    close();
    int32_t fileID = idGen(fileName);
    
    _mode |= mode;
    _currentFile = _fileEntries.find(fileID);
    if(_currentFile != _fileEntries.end()) {
        _stream.seekg(_currentFile->second.first, std::ios::beg);
        if(_mode & std::ios::trunc)
            throw(FileException(LOG_ERROR, "MixFile", "*.mix", "Writing to MixFile not supported."));
        else {
            std::string buffer(_currentFile->second.second, 0);
            _stream.read(const_cast<char*>(reinterpret_cast<const char*>(buffer.data())), buffer.size());
            std::ios::init(new std::stringbuf(buffer, _mode));
        }
    } else if(_mode & std::ios_base::out) {
        throw(FileException(LOG_ERROR, "MixFile", "*.mix", "Writing to MixFile not supported."));
    } else
        throw(FileNotFoundException(LOG_ERROR, "MixFile", fileName));
}

void MixFile::readIndex()
{
    if(!_stream.sizeg())
        return;
    
    const uint32_t encFlag = 0x00020000;
    uint32_t offset = _stream.tellg();
    uint32_t flags = _stream.getU32LE();
    uint16_t entryCount = *reinterpret_cast<uint16_t*>(&flags);
            
    //Test if the file is encrypted and if it has flags at all
    if(flags & encFlag && !entryCount) {
        readEncryptedIndex();
        return;
    } else if (!entryCount) {
        entryCount = _stream.getU16LE();
        offset += 10;
    } else {
        offset += 6;
    }
    
    _stream.seekg(offset, std::ios::beg);
    offset += 12 * entryCount;      //header is followed by index entries of 12 bytes
    
    while(entryCount){
        int32_t fid = _stream.getU32LE();
        uint32_t foffset = _stream.getU32LE() + offset;
        uint32_t fsize = _stream.getU32LE();
        
        _fileEntries.insert(std::make_pair(fid, FileEntry(foffset, fsize)));
        
        entryCount--;
    }
}

void MixFile::readEncryptedIndex()
{
    throw(FileException(LOG_ERROR, "MixFile", "*.mix", "Encrypted Mix not handled yet."));
}

}