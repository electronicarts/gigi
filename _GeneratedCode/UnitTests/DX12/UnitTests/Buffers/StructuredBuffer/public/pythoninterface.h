#pragma once

#include "technique.h"

namespace StructuredBuffer
{
    inline PyObject* TrinaryToString(PyObject* self, PyObject* args)
    {
        int value;
        if (!PyArg_ParseTuple(args, "i:TrinaryToString", &value))
            return PyErr_Format(PyExc_TypeError, "type error");

        switch((Trinary)value)
        {
            case Trinary::True: return Py_BuildValue("s", "True");
            case Trinary::False: return Py_BuildValue("s", "False");
            case Trinary::Maybe: return Py_BuildValue("s", "Maybe");
            default: return Py_BuildValue("s", "<invalid Trinary value>");
        }
    }

    static PyMethodDef pythonModuleMethods[] = {
        {"TrinaryToString", TrinaryToString, METH_VARARGS, ""},
        {nullptr, nullptr, 0, nullptr}
    };

    static PyModuleDef pythonModule = {
        PyModuleDef_HEAD_INIT, "StructuredBuffer", NULL, -1, pythonModuleMethods,
        NULL, NULL, NULL, NULL
    };

    PyObject* CreateModule()
    {
        PyObject* module = PyModule_Create(&pythonModule);
        PyModule_AddIntConstant(module, "Trinary_True", 0);
        PyModule_AddIntConstant(module, "Trinary_False", 1);
        PyModule_AddIntConstant(module, "Trinary_Maybe", 2);
        return module;
    }
};
