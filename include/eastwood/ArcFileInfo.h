#ifndef EASTWOOD_ARCFILEINFO_H
#define	EASTWOOD_ARCFILEINFO_H

#include <string>

enum arctype {
    ARC_DIR = 0,
    ARC_PAK = 1,
    ARC_MIX = 2
};

struct ArcFileInfo
{ 
    int start;
    int size;
    arctype type;
    bool cached;
    std::string archivepath;
};

#endif	/* EASTWOOD_ARCFILEINFO_H */

