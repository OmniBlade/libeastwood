/* 
 * File:   MixFile.h
 * Author: fbsagr
 *
 * Created on September 19, 2014, 10:01 AM
 */

#ifndef EASTWOOD_MIXFILE_H
#define	EASTWOOD_MIXFILE_H

#include <map>

#include "eastwood/IOStream.h"

namespace eastwood {
    
typedef std::pair<uint32_t, uint32_t> FileEntry;
    
class MixFile : public IOStream
{
public:
    MixFile(std::iostream &stream);
    ~MixFile();
    
    void close();
    void open(std::string fileName, std::ios::openmode mode = std::ios_base::in | std::ios_base::binary);
    
    bool is_open() const throw() {
        return rdbuf() != NULL;
    }
    
    bool exists(std::string fileName) throw() {
        int32_t id = idGen(fileName);
        return _fileEntries.find(id) != _fileEntries.end();
    }

    uint32_t entries() const throw() {
        return _fileEntries.size();
    }
    
    // If negative, file needs to be truncated
    int32_t sizediff();
    
private:
    void readIndex();
    void readEncryptedIndex();
    int32_t idGen(std::string &fileName);

    std::ios_base::openmode _mode;
    std::map<int32_t, FileEntry>::iterator _currentFile;
    IOStream &_stream;
    std::map<int32_t, FileEntry> _fileEntries;
};
    
}

#endif	/* EASTWOOD_MIXFILE_H */

