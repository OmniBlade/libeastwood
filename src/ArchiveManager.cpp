#include "eastwood/ArchiveManager.h"
#include "eastwood/Exception.h"
#include "eastwood/Log.h"
#include "eastwood/BlowFish.h"
#include "eastwood/WsKey.h"

namespace eastwood {

namespace {

//Mix constants
const uint32_t ENCRYPTED = 0x00020000;

//Iso constants
const int ISO_BLKSIZE = 2048;
const std::string ISO_ID = "CD001";

//Blank arcfileinfo to return if not found
ArcFileInfo BLANK = {0, 0, ARC_DIR, NULL, std::string()};

}
    
size_t ArchiveManager::indexDir(std::string path)
{
    struct dirent* dir;
    DIR *dp = opendir(path.c_str());
    struct stat st;
    t_arc_entry entry;
    std::pair<t_arc_index_iter,bool> rv;
    
    //check we managed to open a directory path
    if (dp == NULL) {
        throw(Exception(LOG_ERROR, "ArchiveManager", "Could not open directory"));
    }
    
    //initialise a fresh map object
    _archives.push_back(t_arc_index());
    
    while ((dir = readdir(dp)) != NULL) {
        std::string filepath;
        //ignore standard dirs
        if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0)
            continue;
        
        //check we can stat the current file
        filepath = path + DIR_SEP + dir->d_name;
        if (stat(filepath.c_str(), &st) < 0) {
            //TODO error and warnings
            LOG_WARNING("Couldn't stat %s", filepath.c_str());
            continue;
        }
        
        //ignore if a directory of any kind, sub dirs not supported
        if (S_ISDIR(st.st_mode)) continue;
        
        entry.first = idGen(dir->d_name);
        entry.second.archivepath = filepath;
        entry.second.size = st.st_size;
        entry.second.start = 0;
        entry.second.type = ARC_DIR;
        
        rv = _archives.back().insert(entry);
        
        //if insertion failed, assume it was due to id collision
        if(!rv.second) {
            LOG_WARNING("Could not index %s, likely a hash collision", dir->d_name);
        }
    }
    
    closedir(dp);
    
    return _archives.size() - 1;
}

size_t ArchiveManager::indexPak(std::string pakfile, bool usefind)
{
    //uint32_t flags;
    //uint16_t filecount;
    char name[256];
    ArcFileInfo archive;
    t_arc_entry entry;
    std::pair<t_arc_index_iter,bool> rv;
    
    if(usefind) {
        archive = find(pakfile);
        _stream.open(archive);
    } else {
        _stream.open(pakfile.c_str(), std::ios_base::binary | std::ios_base::in);
        archive.archivepath = pakfile;
        archive.start = 0;
        archive.size = _stream.sizeg();
    }
    
    if(!_stream.is_open())
        throw(Exception(LOG_ERROR, "ArchiveManager", "Could not open Pak"));
    
    //initialise a fresh map object
    _archives.push_back(t_arc_index());
    
    //get our first file offset
    uint32_t offset = _stream.getU32LE();

    while(offset) {
        uint32_t start = offset;
        uint32_t size;

	_stream.getline(name, 256, 0);
        LOG_DEBUG("Found file %s", name);

        size = ((offset = _stream.getU32LE()) != 0 ? offset : _stream.sizeg()) - start;
        
        entry.first = idGen(name);
        entry.second.start = start + archive.start;
        entry.second.size = size;
        entry.second.archivepath = archive.archivepath;
        entry.second.type = ARC_PAK;
        rv = _archives.back().insert(entry);
        
        //if insertion failed, assume bad format.
        if(!rv.second) {
            LOG_WARNING("Could not index %s, likely a hash collision", name);
        }
    }
    
    _stream.close();
    
    return _archives.size() - 1;
}

