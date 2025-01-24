///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#include "ViewerPython.h"

#define PY_SSIZE_T_CLEAN
#ifdef __unix
  #ifdef _DEBUG
    #undef _DEBUG
    #include <Python.h>
    #include <tupleobject.h>
    #define _DEBUG
  #else
    #include <Python.h>
    #include <tupleobject.h>
  #endif
#else
  #ifdef _DEBUG
    #undef _DEBUG
    #include <python.h>
    #include <tupleobject.h>
    #define _DEBUG 1
  #else
    #include <python.h>
    #include <tupleobject.h>
  #endif
#endif

// Portions of this software were based on https://gist.github.com/lukicdarkoo/40684dbffe234796376dc001d2fc4cfb

struct GigiArrayWrapper
{
    PyObject_HEAD
    GigiArray* arr;
};

static int GigiArray_getbuffer(PyObject * exporter, Py_buffer *view, int flags)
{
    if (view == nullptr)
    {
        PyErr_SetString(PyExc_BufferError, "NULL view in getbuffer");
        return -1;
    }

    view->obj = exporter;
    view->readonly = 1;
    view->suboffsets = nullptr;
    view->internal = nullptr;

    const GigiArrayWrapper& arrWrapper = *(GigiArrayWrapper*)exporter;
    if (arrWrapper.arr)
    {
        const GigiArray& arr = *arrWrapper.arr;
        view->buf = (void*)arr.data.data();
        view->len = arr.data.size();
        view->ndim = (int)arr.dims.size();
        view->shape = (int64_t*)arr.dims.data();
        view->strides = (int64_t*)arr.strides.data();
        view->format = (char*)arr.formatString.c_str();
        view->itemsize = arr.itemSize;
    }
    else
    {
        view->buf = nullptr;
        view->len = 0;
        view->ndim = 1;
        view->shape = nullptr;
        view->strides = nullptr;
        view->format = nullptr;
        view->itemsize = sizeof(char);
    }

    return 0;
}

static PyBufferProcs GigiArray_as_buffer = {
    (getbufferproc)GigiArray_getbuffer,
    (releasebufferproc)0,
};


void GigiArray_dealloc(PyObject* obj)
{
    GigiArrayWrapper* self = (GigiArrayWrapper*)obj;
    if (self->arr)
    {
        delete self->arr;
        self->arr = nullptr;
    }
}

static PyTypeObject GigiArrayType = {
    .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "GigiArray.GigiArray",
    .tp_basicsize = sizeof(GigiArrayWrapper),
    .tp_dealloc = GigiArray_dealloc,
    .tp_as_buffer = &GigiArray_as_buffer,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_doc = PyDoc_STR("GigiArray object"),
};

static PyModuleDef GigiArray_module =
{
    PyModuleDef_HEAD_INIT, "GigiArray", NULL, -1, NULL,
    NULL, NULL, NULL, NULL
};

void PyInit_GigiArray()
{
    PyImport_AppendInittab("GigiArray",
        []() -> PyObject*
        {
            PyObject* module = PyModule_Create(&GigiArray_module);

            GigiArrayType.tp_new = PyType_GenericNew;
            if (PyType_Ready(&GigiArrayType) < 0)
                return nullptr;
            PyModule_AddObject(module, "array", (PyObject*)&GigiArrayType);

            return module;
        }
    );
}

PyObject* PyCreate_GigiArray(const GigiArray& array)
{
    GigiArrayWrapper* ret = PyObject_New(GigiArrayWrapper, &GigiArrayType);
    ret->arr = new GigiArray;
    ret->arr[0] = array;
    return (PyObject*)ret;
}
