#pragma once

#include "technique.h"

namespace SlangAutoDiff
{
    inline PyObject* Set_NumGaussians(PyObject* self, PyObject* args)
    {
        int contextIndex;
        int value;

        if (!PyArg_ParseTuple(args, "ii:Set_NumGaussians", &contextIndex, &value))
            return PyErr_Format(PyExc_TypeError, "type error");

        Context* context = Context::GetContext(contextIndex);
        if (!context)
            return PyErr_Format(PyExc_IndexError, __FUNCTION__, "() : index % i is out of range(count = % i)", contextIndex, Context::GetContextCount());

        context->m_input.variable_NumGaussians = value;

        Py_INCREF(Py_None);
        return Py_None;
    }

    inline PyObject* Set_Reset(PyObject* self, PyObject* args)
    {
        int contextIndex;
        bool value;

        if (!PyArg_ParseTuple(args, "ib:Set_Reset", &contextIndex, &value))
            return PyErr_Format(PyExc_TypeError, "type error");

        Context* context = Context::GetContext(contextIndex);
        if (!context)
            return PyErr_Format(PyExc_IndexError, __FUNCTION__, "() : index % i is out of range(count = % i)", contextIndex, Context::GetContextCount());

        context->m_input.variable_Reset = value;

        Py_INCREF(Py_None);
        return Py_None;
    }

    inline PyObject* Set_LearningRate(PyObject* self, PyObject* args)
    {
        int contextIndex;
        float value;

        if (!PyArg_ParseTuple(args, "if:Set_LearningRate", &contextIndex, &value))
            return PyErr_Format(PyExc_TypeError, "type error");

        Context* context = Context::GetContext(contextIndex);
        if (!context)
            return PyErr_Format(PyExc_IndexError, __FUNCTION__, "() : index % i is out of range(count = % i)", contextIndex, Context::GetContextCount());

        context->m_input.variable_LearningRate = value;

        Py_INCREF(Py_None);
        return Py_None;
    }

    inline PyObject* Set_MaximumStepSize(PyObject* self, PyObject* args)
    {
        int contextIndex;
        float value;

        if (!PyArg_ParseTuple(args, "if:Set_MaximumStepSize", &contextIndex, &value))
            return PyErr_Format(PyExc_TypeError, "type error");

        Context* context = Context::GetContext(contextIndex);
        if (!context)
            return PyErr_Format(PyExc_IndexError, __FUNCTION__, "() : index % i is out of range(count = % i)", contextIndex, Context::GetContextCount());

        context->m_input.variable_MaximumStepSize = value;

        Py_INCREF(Py_None);
        return Py_None;
    }

    inline PyObject* Set_UseBackwardAD(PyObject* self, PyObject* args)
    {
        int contextIndex;
        bool value;

        if (!PyArg_ParseTuple(args, "ib:Set_UseBackwardAD", &contextIndex, &value))
            return PyErr_Format(PyExc_TypeError, "type error");

        Context* context = Context::GetContext(contextIndex);
        if (!context)
            return PyErr_Format(PyExc_IndexError, __FUNCTION__, "() : index % i is out of range(count = % i)", contextIndex, Context::GetContextCount());

        context->m_input.variable_UseBackwardAD = value;

        Py_INCREF(Py_None);
        return Py_None;
    }

    inline PyObject* Set_QuantizeDisplay(PyObject* self, PyObject* args)
    {
        int contextIndex;
        bool value;

        if (!PyArg_ParseTuple(args, "ib:Set_QuantizeDisplay", &contextIndex, &value))
            return PyErr_Format(PyExc_TypeError, "type error");

        Context* context = Context::GetContext(contextIndex);
        if (!context)
            return PyErr_Format(PyExc_IndexError, __FUNCTION__, "() : index % i is out of range(count = % i)", contextIndex, Context::GetContextCount());

        context->m_input.variable_QuantizeDisplay = value;

        Py_INCREF(Py_None);
        return Py_None;
    }

    static PyMethodDef pythonModuleMethods[] = {
        {"Set_NumGaussians", Set_NumGaussians, METH_VARARGS, ""},
        {"Set_Reset", Set_Reset, METH_VARARGS, ""},
        {"Set_LearningRate", Set_LearningRate, METH_VARARGS, ""},
        {"Set_MaximumStepSize", Set_MaximumStepSize, METH_VARARGS, ""},
        {"Set_UseBackwardAD", Set_UseBackwardAD, METH_VARARGS, ""},
        {"Set_QuantizeDisplay", Set_QuantizeDisplay, METH_VARARGS, ""},
        {nullptr, nullptr, 0, nullptr}
    };

    static PyModuleDef pythonModule = {
        PyModuleDef_HEAD_INIT, "SlangAutoDiff", NULL, -1, pythonModuleMethods,
        NULL, NULL, NULL, NULL
    };

    PyObject* CreateModule()
    {
        PyObject* module = PyModule_Create(&pythonModule);
        return module;
    }
};
