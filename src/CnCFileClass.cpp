#include "eastwood/CnCFileClass.h"
#include "eastwood/DirEnt.h"
#include "eastwood/Log.h"
#include "eastwood/BlowFish.h"
#include "eastwood/WsKey.h"
#include <algorithm>

namespace eastwood {

const uint32_t enc_index = 0x00020000;
static std::string m_datadir;
static std::vector<WWFileInfoLinkedElem*> m_index;

CCFileClass::CCFileClass(const std::string& datadir):
m_open()
{
    m_datadir = datadir;
}

CCFileClass::~CCFileClass()
{
    close();
}

bool CCFileClass::addFileInfo(const std::string& fname, uint32_t fsize, uint32_t index)
{
    WWFileInfoLinkedElem* linkp;
    int32_t id;
    
    id = idGen(fname);
    
    linkp = new WWFileInfoLinkedElem;
    if (linkp == NULL) {
        //TODO error and warning
        LOG_ERROR("Couldn't allocate new object");
        return false;
    }
    
    if(m_index.size() <= index) m_index.push_back(NULL);
    
    linkp->info.filename = fname;
    linkp->info.fileSize = fsize;
    linkp->info.filePosition = 0;
    linkp->id = id;
    linkp->cd = -1;
    linkp->next = m_index[index];
    m_index[index] = linkp;
    LOG_DEBUG("Added %s with id of %08x", linkp->info.filename.c_str(), linkp->id);
    return true;
}

inline uint32_t CCFileClass::getIndex()
{
    if(m_index.capacity() < MAX_ARCHIVES) {
        m_index.reserve(MAX_ARCHIVES + 1);
    }
    
    for(uint32_t i = 0; i < MAX_ARCHIVES; i++){
        if(m_index[i] == NULL){
            return i;
        }
    }
    
    return MAX_ARCHIVES;
}

bool CCFileClass::init()
{
    init(m_datadir);
}

//Initialise a directory
bool CCFileClass::init(const std::string& datadir)
{
    struct dirent* dir;
    DIR *dp = opendir(datadir.c_str());
    struct stat st;
    bool infop;
    uint32_t index;
    
    //find first unused index in vector
    index = getIndex();
    LOG_DEBUG("Got a free index of %d", index);
    
    if(index == MAX_ARCHIVES){
        LOG_ERROR("Archive limit reached");
        return false;
    }
    
    if (dp == NULL) {
        LOG_ERROR("Couldn't open directory %s", datadir.c_str());
        return false;
    }
    
    while ((dir = readdir(dp)) != NULL) {
        std::string path;
        
        //ignore standard dirs
        if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0)
            continue;
        
        path = datadir + dir->d_name;
        if (stat(path.c_str(), &st) < 0) {
            //TODO error and warnings
            LOG_WARNING("Couldn't stat %s", path.c_str());
            continue;
        }
        
        //ignore if a directory of any kind, sub dirs not supported
        if (S_ISDIR(st.st_mode)) continue;
        
        if(!(infop = addFileInfo(dir->d_name, st.st_size, index))){
            //TODO error and warnings
            LOG_WARNING("FileInfo");
            continue;
        }
    }
    
    closedir(dp);
    
    return true;
}

bool CCFileClass::createFile(std::string fname)
{
    m_open.fp = fopen((m_datadir + DIR_SEP + fname).c_str(), "wb");
    if(m_open.fp != NULL){
        return addFileInfo(fname, 0, 0);
    } else {
        return false;
    }
}

