#pragma once

#include "technique.h"

namespace VariableAliases
{
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

    inline PyObject* Set_Node1Channel(PyObject* self, PyObject* args)
    {
        int contextIndex;
        int value;

        if (!PyArg_ParseTuple(args, "ii:Set_Node1Channel", &contextIndex, &value))
            return PyErr_Format(PyExc_TypeError, "type error");

        Context* context = Context::GetContext(contextIndex);
        if (!context)
            return PyErr_Format(PyExc_IndexError, __FUNCTION__, "() : index % i is out of range(count = % i)", contextIndex, Context::GetContextCount());

        context->m_input.variable_Node1Channel = value;

        Py_INCREF(Py_None);
        return Py_None;
    }

    inline PyObject* Set_Node1Value(PyObject* self, PyObject* args)
    {
        int contextIndex;
        float value;

        if (!PyArg_ParseTuple(args, "if:Set_Node1Value", &contextIndex, &value))
            return PyErr_Format(PyExc_TypeError, "type error");

        Context* context = Context::GetContext(contextIndex);
        if (!context)
            return PyErr_Format(PyExc_IndexError, __FUNCTION__, "() : index % i is out of range(count = % i)", contextIndex, Context::GetContextCount());

        context->m_input.variable_Node1Value = value;

        Py_INCREF(Py_None);
        return Py_None;
    }

    inline PyObject* Set_Node2Channel(PyObject* self, PyObject* args)
    {
        int contextIndex;
        int value;

        if (!PyArg_ParseTuple(args, "ii:Set_Node2Channel", &contextIndex, &value))
            return PyErr_Format(PyExc_TypeError, "type error");

        Context* context = Context::GetContext(contextIndex);
        if (!context)
            return PyErr_Format(PyExc_IndexError, __FUNCTION__, "() : index % i is out of range(count = % i)", contextIndex, Context::GetContextCount());

        context->m_input.variable_Node2Channel = value;

        Py_INCREF(Py_None);
        return Py_None;
    }

    inline PyObject* Set_Node2Value(PyObject* self, PyObject* args)
    {
        int contextIndex;
        float value;

        if (!PyArg_ParseTuple(args, "if:Set_Node2Value", &contextIndex, &value))
            return PyErr_Format(PyExc_TypeError, "type error");

        Context* context = Context::GetContext(contextIndex);
        if (!context)
            return PyErr_Format(PyExc_IndexError, __FUNCTION__, "() : index % i is out of range(count = % i)", contextIndex, Context::GetContextCount());

        context->m_input.variable_Node2Value = value;

        Py_INCREF(Py_None);
        return Py_None;
    }

    inline PyObject* Set_Node3Channel(PyObject* self, PyObject* args)
    {
        int contextIndex;
        int value;

        if (!PyArg_ParseTuple(args, "ii:Set_Node3Channel", &contextIndex, &value))
            return PyErr_Format(PyExc_TypeError, "type error");

        Context* context = Context::GetContext(contextIndex);
        if (!context)
            return PyErr_Format(PyExc_IndexError, __FUNCTION__, "() : index % i is out of range(count = % i)", contextIndex, Context::GetContextCount());

        context->m_input.variable_Node3Channel = value;

        Py_INCREF(Py_None);
        return Py_None;
    }

    inline PyObject* Set_UnusedFloat(PyObject* self, PyObject* args)
    {
        int contextIndex;
        float value;

        if (!PyArg_ParseTuple(args, "if:Set_UnusedFloat", &contextIndex, &value))
            return PyErr_Format(PyExc_TypeError, "type error");

        Context* context = Context::GetContext(contextIndex);
        if (!context)
            return PyErr_Format(PyExc_IndexError, __FUNCTION__, "() : index % i is out of range(count = % i)", contextIndex, Context::GetContextCount());

        context->m_input.variable_UnusedFloat = value;

        Py_INCREF(Py_None);
        return Py_None;
    }

    static PyMethodDef pythonModuleMethods[] = {
        {"Set_RenderSize", Set_RenderSize, METH_VARARGS, ""},
        {"Set_Node1Channel", Set_Node1Channel, METH_VARARGS, ""},
        {"Set_Node1Value", Set_Node1Value, METH_VARARGS, ""},
        {"Set_Node2Channel", Set_Node2Channel, METH_VARARGS, ""},
        {"Set_Node2Value", Set_Node2Value, METH_VARARGS, ""},
        {"Set_Node3Channel", Set_Node3Channel, METH_VARARGS, ""},
        {"Set_UnusedFloat", Set_UnusedFloat, METH_VARARGS, "This is for the unused alias in the shader"},
        {nullptr, nullptr, 0, nullptr}
    };

    static PyModuleDef pythonModule = {
        PyModuleDef_HEAD_INIT, "VariableAliases", NULL, -1, pythonModuleMethods,
        NULL, NULL, NULL, NULL
    };

    PyObject* CreateModule()
    {
        PyObject* module = PyModule_Create(&pythonModule);
        return module;
    }
};
