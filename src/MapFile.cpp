#include "eastwood/StdDef.h"

#include "eastwood/Exception.h"
#include "eastwood/MapFile.h"


namespace eastwood {

MapFile::MapFile(const std::istream &stream) :
    _stream(const_cast<IStream&>(reinterpret_cast<const IStream&>(stream))),    
    _tileSet(NULL)
{
    size_t mapSize = _stream.size();
    uint16_t numTileSets = _stream.getU16LE();

    _tileSet = new std::vector<std::vector<uint16_t> >(numTileSets);

    for(uint16_t i = 0, pre = numTileSets, cur = 0; i < numTileSets; i++, pre=cur) {
	uint16_t size = (cur= (i == numTileSets-1) ? mapSize/2 : _stream.getU16LE()) - pre;
	(*_tileSet)[i] = std::vector<uint16_t>(size);
    }

    for(std::vector<std::vector<uint16_t> >::iterator it = (*_tileSet).begin();
	    it != (*_tileSet).end(); it++) {
	for(std::vector<uint16_t>::iterator it2 = it->begin();
		it2 != it->end(); it2++)
	    *it2 = _stream.getU16LE();
    }
}

MapFile::~MapFile()
{
    //FIXME:
    //delete _tileSet;
}

}

