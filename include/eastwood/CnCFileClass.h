/* 
 * File:   ccfileclass.h
 * Author: aidan
 *
 * Created on 10 September 2014, 23:39
 */

#ifndef CNCFILECLASS_H
#define	CNCFILECLASS_H

#include "StdDef.h"
#include "Strings.h"
#include <vector>

#define MAX_ARCHIVES 255
#define MAX_FILES    5

namespace eastwood {

enum {
    FILE_MODE_READ       = 0x01,
    FILE_MODE_WRITE      = 0x02,
    FILE_MODE_READ_WRITE = FILE_MODE_READ | FILE_MODE_WRITE,

    FILE_MAX = 20,
    FILE_INVALID = 0xFF
};

struct WWFile{
    FILE *fp;
    uint32_t size;
    uint32_t start;
    uint32_t position;
};
    
struct WWFileInfo{
    std::string filename;
    uint32_t fileSize;
    void *buffer;
    uint32_t filePosition;
    WWFileInfo *mix;
    struct {
        uint32_t inMemory:1;
        uint32_t inMIXFile:1;
    };
};

struct WWFileInfoLinkedElem {
    WWFileInfoLinkedElem *next;
    WWFileInfo *mix;
    int32_t id;
    int32_t cd;
    WWFileInfo info;
};

class CCFileClass
{   
public:
    CCFileClass() : m_open() {}
    CCFileClass(const std::string& datadir);
    ~CCFileClass();
    bool init();
    bool init(const std::string& dir);
    void deinit();
    bool retrieve(std::string mixfile);
    bool open(const std::string& fname, uint32_t mode = FILE_MODE_READ);
    void close();
    void deleteFile(std::string fname);
    bool createFile(std::string fname);
    uint32_t read(void *buf, uint32_t length);
    uint8_t read8();
    uint16_t readle16();
    uint32_t readle32();
    uint32_t write(const void *buffer, uint32_t length);
    bool write16(uint16_t value);
    bool write32(uint32_t value);
    bool write8(uint8_t value);
    uint32_t seek(int32_t position, uint8_t mode);
    uint32_t tell();
    uint32_t getSize() const{
        return m_open.size;
    }
    
protected: 
    bool indexPlain(int8_t* head, WWFileInfo* mixfp);
    bool indexCrypt(WWFileInfo* mixfp);
    uint32_t getIndex();
    int32_t idGen(std::string fname);
    bool addFileInfo(const std::string& fname, uint32_t fsize, uint32_t index);
    WWFileInfo* finder(int32_t id, WWFileInfo** mixinfo);
    void cleanUp(uint32_t index);
    
    WWFile m_open;
    //static std::string m_datadir;
};

} //eastwood

#endif	/* CNCFILECLASS_H */