bool CCFileClass::retrieve(std::string mixfile){
    int8_t buf[10];      //hold part of header to work out format
    uint16_t count;     //number of files in archive
    bool enc;
    WWFileInfo* mixfp;
    WWFileInfo* fileinfo;
    int32_t fid;
    
    //open the mix we want to retrieve the index for
    //Need this rather than just open as we need to know if mix is within a mix
    fid = idGen(mixfile);
    LOG_DEBUG("Retrieving %s with id %08x", mixfile.c_str(), fid);
    fileinfo = finder(fid, &mixfp);
    
    if(fileinfo == NULL) {
        LOG_WARNING("Couldn't find file %s", mixfile.c_str());
        return false;
    }
    
    if(mixfp == NULL){
        m_open.fp = fopen((m_datadir + DIR_SEP + fileinfo->filename).c_str(), "rb");
        LOG_DEBUG("opening %s ", fileinfo->filename.c_str());
        if(m_open.fp == NULL) {
            LOG_ERROR("Couldn't open file %s", fileinfo->filename.c_str());
            return false;
        }
        
        m_open.position = 0;
        m_open.size = fileinfo->fileSize;
        m_open.start = 0;
        mixfp = fileinfo;
    } else {
        m_open.fp = fopen((m_datadir + DIR_SEP + mixfp->filename).c_str(), "rb");
        LOG_DEBUG("opening %s ", mixfp->filename.c_str());
        if(m_open.fp == NULL) {
            LOG_ERROR("Couldn't open file %s", fileinfo->filename.c_str());
            return false;
        }
        
        m_open.position = 0;
        m_open.size = fileinfo->fileSize;
        m_open.start = fileinfo->filePosition;
        
        //seek to where our file starts in the archive
        fseek(m_open.fp, m_open.start, SEEK_SET);
    }
    
    //Work out what kind of file we have
    read(buf,10);
    
    count = *reinterpret_cast<uint16_t*>(buf);
    enc = *reinterpret_cast<int32_t*>(buf) & enc_index;
    
    LOG_DEBUG("Mix is encrypted? %d", enc);
    
    if(count || !enc){ 
        //we have unencrypted
        bool rv = indexPlain(buf, mixfp);
        close();
        return rv;
    } else {
        //TODO we have encrypted
        bool rv = indexCrypt(mixfp);
        close();
        return rv;
    }
    
    //return true;
}

bool CCFileClass::indexPlain(int8_t* head, WWFileInfo* mixfp)
{
    //int8_t* buf = head;
    uint32_t offset = 6 + m_open.start;     //at least 6 at this point
    uint32_t fcount = *reinterpret_cast<uint16_t*>(head);
    uint32_t index;
    
    if(fcount){
        seek(6, SEEK_SET);
        LOG_DEBUG("Indexing TD format mix");
    } else {
        fcount = *reinterpret_cast<uint16_t*>(head + 4);
        offset += 4;
        seek(10, SEEK_SET);
        LOG_DEBUG("Indexing RA format mix");
    }
    
    //add total size of index to offset
    offset += 12 * fcount;
    
    //find out where we are putting the index
    index = getIndex();
    if(m_index.size() <= index) m_index.push_back(NULL);
    
    for(uint32_t i = 0; i < fcount; i++){
        WWFileInfoLinkedElem* linkp;

        linkp = new WWFileInfoLinkedElem;
        if (linkp == NULL) {
            LOG_ERROR("Couldn't allocate new object");
            cleanUp(index);
            return false;
        }
        
        linkp->id = readle32();
        linkp->info.filePosition = readle32() + offset;
        linkp->info.filename = "";      //we don't know filename from id
        linkp->info.fileSize = readle32();
        linkp->info.inMIXFile = true;
        linkp->cd = -1;
        linkp->mix = mixfp;
        linkp->next = m_index[index];
        m_index[index] = linkp;
        LOG_DEBUG("Indexed %08x with mix file pointer to %s", 
                 m_index[index]->id, m_index[index]->mix->filename.c_str());
    }
    return true;
}

