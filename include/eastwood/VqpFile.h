#ifndef EASTWOOD_VQPFILE_H
#define	EASTWOOD_VQPFILE_H

#include "Palette.h"

#include <istream>
#include <vector>

namespace eastwood {
    
class VqpFile {
public:
    VqpFile(Palette pal);
    VqpFile(std::istream& stream);
    ~VqpFile() {}
    
    uint8_t getAverage(uint8_t a, uint8_t b, int table = 0);
    uint32_t size() { return _lookuptable.size(); }
    
private:
    void createTable(Palette pal);
    inline Color getAverage(Color a, Color b);
    std::vector< std::vector<uint8_t> > _lookuptable;
    
};
    
} //eastwood

#endif	/* EASTWOOD_VQPFILE_H */

