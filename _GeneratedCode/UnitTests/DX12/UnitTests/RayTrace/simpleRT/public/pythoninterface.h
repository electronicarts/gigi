#pragma once

#include "technique.h"

namespace simpleRT
{
    inline PyObject* Set_enabled(PyObject* self, PyObject* args)
    {
        int contextIndex;
        bool value;

        if (!PyArg_ParseTuple(args, "ib:Set_enabled", &contextIndex, &value))
            return PyErr_Format(PyExc_TypeError, "type error");

        Context* context = Context::GetContext(contextIndex);
        if (!context)
            return PyErr_Format(PyExc_IndexError, __FUNCTION__, "() : index % i is out of range(count = % i)", contextIndex, Context::GetContextCount());

        context->m_input.variable_enabled = value;

        Py_INCREF(Py_None);
        return Py_None;
    }

    inline PyObject* Set_hitColor(PyObject* self, PyObject* args)
    {
        int contextIndex;
        float3 value;

        if (!PyArg_ParseTuple(args, "ifff:Set_hitColor", &contextIndex, &value[0], &value[1], &value[2]))
            return PyErr_Format(PyExc_TypeError, "type error");

        Context* context = Context::GetContext(contextIndex);
        if (!context)
            return PyErr_Format(PyExc_IndexError, __FUNCTION__, "() : index % i is out of range(count = % i)", contextIndex, Context::GetContextCount());

        context->m_input.variable_hitColor = value;

        Py_INCREF(Py_None);
        return Py_None;
    }

    inline PyObject* Set_missColor(PyObject* self, PyObject* args)
    {
        int contextIndex;
        float3 value;

        if (!PyArg_ParseTuple(args, "ifff:Set_missColor", &contextIndex, &value[0], &value[1], &value[2]))
            return PyErr_Format(PyExc_TypeError, "type error");

        Context* context = Context::GetContext(contextIndex);
        if (!context)
            return PyErr_Format(PyExc_IndexError, __FUNCTION__, "() : index % i is out of range(count = % i)", contextIndex, Context::GetContextCount());

        context->m_input.variable_missColor = value;

        Py_INCREF(Py_None);
        return Py_None;
    }

    static PyMethodDef pythonModuleMethods[] = {
        {"Set_enabled", Set_enabled, METH_VARARGS, ""},
        {"Set_hitColor", Set_hitColor, METH_VARARGS, ""},
        {"Set_missColor", Set_missColor, METH_VARARGS, ""},
        {nullptr, nullptr, 0, nullptr}
    };

    static PyModuleDef pythonModule = {
        PyModuleDef_HEAD_INIT, "simpleRT", NULL, -1, pythonModuleMethods,
        NULL, NULL, NULL, NULL
    };

    PyObject* CreateModule()
    {
        PyObject* module = PyModule_Create(&pythonModule);
        return module;
    }
};