bool CCFileClass::indexCrypt(WWFileInfo* mixfp)
{
    uint32_t offset = 84 + m_open.start;     //at least 84 at this point
    uint32_t fcount;
    uint32_t index = getIndex();
    Cblowfish bfish;
    int8_t bfishbuf[8];
    uint8_t keysource[80];
    uint8_t key[56];
    uint32_t bcount;
    
    if(m_index.size() <= index) m_index.push_back(NULL);
    
    //setup blowfish to handle header
    seek(4, SEEK_SET);
    read(keysource, 80);
    get_blowfish_key(keysource, key);
    bfish.set_key(key, 56);
    
    //read first block to get file count, needed to calculate header size
    read(bfishbuf, 8);
    bfish.decipher(reinterpret_cast<void*>(bfishbuf), 
                    reinterpret_cast<void*>(bfishbuf), 8);
    memcpy(reinterpret_cast<char*>(&fcount), bfishbuf, 2);
    
    //caculate header size and prepare buffer for it
    bcount = ((fcount * 12) - 2) / 8;
    if (((fcount * 12) - 2) % 8) bcount++;
    //add 8 to compensate for block we already decrypted
    offset += bcount * 8 + 8;
    
    //prepare buffer for index
    char pindbuf[bcount * 8 + 2];
    memcpy(pindbuf, bfishbuf + 6 , 2);
    
    //loop to decrypt index into index buffer
    for(int i = 0; i < bcount; i++) {
        read(bfishbuf, 8);
        bfish.decipher(reinterpret_cast<void*>(bfishbuf), 
                    reinterpret_cast<void*>(bfishbuf), 8);
        memcpy(pindbuf + 2 + 8 * i, bfishbuf, 8);
    }
    
    for(uint32_t i = 0; i < fcount; i++){
        WWFileInfoLinkedElem* linkp;

        linkp = new WWFileInfoLinkedElem;
        if (linkp == NULL) {
            LOG_ERROR("Couldn't allocate new object");
            cleanUp(index);
            return false;
        }
        
        memcpy(reinterpret_cast<char*>(&linkp->id), pindbuf + i * 12,
               sizeof(int32_t));
        memcpy(reinterpret_cast<char*>(&linkp->info.filePosition), 
               pindbuf + 4 + i * 12, sizeof(int32_t));
        memcpy(reinterpret_cast<char*>(&linkp->info.fileSize), 
               pindbuf + 8 + i * 12, sizeof(int32_t));
        
        linkp->info.filePosition += offset;
        linkp->info.filename = "";      //we don't know filename from id
        linkp->cd = -1;
        linkp->mix = mixfp;
        linkp->next = m_index[index];
        m_index[index] = linkp;
        /*LOG_INFO("Indexed %08x from encrypted mix with mix file pointer to %s", 
                 m_index[index]->id, m_index[index]->mix->filename.c_str());*/
    }
    return true;
}

//deinit the whole index, should only call on closedown.
void CCFileClass::deinit()
{
    for(unsigned int i = 0; i < m_index.size(); i++){
        while(m_index[i]){
            WWFileInfoLinkedElem* e = m_index[i];
            m_index[i] = e->next;
            delete e;
        }
    }
}

void CCFileClass::cleanUp(uint32_t index)
{
    while(m_index[index]){
        WWFileInfoLinkedElem* e = m_index[index];
        m_index[index] = e->next;
        delete e;
    }
}

//Generates the ID's found in mix files.
int32_t CCFileClass::idGen(std::string fname)
{
    std::transform(fname.begin(), fname.end(), fname.begin(),
            (int(*)(int)) toupper); // convert to uppercase
    int i = 0;
    uint32_t id = 0;
    int l = fname.length(); // length of the filename
    while (i < l) {
        uint32_t a = 0;
        for (int j = 0; j < 4; j++) {
            a >>= 8;
            if (i < l)
                a += static_cast<uint32_t> (fname[i]) << 24;
            i++;
        }
        id = (id << 1 | id >> 31) + a;
    }
    return id;
}

bool CCFileClass::open(const std::string& fname, uint32_t mode)
{
    std::string file_path;
    std::string mode_str;
    WWFileInfo* mixfile;
    WWFileInfo* fileinfo;
    int32_t fid;
    
    if ((mode & FILE_MODE_READ_WRITE) == 0) return false;
    
    mode_str = (mode == FILE_MODE_WRITE) ? "wb" : ((mode == FILE_MODE_READ_WRITE) ? "wb+" : "rb");
    
    fid = idGen(fname);
    
    fileinfo = finder(fid, &mixfile);
    if(fileinfo == NULL && !(mode & FILE_MODE_WRITE)) {
        LOG_WARNING("Couldn't find file %s", fname.c_str());
        return false;
    } else if(fileinfo == NULL && mode & FILE_MODE_WRITE) {
        return createFile(fname);
    }
    
    if(mixfile == NULL){
        m_open.fp = fopen((m_datadir + DIR_SEP + fileinfo->filename).c_str(), mode_str.c_str());
        if(m_open.fp == NULL) {
            LOG_ERROR("Failed to open file %s", fileinfo->filename.c_str());
            return false;
        }
        
        m_open.position = 0;
        m_open.size = fileinfo->fileSize;
        m_open.start = 0;
    } else {
        if(mode & FILE_MODE_WRITE) {
            LOG_WARNING("Cannot open file within a MixFile for writing.");
            return false;
        }
        m_open.fp = fopen((m_datadir + DIR_SEP + mixfile->filename).c_str(), "rb");
        if(m_open.fp == NULL) {
            LOG_ERROR("Failed to open file %s", fileinfo->filename.c_str());
            return false;
        }
        
        m_open.position = 0;
        m_open.size = fileinfo->fileSize;
        m_open.start = fileinfo->filePosition;
        
        //seek to where our file starts in the archive
        fseek(m_open.fp, m_open.start, SEEK_SET);
    }
 
    return true;
    
}

