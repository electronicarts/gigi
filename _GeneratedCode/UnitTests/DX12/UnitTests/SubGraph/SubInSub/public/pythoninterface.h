#pragma once

#include "technique.h"

namespace SubInSub
{
    inline PyObject* Set_Inner1_Inner1Mult(PyObject* self, PyObject* args)
    {
        int contextIndex;
        float4 value;

        if (!PyArg_ParseTuple(args, "iffff:Set_Inner1_Inner1Mult", &contextIndex, &value[0], &value[1], &value[2], &value[3]))
            return PyErr_Format(PyExc_TypeError, "type error");

        Context* context = Context::GetContext(contextIndex);
        if (!context)
            return PyErr_Format(PyExc_IndexError, __FUNCTION__, "() : index % i is out of range(count = % i)", contextIndex, Context::GetContextCount());

        context->m_input.variable_Inner1_Inner1Mult = value;

        Py_INCREF(Py_None);
        return Py_None;
    }

    static PyMethodDef pythonModuleMethods[] = {
        {"Set_Inner1_Inner1Mult", Set_Inner1_Inner1Mult, METH_VARARGS, ""},
        {nullptr, nullptr, 0, nullptr}
    };

    static PyModuleDef pythonModule = {
        PyModuleDef_HEAD_INIT, "SubInSub", NULL, -1, pythonModuleMethods,
        NULL, NULL, NULL, NULL
    };

    PyObject* CreateModule()
    {
        PyObject* module = PyModule_Create(&pythonModule);
        return module;
    }
};
