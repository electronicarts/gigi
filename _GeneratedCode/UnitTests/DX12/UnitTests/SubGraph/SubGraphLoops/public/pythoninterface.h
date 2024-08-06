#pragma once

#include "technique.h"

namespace SubGraphLoops
{
    inline PyObject* Set_FilterSub_Iteration_0_sRGB(PyObject* self, PyObject* args)
    {
        int contextIndex;
        bool value;

        if (!PyArg_ParseTuple(args, "ib:Set_FilterSub_Iteration_0_sRGB", &contextIndex, &value))
            return PyErr_Format(PyExc_TypeError, "type error");

        Context* context = Context::GetContext(contextIndex);
        if (!context)
            return PyErr_Format(PyExc_IndexError, __FUNCTION__, "() : index % i is out of range(count = % i)", contextIndex, Context::GetContextCount());

        context->m_input.variable_FilterSub_Iteration_0_sRGB = value;

        Py_INCREF(Py_None);
        return Py_None;
    }

    inline PyObject* Set_FilterSub_Iteration_1_sRGB(PyObject* self, PyObject* args)
    {
        int contextIndex;
        bool value;

        if (!PyArg_ParseTuple(args, "ib:Set_FilterSub_Iteration_1_sRGB", &contextIndex, &value))
            return PyErr_Format(PyExc_TypeError, "type error");

        Context* context = Context::GetContext(contextIndex);
        if (!context)
            return PyErr_Format(PyExc_IndexError, __FUNCTION__, "() : index % i is out of range(count = % i)", contextIndex, Context::GetContextCount());

        context->m_input.variable_FilterSub_Iteration_1_sRGB = value;

        Py_INCREF(Py_None);
        return Py_None;
    }

    inline PyObject* Set_FilterSub_Iteration_2_sRGB(PyObject* self, PyObject* args)
    {
        int contextIndex;
        bool value;

        if (!PyArg_ParseTuple(args, "ib:Set_FilterSub_Iteration_2_sRGB", &contextIndex, &value))
            return PyErr_Format(PyExc_TypeError, "type error");

        Context* context = Context::GetContext(contextIndex);
        if (!context)
            return PyErr_Format(PyExc_IndexError, __FUNCTION__, "() : index % i is out of range(count = % i)", contextIndex, Context::GetContextCount());

        context->m_input.variable_FilterSub_Iteration_2_sRGB = value;

        Py_INCREF(Py_None);
        return Py_None;
    }

    inline PyObject* Set_FilterSub_Iteration_3_sRGB(PyObject* self, PyObject* args)
    {
        int contextIndex;
        bool value;

        if (!PyArg_ParseTuple(args, "ib:Set_FilterSub_Iteration_3_sRGB", &contextIndex, &value))
            return PyErr_Format(PyExc_TypeError, "type error");

        Context* context = Context::GetContext(contextIndex);
        if (!context)
            return PyErr_Format(PyExc_IndexError, __FUNCTION__, "() : index % i is out of range(count = % i)", contextIndex, Context::GetContextCount());

        context->m_input.variable_FilterSub_Iteration_3_sRGB = value;

        Py_INCREF(Py_None);
        return Py_None;
    }

    inline PyObject* Set_FilterSub_Iteration_4_sRGB(PyObject* self, PyObject* args)
    {
        int contextIndex;
        bool value;

        if (!PyArg_ParseTuple(args, "ib:Set_FilterSub_Iteration_4_sRGB", &contextIndex, &value))
            return PyErr_Format(PyExc_TypeError, "type error");

        Context* context = Context::GetContext(contextIndex);
        if (!context)
            return PyErr_Format(PyExc_IndexError, __FUNCTION__, "() : index % i is out of range(count = % i)", contextIndex, Context::GetContextCount());

        context->m_input.variable_FilterSub_Iteration_4_sRGB = value;

        Py_INCREF(Py_None);
        return Py_None;
    }

    static PyMethodDef pythonModuleMethods[] = {
        {"Set_FilterSub_Iteration_0_sRGB", Set_FilterSub_Iteration_0_sRGB, METH_VARARGS, ""},
        {"Set_FilterSub_Iteration_1_sRGB", Set_FilterSub_Iteration_1_sRGB, METH_VARARGS, ""},
        {"Set_FilterSub_Iteration_2_sRGB", Set_FilterSub_Iteration_2_sRGB, METH_VARARGS, ""},
        {"Set_FilterSub_Iteration_3_sRGB", Set_FilterSub_Iteration_3_sRGB, METH_VARARGS, ""},
        {"Set_FilterSub_Iteration_4_sRGB", Set_FilterSub_Iteration_4_sRGB, METH_VARARGS, ""},
        {nullptr, nullptr, 0, nullptr}
    };

    static PyModuleDef pythonModule = {
        PyModuleDef_HEAD_INIT, "SubGraphLoops", NULL, -1, pythonModuleMethods,
        NULL, NULL, NULL, NULL
    };

    PyObject* CreateModule()
    {
        PyObject* module = PyModule_Create(&pythonModule);
        return module;
    }
};
