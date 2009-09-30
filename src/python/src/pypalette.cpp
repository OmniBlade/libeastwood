#include <istream>
#include <fstream>
#include <sstream>
#include "pyeastwood.h"
#include <structmember.h>

#include "eastwood/StdDef.h"
#include "eastwood/Palette.h"

#include "pypalette.h"

using namespace eastwood;

static PyObject *
Palette_new(PyTypeObject *type, PyObject *args, __attribute__((unused)) PyObject *kwargs)
{
    Py_Palette *self = NULL;
    self = (Py_Palette *)type->tp_alloc(type, 0);
    if (self != NULL) {
	self->palette = reinterpret_cast<Palette*>(args);
	self->size = self->palette->size();
    }

    return (PyObject *)self;
}

static void
Palette_dealloc(Py_Palette *self)
{
    delete self->palette;
    PyObject_Del((PyObject*)self);
}

static PyMemberDef Palette_members[] = {
    {const_cast<char*>("size"), T_USHORT, offsetof(Py_Palette, size), RO, NULL},
    {NULL, 0, 0, 0, NULL}
};

PyTypeObject Palette_Type = {
    PyObject_HEAD_INIT(NULL)
    0,						/*ob_size*/
    "pyeastwood.Palette",			/*tp_name*/
    sizeof(Py_Palette),				/*tp_basicsize*/
    0,						/*tp_itemsize*/
    (destructor)Palette_dealloc,		/*tp_dealloc*/
    0,						/*tp_print*/
    0,						/*tp_getattr*/
    0,						/*tp_setattr*/
    0,						/*tp_compare*/
    0,						/*tp_repr*/
    0,						/*tp_as_number*/
    0,						/*tp_as_sequence*/
    0,						/*tp_as_mapping*/
    0,						/*tp_hash*/
    0,						/*tp_call*/
    0,						/*tp_str*/
    PyObject_GenericGetAttr,			/*tp_getattro*/
    PyObject_GenericSetAttr,			/*tp_setattro*/
    0,						/*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE,	/*tp_flags*/
    0,						/*tp_doc*/
    0,						/*tp_traverse*/
    0,						/*tp_clear*/
    0,						/*tp_richcompare*/
    0,						/*tp_weaklistoffset*/
    0,						/*tp_iter*/
    0,						/*tp_iternext*/
    0,						/*tp_methods*/
    Palette_members,				/*tp_members*/
    0,						/*tp_getset*/
    0,                      			/*tp_base*/
    0,                      			/*tp_dict*/
    0,                      			/*tp_descr_get*/
    0,                      			/*tp_descr_set*/
    0,                      			/*tp_dictoffset*/
    0,						/*tp_init*/
    PyType_GenericAlloc,    			/*tp_alloc*/
    Palette_new,	      			/*tp_new*/
    0,		          			/*tp_free*/
    0,                      			/*tp_is_gc*/
    0,						/*tp_bases*/
    0,						/*tp_mro*/
    0,						/*tp_cache*/
    0,						/*tp_subclasses*/
    0,						/*tp_weaklist*/
    0,						/*tp_del*/
    0						/*tp_version_tag*/
};