void CCFileClass::close()
{
    if (m_open.fp == NULL) return;

    fclose(m_open.fp);
    m_open.fp = NULL;
    m_open.size = 0;
}

WWFileInfo* CCFileClass::finder(int32_t id, WWFileInfo** mixinfo)
{
    WWFileInfoLinkedElem* info;
    if(*mixinfo) *mixinfo = NULL;

    for(unsigned int i = 0; i < m_index.size(); i++){
        info = m_index[i];
        LOG_DEBUG("Searching index %d", i);
        if(info == NULL) {
            LOG_WARNING("Index %d was NULL", i);
            break;
        }
        
        while(info){
            LOG_DEBUG("Comparing %08x to indexed id %08x", id, info->id);
            if(info->id == id){
                if(info->mix) *mixinfo = info->mix;
                return &info->info;
            }
            info = info->next;
        }
    }
    return NULL;
}

uint32_t CCFileClass::read(void* buf, uint32_t length)
{
    if(m_open.fp == NULL) return 0;
    if(m_open.position > m_open.size) return 0;
    if(length == 0) return 0;
    
    if(length > m_open.size - m_open.position){
        length = m_open.size - m_open.position;
    }
    
    if (fread(buf, length, 1, m_open.fp) != 1) {
        LOG_ERROR("Read error\n");
        close();

        length = 0;
    }

    m_open.position += length;
    return length;
}

uint32_t CCFileClass::readle32()
{
    uint8_t buffer[4];
    read(buffer, sizeof(buffer));
    return le32toh(*reinterpret_cast<uint32_t*>(buffer));
}

uint16_t CCFileClass::readle16()
{
    uint8_t buffer[2];
    read(buffer, sizeof(buffer));
    return le16toh(*reinterpret_cast<uint16_t*>(buffer));
}

uint8_t CCFileClass::read8()
{
    uint8_t buffer[1];
    read(buffer, sizeof(buffer));
    return buffer[0];
}

uint32_t CCFileClass::write(const void *buffer, uint32_t length)
{
    if(m_open.fp == NULL) return 0;
    if(length == 0) return 0;
    
    if (fwrite(buffer, length, 1, m_open.fp) != 1) {
        LOG_ERROR("Write error\n");
        close();

        length = 0;
    }
    
    m_open.position += length;
    
    if (m_open.position > m_open.size) m_open.size = m_open.position;
    
    return length;
}

bool CCFileClass::writele16(uint16_t value)
{
    htole16(value);
    return (write(&value, 2) == 2);
}

bool CCFileClass::writele32(uint32_t value)
{
    htole32(value);
    return (write(&value, 4) == 4);
}

bool CCFileClass::write8(uint8_t value)
{
    return(write(&value, 1) == 1);
}

uint32_t CCFileClass::seek(int32_t position, uint8_t mode)
{
    if (m_open.fp == NULL) return 0;
    if (mode > 2) { close(); return 0; }
    switch (mode) {
        case 0:
            fseek(m_open.fp, m_open.start + position, SEEK_SET);
            m_open.position = position;
            break;
        case 1:
            fseek(m_open.fp, position, SEEK_CUR);
            m_open.position += position;
            break;
        case 2:
            fseek(m_open.fp, m_open.start + m_open.size - position, SEEK_SET);
            m_open.position = m_open.size - position;
            break;
    }

    return m_open.position;
}

uint32_t CCFileClass::tell()
{
    return ftell(m_open.fp) - m_open.start;
}

} //eastwood