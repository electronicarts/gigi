#pragma once

#include "technique.h"

namespace Mips_DrawCall
{
    inline PyObject* Set_UseMips(PyObject* self, PyObject* args)
    {
        int contextIndex;
        bool value;

        if (!PyArg_ParseTuple(args, "ib:Set_UseMips", &contextIndex, &value))
            return PyErr_Format(PyExc_TypeError, "type error");

        Context* context = Context::GetContext(contextIndex);
        if (!context)
            return PyErr_Format(PyExc_IndexError, __FUNCTION__, "() : index % i is out of range(count = % i)", contextIndex, Context::GetContextCount());

        context->m_input.variable_UseMips = value;

        Py_INCREF(Py_None);
        return Py_None;
    }

    inline PyObject* Set_RenderSize(PyObject* self, PyObject* args)
    {
        int contextIndex;
        uint2 value;

        if (!PyArg_ParseTuple(args, "iII:Set_RenderSize", &contextIndex, &value[0], &value[1]))
            return PyErr_Format(PyExc_TypeError, "type error");

        Context* context = Context::GetContext(contextIndex);
        if (!context)
            return PyErr_Format(PyExc_IndexError, __FUNCTION__, "() : index % i is out of range(count = % i)", contextIndex, Context::GetContextCount());

        context->m_input.variable_RenderSize = value;

        Py_INCREF(Py_None);
        return Py_None;
    }

    static PyMethodDef pythonModuleMethods[] = {
        {"Set_UseMips", Set_UseMips, METH_VARARGS, ""},
        {"Set_RenderSize", Set_RenderSize, METH_VARARGS, ""},
        {nullptr, nullptr, 0, nullptr}
    };

    static PyModuleDef pythonModule = {
        PyModuleDef_HEAD_INIT, "Mips_DrawCall", NULL, -1, pythonModuleMethods,
        NULL, NULL, NULL, NULL
    };

    PyObject* CreateModule()
    {
        PyObject* module = PyModule_Create(&pythonModule);
        return module;
    }
};
