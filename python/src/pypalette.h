#ifndef EASTWOOD_PYTHON_PYPALETTE_H
#define EASTWOOD_PYTHON_PYPALETTE_H

#include "eastwood/Palette.h"

struct Py_Palette {
    PyObject_HEAD
    eastwood::Palette *palette;
    PyObject *tuple;
};

extern PyTypeObject Palette_Type;

#endif // EASTWOOD_PYTHON_PYPALETTE_H