size_t ArchiveManager::indexMix(std::string mixfile, bool usefind)
{
    uint32_t flags;
    uint16_t filecount;
    //std::string archivename;
    ArcFileInfo archive;
    //std::pair<t_arc_index_iter,bool> rv;
    
    if(usefind) {
        archive = find(mixfile);
        _stream.open(archive);
    } else {
        _stream.open(mixfile.c_str(), std::ios_base::binary | std::ios_base::in);
        archive.archivepath = mixfile;
        archive.start = 0;
        archive.size = _stream.sizeg();
    }
    
    if(!_stream.is_open())
        throw(Exception(LOG_ERROR, "ArchiveManager", "Could not open Mix"));
    
    flags = _stream.getU32LE();
    filecount = *reinterpret_cast<uint16_t*>(&flags);
    
    //decide what type of mix file we are handling
    if(filecount || !(flags & ENCRYPTED)){
        handleUnEncrypted(archive, filecount);
    } else {
        handleEncrypted(archive);
    }
    
    _stream.close();
    
    return _archives.size() - 1;
}

// based on info at http://wiki.osdev.org/ISO_9660
size_t ArchiveManager::indexIso(std::string isofile, bool usefind)
{
    std::string isoid;
    ArcFileInfo archive;
    t_arc_entry entry;
    std::pair<t_arc_index_iter,bool> rv;
    unsigned int ptsize;
    unsigned int ptoffset;
    std::vector<unsigned int> diroffsets;
    std::vector<unsigned int>::iterator dirit;
    
    if(usefind) {
        archive = find(isofile);
        _stream.open(archive);
    } else {
        _stream.open(isofile.c_str(), std::ios_base::binary | std::ios_base::in);
        archive.archivepath = isofile;
        archive.start = 0;
        archive.size = _stream.sizeg();
    }
    
    //basic checks on if the file opened and is big enough
    if(!_stream.is_open())
        throw(Exception(LOG_ERROR, "ArchiveManager", "Could not open Iso"));
    
    if(_stream.sizeg() < 16 * ISO_BLKSIZE)
        throw(Exception(LOG_ERROR, "ArchiveManager", "Invalid Iso format"));
    
    //seek to start of ISO primary descriptor
    _stream.seekg(16 * ISO_BLKSIZE, std::ios_base::beg);
    
    //do some format checks
    if(_stream.get() != 1)
        throw(Exception(LOG_ERROR, "ArchiveManager", "Invalid Iso format"));
    
    isoid.resize(5);
    _stream.read(&isoid.at(0), 5);
    
    if(isoid != ISO_ID) {
        throw(Exception(LOG_ERROR, "ArchiveManager", "Invalid Iso format"));
    }
    
    LOG_DEBUG("Iso id is %s", isoid.c_str());
    
    //path_table information
    _stream.seekg(122, std::ios_base::cur);
    unsigned int blksize = _stream.getU16LE();
    _stream.ignore(2);
    if(blksize != ISO_BLKSIZE){
        LOG_DEBUG("Block size %u not as expected", blksize);
        throw(Exception(LOG_ERROR, "ArchiveManager", "Iso format not handled"));
    }
    
    ptsize = _stream.getU32LE();
    _stream.ignore(4);
    ptoffset = _stream.getU32LE();
    
    LOG_DEBUG("ptsize %d, ptoffset %d", ptsize, ptoffset);
    
    //seek to path table
    _stream.seekg(ptoffset * ISO_BLKSIZE, std::ios_base::beg);
    
    //parse the info we need from the path table
    while(ptsize > 0) {
        int idlen = _stream.get();
        ptsize -= 8 + idlen + (idlen % 2);
        _stream.ignore(1);
        diroffsets.push_back(_stream.getU32LE());
        //skip bunch of info we don't care about
        _stream.ignore(2 + idlen + (idlen % 2));
    }
    
    LOG_DEBUG("There are %u directories to index", diroffsets.size());
    
    //lookup the directory record and parse it into our archive db
    for(dirit = diroffsets.begin(); dirit != diroffsets.end(); dirit++) {
        //seek to extent entry
        _stream.seekg((*dirit * ISO_BLKSIZE) + 2, std::ios_base::beg);
        unsigned int offset = _stream.getU32LE();
        LOG_DEBUG("Dir entry %u", offset * ISO_BLKSIZE);
        _stream.ignore(4);
        unsigned int size = _stream.getU32LE();
        LOG_DEBUG("Dir entry size %u", size);
        _stream.seekg(offset * ISO_BLKSIZE, std::ios_base::beg);
        handleIsoDirRec(archive, size);
    }
    
    return _archives.size() - 1;
}

