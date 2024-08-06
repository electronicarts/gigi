#pragma once

#include "technique.h"

namespace MeshAmplificationLines
{
    inline PyObject* Set_AmplificationCount(PyObject* self, PyObject* args)
    {
        int contextIndex;
        uint value;

        if (!PyArg_ParseTuple(args, "iI:Set_AmplificationCount", &contextIndex, &value))
            return PyErr_Format(PyExc_TypeError, "type error");

        Context* context = Context::GetContext(contextIndex);
        if (!context)
            return PyErr_Format(PyExc_IndexError, __FUNCTION__, "() : index % i is out of range(count = % i)", contextIndex, Context::GetContextCount());

        context->m_input.variable_AmplificationCount = value;

        Py_INCREF(Py_None);
        return Py_None;
    }

    inline PyObject* Set_AmplificationSpacing(PyObject* self, PyObject* args)
    {
        int contextIndex;
        float value;

        if (!PyArg_ParseTuple(args, "if:Set_AmplificationSpacing", &contextIndex, &value))
            return PyErr_Format(PyExc_TypeError, "type error");

        Context* context = Context::GetContext(contextIndex);
        if (!context)
            return PyErr_Format(PyExc_IndexError, __FUNCTION__, "() : index % i is out of range(count = % i)", contextIndex, Context::GetContextCount());

        context->m_input.variable_AmplificationSpacing = value;

        Py_INCREF(Py_None);
        return Py_None;
    }

    inline PyObject* Set_Offset(PyObject* self, PyObject* args)
    {
        int contextIndex;
        float3 value;

        if (!PyArg_ParseTuple(args, "ifff:Set_Offset", &contextIndex, &value[0], &value[1], &value[2]))
            return PyErr_Format(PyExc_TypeError, "type error");

        Context* context = Context::GetContext(contextIndex);
        if (!context)
            return PyErr_Format(PyExc_IndexError, __FUNCTION__, "() : index % i is out of range(count = % i)", contextIndex, Context::GetContextCount());

        context->m_input.variable_Offset = value;

        Py_INCREF(Py_None);
        return Py_None;
    }

    static PyMethodDef pythonModuleMethods[] = {
        {"Set_AmplificationCount", Set_AmplificationCount, METH_VARARGS, ""},
        {"Set_AmplificationSpacing", Set_AmplificationSpacing, METH_VARARGS, ""},
        {"Set_Offset", Set_Offset, METH_VARARGS, ""},
        {nullptr, nullptr, 0, nullptr}
    };

    static PyModuleDef pythonModule = {
        PyModuleDef_HEAD_INIT, "MeshAmplificationLines", NULL, -1, pythonModuleMethods,
        NULL, NULL, NULL, NULL
    };

    PyObject* CreateModule()
    {
        PyObject* module = PyModule_Create(&pythonModule);
        return module;
    }
};
