#ifndef EASTWOOD_ARCFILEINFO_H
#define	EASTWOOD_ARCFILEINFO_H

#include <string>

enum arctype {
    ARC_DIR = 0,
    ARC_PAK = 1,
    ARC_MIX = 2,
    ARC_ISO = 3,
    ARC_ISH = 4 
};

struct ArcFileInfo
{ 
    int start;
    int size;
    int cmpsize;
    arctype type;
    char* cache;
    std::string archivepath;
};

#endif	/* EASTWOOD_ARCFILEINFO_H */