size_t ArchiveManager::indexIsz(std::string iszfile, bool usefind)
{
    return 0;
}

void ArchiveManager::handleUnEncrypted(ArcFileInfo& archive, uint16_t filecount)
{
    uint32_t offset = 6;  //at least 6 at this point
    t_arc_entry entry;
    std::pair<t_arc_index_iter,bool> rv;
    
    _archives.push_back(t_arc_index());
    
    //This ignores the body size and does no validation as the games do.
    if(filecount){
        _stream.seekg(6, std::ios_base::beg);
        LOG_DEBUG("Indexing TD format mix");
    } else {
        filecount = _stream.getU16LE();
        offset += 4;
        _stream.seekg(10, std::ios_base::beg);
        LOG_DEBUG("Indexing RA format mix");
    }
    
    //add total size of index to offset
    offset += 12 * filecount;
    
    for(uint32_t i = 0; i < filecount; i++) {
        entry.first = _stream.getU32LE();
        entry.second.start = _stream.getU32LE() + offset + archive.start;
        entry.second.size = _stream.getU32LE();
        entry.second.archivepath = archive.archivepath;
        entry.second.type = ARC_MIX;
        rv = _archives.back().insert(entry);
        
        //if insertion failed, assume bad format.
        if(!rv.second)
            throw(Exception(LOG_ERROR, "ArchiveManager", "Invalid Mix format"));
    }
    
}

void ArchiveManager::handleEncrypted(ArcFileInfo& archive)
{
    uint32_t offset = 84 + archive.start;     //at least 84 at this point
    uint32_t filecount;
    Cblowfish bfish;
    int8_t bfishbuf[8];
    uint8_t keysource[80];
    uint8_t key[56];
    uint32_t bcount;
    t_arc_entry entry;
    std::pair<t_arc_index_iter,bool> rv;
    
    
    //setup blowfish to handle header
    _stream.seekg(4, std::ios_base::beg);
    _stream.read(reinterpret_cast<char*>(keysource), 80);
    get_blowfish_key(keysource, key);
    bfish.set_key(key, 56);
    
    //read first block to get file count, needed to calculate header size
    _stream.read(reinterpret_cast<char*>(bfishbuf), 8);
    bfish.decipher(reinterpret_cast<void*>(bfishbuf), 
                    reinterpret_cast<void*>(bfishbuf), 8);
    memcpy(reinterpret_cast<char*>(&filecount), bfishbuf, 2);
    
    //caculate header size and prepare buffer for it
    bcount = ((filecount * 12) - 2) / 8;
    if (((filecount * 12) - 2) % 8) bcount++;
    //add 8 to compensate for block we already decrypted
    offset += bcount * 8 + 8;
    
    //prepare buffer for index
    char pindbuf[bcount * 8 + 2];
    memcpy(pindbuf, bfishbuf + 6 , 2);
    
    //loop to decrypt index into index buffer
    for(unsigned int i = 0; i < bcount; i++) {
        _stream.read(reinterpret_cast<char*>(bfishbuf), 8);
        bfish.decipher(reinterpret_cast<void*>(bfishbuf), 
                    reinterpret_cast<void*>(bfishbuf), 8);
        memcpy(pindbuf + 2 + 8 * i, bfishbuf, 8);
    }
    
    //get ready to create index
    _archives.push_back(t_arc_index());
    
    for(uint32_t i = 0; i < filecount; i++){
        memcpy(reinterpret_cast<char*>(&entry.first), pindbuf + i * 12,
               sizeof(int32_t));
        memcpy(reinterpret_cast<char*>(&entry.second.start), 
               pindbuf + 4 + i * 12, sizeof(int32_t));
        memcpy(reinterpret_cast<char*>(&entry.second.size), 
               pindbuf + 8 + i * 12, sizeof(int32_t));
        
        entry.second.start += offset;
        entry.second.archivepath = archive.archivepath;
        entry.second.type = ARC_MIX;
        rv = _archives.back().insert(entry);
        
        //if insertion failed, assume bad format.
        if(!rv.second)
            throw(Exception(LOG_ERROR, "ArchiveManager", "Invalid Mix format"));
    }
}


