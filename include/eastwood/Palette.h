#ifndef EASTWOOD_PALETTE_H
#define EASTWOOD_PALETTE_H

#include <vector>
#include "eastwood/StdDef.h"

namespace eastwood {

struct Color {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t unused;
};

class Palette {
public:
    Palette(uint16_t colors = 256);
    virtual ~Palette();

    Color& operator[] (uint16_t i) { return _palette.at(i); }
    virtual operator bool() const throw() {
        return !_palette.empty();
    }

    virtual bool operator! () const throw() {
        return _palette.empty();
    }

    uint16_t size() const throw() { return _palette.size(); }
    uint8_t nearest(Color a);
    bool savePAL(std::ostream &output, bool fullpal = false);

protected:
    double colorDistance(Color cola, Color colb);
    std::vector<Color> _palette;
};

}
#endif // EASTWOOD_PALETTE_H
