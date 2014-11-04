#ifndef EASTWOOD_ARCHIVEMANAGER_H
#define	EASTWOOD_ARCHIVEMANAGER_H

#include "StdDef.h"
#include "ArcIOStream.h"
#include "IOStream.h"
#include "DirEnt.h"
#include "ArcFileInfo.h"
#include <vector>
#include <list>
#include <map>

typedef std::map<int32_t, ArcFileInfo> t_arc_index;
typedef std::pair<int32_t, ArcFileInfo> t_arc_entry;
typedef std::map<int32_t, ArcFileInfo>::iterator t_arc_index_iter;
typedef std::list<t_arc_index>::iterator t_archive_iter;

namespace eastwood {

class ArchiveManager
{
public:
    ArchiveManager() : _archives(), _stream() 
    { }
    size_t indexDir(std::string path);
    size_t indexPak(std::string pakfile, bool usefind = false);
    size_t indexMix(std::string mixfile, bool usefind = false);
    ArcFileInfo& find(std::string filename);
    int size() { return _archives.size(); }
    bool empty() { return _archives.empty(); }

protected:
    virtual int32_t idGen(std::string filename);
    
private:
    void handleEncrypted(ArcFileInfo& archive);
    void handleUnEncrypted(ArcFileInfo& archive, uint16_t filecount);
    std::list<t_arc_index> _archives;
    ArcIOStream _stream;
};

}//eastwood

#endif	/* EASTWOOD_ARCHIVEMANAGER_H */

