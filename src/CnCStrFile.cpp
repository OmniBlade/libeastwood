#include "eastwood/CnCStrFile.h"
#include "eastwood/Strings.h"
#include "eastwood/Log.h"
//#include <sys/stat.h>
//#include <sys/types.h>

namespace eastwood {

CCStringFile::CCStringFile(CCFileClass& file) :
_strings(0)
{
    uint8_t* buf;
    unsigned int count;
    
    //read file into our buffer
    buf = new uint8_t[file.getSize()];
    file.read(buf, file.getSize());
    file.close();
    
    //first short is offset to end of header, halved is number of entries
    count = *reinterpret_cast<uint16_t*>(buf) / 2;
    
    uint8_t* header;
    uint8_t* string;
    
    //read offset for each entry then use string constructor to grab them up to \0
    for(unsigned int i = 0; i < count; i++){
        //set all our pointers to point at start of data
        header = string = buf;
        //move header to the entry offset info
        header += i * 2;
        //use header offset info to move string to where our string is
        string += *reinterpret_cast<uint16_t*>(header);
        //string constructor makes string from \0 terminated entry
        std::string str = reinterpret_cast<char*>(string);
        //add string to our vector
        _strings.push_back(str);
    }
        
    delete[] buf;
}

void CCStringFile::list()
{
    for(uint32_t i = 0; i < _strings.size(); i++){
        std::string str = getString(i);
        printf("%d: %s\n", i, str.c_str());
    }
}

}