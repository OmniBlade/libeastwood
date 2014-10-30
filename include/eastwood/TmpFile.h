#ifndef EASTWOOD_TMPFILE_H
#define	EASTWOOD_TMPFILE_H

#include "BaseImageSequence.h"
#include <vector>
#include <istream>

namespace eastwood {
    
class TmpFile : public BaseImageSequence
{
public:
    TmpFile(std::istream& stream);
    ~TmpFile() {}
    
    virtual BaseImage& operator[] (uint16_t i);
    virtual uint16_t size() const throw() { return _imageindex.size(); }
    
    Surface getSurface(Palette pal);
    int16_t x() { return _x; }
    int16_t y() { return _y; }
private:
    std::vector<char> _unknown;
    std::vector<char> _terrainindex;
    std::vector<char> _imageindex;
    int16_t _x;
    int16_t _y;
    bool _raformat;
};
    
} //eastwood

#endif	/* EASTWOOD_TMPFILE_H */

