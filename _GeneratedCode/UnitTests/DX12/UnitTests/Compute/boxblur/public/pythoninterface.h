#pragma once

#include "technique.h"

namespace boxblur
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

    inline PyObject* Set_radius(PyObject* self, PyObject* args)
    {
        int contextIndex;
        int value;

        if (!PyArg_ParseTuple(args, "ii:Set_radius", &contextIndex, &value))
            return PyErr_Format(PyExc_TypeError, "type error");

        Context* context = Context::GetContext(contextIndex);
        if (!context)
            return PyErr_Format(PyExc_IndexError, __FUNCTION__, "() : index % i is out of range(count = % i)", contextIndex, Context::GetContextCount());

        context->m_input.variable_radius = value;

        Py_INCREF(Py_None);
        return Py_None;
    }

    inline PyObject* Set_sRGB(PyObject* self, PyObject* args)
    {
        int contextIndex;
        bool value;

        if (!PyArg_ParseTuple(args, "ib:Set_sRGB", &contextIndex, &value))
            return PyErr_Format(PyExc_TypeError, "type error");

        Context* context = Context::GetContext(contextIndex);
        if (!context)
            return PyErr_Format(PyExc_IndexError, __FUNCTION__, "() : index % i is out of range(count = % i)", contextIndex, Context::GetContextCount());

        context->m_input.variable_sRGB = value;

        Py_INCREF(Py_None);
        return Py_None;
    }

    static PyMethodDef pythonModuleMethods[] = {
        {"Set_enabled", Set_enabled, METH_VARARGS, "Enables or disables the blurring effect."},
        {"Set_radius", Set_radius, METH_VARARGS, "The radius of the blur.  Actual size in pixles of the blur is (radius*2+1)^2"},
        {"Set_sRGB", Set_sRGB, METH_VARARGS, ""},
        {nullptr, nullptr, 0, nullptr}
    };

    static PyModuleDef pythonModule = {
        PyModuleDef_HEAD_INIT, "boxblur", NULL, -1, pythonModuleMethods,
        NULL, NULL, NULL, NULL
    };

    PyObject* CreateModule()
    {
        PyObject* module = PyModule_Create(&pythonModule);
        return module;
    }
};
