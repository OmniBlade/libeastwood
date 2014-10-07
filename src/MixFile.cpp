#include "eastwood/StdDef.h"

#include "eastwood/Exception.h"
#include "eastwood/Log.h"
#include "eastwood/MixFile.h"
#include "eastwood/BlowFish.h"
#include "eastwood/WsKey.h"

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
    uint32_t offset = _stream.tellg();
    Cblowfish blfish;
    char pblkbuf[8];
    char keySource[80];
    char wsKey[56];
    uint16_t entryCount;
    uint32_t blockCount;
    
    //encrypted mix holds its key in a pubkey encrypted block
    _stream.read(keySource, 80);
    get_blowfish_key(reinterpret_cast<uint8_t*>(keySource),
                     reinterpret_cast<uint8_t*>(wsKey));
    blfish.set_key(reinterpret_cast<uint8_t*>(wsKey), 56);
    
    offset += 80;
    
    //we have to decrypt the first block to get the number of entries
    _stream.read(pblkbuf, 8);
    blfish.decipher(reinterpret_cast<void*>(pblkbuf), 
                    reinterpret_cast<void*>(pblkbuf), 8);
    memcpy(reinterpret_cast<char*>(&entryCount), pblkbuf, 2);
    
    /* To workout size of our header and how much we need to decrypt
     * taking into account 2 bytes left from getting the file count
     * adding 8 to compensate for block we already decrypted.
     */
    blockCount = ((entryCount * 12) - 2) / 8;
    if (((entryCount * 12) - 2) % 8) blockCount++;
    offset += blockCount * 8 + 8;
    
    char indexBuf[blockCount * 8 + 2];
    memcpy(indexBuf, pblkbuf + 6 , 2);
    
    //loop to decrypt index into index buffer
    for(int i = 0; i < blockCount; i++) {
        _stream.read(pblkbuf, 8);
        blfish.decipher(reinterpret_cast<void*>(pblkbuf), 
                    reinterpret_cast<void*>(pblkbuf), 8);
        memcpy(indexBuf + 2 + 8 * i, pblkbuf, 8);
    }
    
    for(uint32_t i = 0; i < entryCount; i++){
        int32_t fid;
        uint32_t foffset;
        uint32_t fsize;
        
        memcpy(reinterpret_cast<char*>(&fid), indexBuf + i * 12,
               sizeof(int32_t));
        memcpy(reinterpret_cast<char*>(&foffset), indexBuf + 4 + i * 12,
               sizeof(uint32_t));
        memcpy(reinterpret_cast<char*>(&fsize), indexBuf + 8 + i * 12,
               sizeof(uint32_t));
        foffset += offset;
        
        _fileEntries.insert(std::make_pair(fid, FileEntry(foffset, fsize)));
    }
}

}
