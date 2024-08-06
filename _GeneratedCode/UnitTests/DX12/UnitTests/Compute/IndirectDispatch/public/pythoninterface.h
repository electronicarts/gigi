#pragma once

#include "technique.h"

namespace IndirectDispatch
{
    inline PyObject* Set_Dispatch_Count_1(PyObject* self, PyObject* args)
    {
        int contextIndex;
        uint3 value;

        if (!PyArg_ParseTuple(args, "iIII:Set_Dispatch_Count_1", &contextIndex, &value[0], &value[1], &value[2]))
            return PyErr_Format(PyExc_TypeError, "type error");

        Context* context = Context::GetContext(contextIndex);
        if (!context)
            return PyErr_Format(PyExc_IndexError, __FUNCTION__, "() : index % i is out of range(count = % i)", contextIndex, Context::GetContextCount());

        context->m_input.variable_Dispatch_Count_1 = value;

        Py_INCREF(Py_None);
        return Py_None;
    }

    inline PyObject* Set_Dispatch_Count_2(PyObject* self, PyObject* args)
    {
        int contextIndex;
        uint3 value;

        if (!PyArg_ParseTuple(args, "iIII:Set_Dispatch_Count_2", &contextIndex, &value[0], &value[1], &value[2]))
            return PyErr_Format(PyExc_TypeError, "type error");

        Context* context = Context::GetContext(contextIndex);
        if (!context)
            return PyErr_Format(PyExc_IndexError, __FUNCTION__, "() : index % i is out of range(count = % i)", contextIndex, Context::GetContextCount());

        context->m_input.variable_Dispatch_Count_2 = value;

        Py_INCREF(Py_None);
        return Py_None;
    }

    inline PyObject* Set_Second_Dispatch_Offset(PyObject* self, PyObject* args)
    {
        int contextIndex;
        int value;

        if (!PyArg_ParseTuple(args, "ii:Set_Second_Dispatch_Offset", &contextIndex, &value))
            return PyErr_Format(PyExc_TypeError, "type error");

        Context* context = Context::GetContext(contextIndex);
        if (!context)
            return PyErr_Format(PyExc_IndexError, __FUNCTION__, "() : index % i is out of range(count = % i)", contextIndex, Context::GetContextCount());

        context->m_input.variable_Second_Dispatch_Offset = value;

        Py_INCREF(Py_None);
        return Py_None;
    }

    static PyMethodDef pythonModuleMethods[] = {
        {"Set_Dispatch_Count_1", Set_Dispatch_Count_1, METH_VARARGS, ""},
        {"Set_Dispatch_Count_2", Set_Dispatch_Count_2, METH_VARARGS, ""},
        {"Set_Second_Dispatch_Offset", Set_Second_Dispatch_Offset, METH_VARARGS, ""},
        {nullptr, nullptr, 0, nullptr}
    };

    static PyModuleDef pythonModule = {
        PyModuleDef_HEAD_INIT, "IndirectDispatch", NULL, -1, pythonModuleMethods,
        NULL, NULL, NULL, NULL
    };

    PyObject* CreateModule()
    {
        PyObject* module = PyModule_Create(&pythonModule);
        return module;
    }
};