void ArchiveManager::handleIsoDirRec(ArcFileInfo& archive, unsigned int size)
{
    t_arc_entry entry;
    std::pair<t_arc_index_iter,bool> rv;
    
    //add new archive to the archive list
    _archives.push_back(t_arc_index());
    
    for(unsigned int i = 0; i < size;) {
        unsigned int reclen = _stream.get();
        if(!reclen) {
            _stream.ignore(ISO_BLKSIZE - (i - 1));
            i += ISO_BLKSIZE - (i - 1);
            LOG_DEBUG("record pos %u against size %u", i, size);
            if(i > size) break;
        }
        _stream.ignore(1);
        entry.second.start = _stream.getU32LE() * ISO_BLKSIZE;
        _stream.ignore(4);
        entry.second.size = _stream.getU32LE();
        _stream.ignore(11);
        //check if flags indicate this is a dir entry, handled by caller so skip
        //also check if file is hidden, we don't care about it if so
        uint8_t flag = _stream.get();
        if(flag & 0x02 || flag & 0x01) {
            LOG_DEBUG("record is for a dir or hidden");
            _stream.ignore(reclen - 26);
            i += reclen;
            continue;
        }
        _stream.ignore(6);
        
        //get the filename
        std::string fname;
        //miss off teminator bytes
        fname.resize(_stream.get());
        _stream.read(&fname.at(0), fname.size());
        //LOG_DEBUG("Read filename of %s", fname.c_str());
        
        //finish creating our file entry
        entry.first = idGen(fname);
        entry.second.archivepath = archive.archivepath;
        entry.second.type = ARC_ISO;
        
        rv = _archives.back().insert(entry);
        
        //if insertion failed, assume bad format.
        if(!rv.second) {
            LOG_WARNING("Could not index %s, likely a hash collision", fname.c_str());
        }
        
        //move everything to where the next entry should be
        _stream.ignore(reclen - (35 + fname.size()));
        i += reclen;
    }
}

    
//Generates the ID's found in mix files.
int32_t ArchiveManager::idGen(std::string filename)
{
    std::transform(filename.begin(), filename.end(), filename.begin(),
            (int(*)(int)) toupper); // convert to uppercase
    int i = 0;
    uint32_t id = 0;
    int l = filename.length(); // length of the filename
    while (i < l) {
        uint32_t a = 0;
        for (int j = 0; j < 4; j++) {
            a >>= 8;
            if (i < l)
                a += static_cast<uint32_t> (filename[i]) << 24;
            i++;
        }
        id = (id << 1 | id >> 31) + a;
    }
    return id;
}

ArcFileInfo& ArchiveManager::find(std::string filename)
{  
    int32_t id = idGen(filename);
    
    for(t_archive_iter it = _archives.begin(); it != _archives.end(); it++) {
        t_arc_index_iter info = it->find(id);
        if(info != it->end()) return info->second;
    }
    
    LOG_DEBUG("Couldn't find file of id %08x", id);
    return BLANK;
}

}//eastwood