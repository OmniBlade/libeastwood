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
//This specializes the indexing of a CD to parts relevant to C&C games only
const std::string indexdirs[] = {"INSTALL", "AUD1", "SETUP"};
const int numdirs = 3;

//InstallShield Constants
const uint32_t ISZ_SIG = 0x8C655D13;
const int32_t ISZ_DATASTART = 255;

//Blank arcfileinfo to return if not found
ArcFileInfo BLANK = {0, 0, 0, false, ARC_DIR, NULL, std::string()};

}

bool matchdir(const std::string& dir)
{
    for(int i = 0; i < numdirs; i++){
        if(indexdirs[i] == dir) {
            LOG_DEBUG("Matching dirs true");
            return true;
        }
    }
    //LOG_DEBUG("Matching dirs false");
    return false;
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
        entry.second.size = entry.second.decmpsize = st.st_size;
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
        entry.second.size = entry.second.decmpsize = size;
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
    
    if(!_stream.is_open()) {
        LOG_DEBUG("Couldn't open file");
        throw(Exception(LOG_ERROR, "ArchiveManager", "Could not open Mix"));
    }
    
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
    
    //seek to path table
    _stream.seekg(ptoffset * ISO_BLKSIZE, std::ios_base::beg);
    
    //parse the info we need from the path table
    while(ptsize > 0) {
        int idlen = _stream.get();
        ptsize -= 8 + idlen + (idlen % 2);
        _stream.ignore(1);
        uint32_t offset = _stream.getU32LE();
        //skip bunch of info we don't care about
        _stream.ignore(2);
        std::string name;
        name.resize(idlen);
        _stream.read(&name.at(0), idlen);
        _stream.ignore(idlen % 2);
        if(idlen < 2 || matchdir(name)) {
            diroffsets.push_back(offset);
        }
    }
    
    LOG_DEBUG("There are %u directories to index", diroffsets.size());
    int i = 1;
    //lookup the directory record and parse it into our archive db
    for(dirit = diroffsets.begin(); dirit != diroffsets.end(); dirit++) {
        //seek to extent entry
        _stream.seekg((*dirit * ISO_BLKSIZE) + 2, std::ios_base::beg);
        unsigned int offset = _stream.getU32LE();
        _stream.ignore(4);
        unsigned int size = _stream.getU32LE();
        _stream.seekg(offset * ISO_BLKSIZE, std::ios_base::beg);
        handleIsoDirRec(archive, size);
    }
    
    _stream.close();
    
    return _archives.size() - 1;
}

size_t ArchiveManager::indexIsz(std::string iszfile, bool usefind)
{
    uint32_t sig;
    int32_t tocaddress;
    uint16_t dircount;
    std::streampos pretoc;
    ArcFileInfo archive;
    t_arc_entry entry;
    std::pair<t_arc_index_iter,bool> rv;
    
    if(usefind) {
        archive = find(iszfile);
        _stream.open(archive);
    } else {
        _stream.open(iszfile.c_str(), std::ios_base::binary | std::ios_base::in);
        archive.archivepath = iszfile;
        archive.start = 0;
        archive.size = _stream.sizeg();
    }
    
    //basic checks on if the file opened
    if(!_stream.is_open())
        throw(Exception(LOG_ERROR, "ArchiveManager", "Could not open InstallShield file"));
    
    sig = _stream.getU32LE();
    
    //test if we have what we think we have
    if(sig != ISZ_SIG)
        throw(Exception(LOG_ERROR, "ArchiveManager", "Not a valid InstallShield 3 archive."));
    
    //get some basic info on where stuff is in file
    _stream.ignore(37);
    tocaddress = _stream.getU32LE();
    _stream.ignore(4);
    dircount = _stream.getU16LE();
    LOG_DEBUG("ISH dir count %d", dircount);
    
    //find the toc and work out how many files we have in the archive
    _stream.seekg(tocaddress, std::ios_base::beg);
    
    //Get offsets to files tables from the directory table
    std::vector<uint32_t> dirfiles;
    
    for(uint32_t i = 0; i < dircount; i++) {
        dirfiles.push_back(handleIszDirs());
    }
    
    //parse the file entries in the toc to get filenames, size and location
    for(uint32_t i = 0; i < dirfiles.size(); i++){
            handleIszFiles(archive, dirfiles[i]);
    }
    
    _stream.close();
    
    return _archives.size() - 1;;
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
        entry.second.size = entry.second.decmpsize = _stream.getU32LE();
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
        entry.second.decmpsize = entry.second.size;
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
            if(i > size) break;
        }
        _stream.ignore(1);
        entry.second.start = (_stream.getU32LE() * ISO_BLKSIZE) + archive.start;
        _stream.ignore(4);
        entry.second.size = entry.second.decmpsize = _stream.getU32LE();
        _stream.ignore(11);
        //check if flags indicate this is a dir entry, handled by caller so skip
        //also check if file is hidden, we don't care about it if so
        uint8_t flag = _stream.get();
        if(flag & 0x02 || flag & 0x01) {
            //LOG_DEBUG("record is for a dir or hidden");
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

uint32_t ArchiveManager::handleIszDirs()
{
    uint16_t fcount = _stream.getU16LE();
    uint16_t chksize = _stream.getU16LE();
    uint16_t nlen = _stream.getU16LE();
    
    LOG_DEBUG("We have %d files", fcount);

    //skip the name of the dir, we just want the files
    _stream.ignore(nlen);
    
    //skip to end of chunk
    _stream.ignore(chksize - nlen - 6);

    return fcount;
}

void ArchiveManager::handleIszFiles(ArcFileInfo& archive, unsigned int size)
{
    t_arc_entry entry;
    std::pair<t_arc_index_iter,bool> rv;
    uint16_t chksize;
    uint8_t namelen;
    unsigned dataoffset = ISZ_DATASTART + archive.start;
    
    //add new archive to the archive list
    _archives.push_back(t_arc_index());
    
    for(unsigned int i = 0; i < size; i++) {
        //get entry info and skip stuff we don't need
        _stream.ignore(3);
        entry.second.decmpsize = _stream.getU32LE();
        entry.second.size = _stream.getU32LE();
        //second 4 bytes here are date/time stamp, not needed for this
        _stream.ignore(12);
        //_stream.read(reinterpret_cast<char*>(&file.second.datetime) + 2, sizeof(uint16_t));
        //_stream.read(reinterpret_cast<char*>(&file.second.datetime), sizeof(uint16_t));
        chksize = _stream.getU16LE();
        _stream.ignore(4);
        namelen = _stream.get();
        std::string fname;
        
        //read in file name, ensure null termination;
        fname.resize(namelen);
        _stream.read(&fname.at(0), namelen);
        entry.first = idGen(fname);

        //complete out file entry with the offset within the body.
        entry.second.start = dataoffset;
        //update offset for the following file
        dataoffset += entry.second.size;
        
        entry.second.type = ARC_ISH;
        entry.second.archivepath = archive.archivepath;
        
        rv = _archives.back().insert(entry);
        
        //if insertion failed, assume bad format.
        if(!rv.second) {
            LOG_WARNING("Could not index %s, likely a hash collision", fname.c_str());
        }

        //skip to end of chunk
        _stream.ignore(chksize - namelen - 30);
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
        if(info != it->end()) {
            LOG_DEBUG("Entry is at %u, sized %u, in archive %s", info->second.start, info->second.size, info->second.archivepath.c_str());
            return info->second;
        }
    }
    
    LOG_DEBUG("Couldn't find file of id %08x", id);
    return BLANK;
}

}//eastwood