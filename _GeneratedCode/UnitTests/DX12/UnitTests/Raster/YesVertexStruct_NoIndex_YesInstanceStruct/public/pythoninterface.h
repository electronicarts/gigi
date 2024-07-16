#pragma once

#include "technique.h"

namespace YesVertexStruct_NoIndex_YesInstanceStruct
{
    inline PyObject* ViewModeToString(PyObject* self, PyObject* args)
    {
        int value;
        if (!PyArg_ParseTuple(args, "i:ViewModeToString", &value))
            return PyErr_Format(PyExc_TypeError, "type error");

        switch((ViewMode)value)
        {
            case ViewMode::Normal: return Py_BuildValue("s", "Normal");
            case ViewMode::UV: return Py_BuildValue("s", "UV");
            case ViewMode::Solid: return Py_BuildValue("s", "Solid");
            default: return Py_BuildValue("s", "<invalid ViewMode value>");
        }
    }

    inline PyObject* Set_viewMode(PyObject* self, PyObject* args)
    {
        int contextIndex;
        int value;

        if (!PyArg_ParseTuple(args, "ii:Set_viewMode", &contextIndex, &value))
            return PyErr_Format(PyExc_TypeError, "type error");

        Context* context = Context::GetContext(contextIndex);
        if (!context)
            return PyErr_Format(PyExc_IndexError, __FUNCTION__, "() : index % i is out of range(count = % i)", contextIndex, Context::GetContextCount());

        context->m_input.variable_viewMode = (ViewMode)value;

        Py_INCREF(Py_None);
        return Py_None;
    }

    static PyMethodDef pythonModuleMethods[] = {
        {"ViewModeToString", ViewModeToString, METH_VARARGS, ""},
        {"Set_viewMode", Set_viewMode, METH_VARARGS, ""},
        {nullptr, nullptr, 0, nullptr}
    };

    static PyModuleDef pythonModule = {
        PyModuleDef_HEAD_INIT, "YesVertexStruct_NoIndex_YesInstanceStruct", NULL, -1, pythonModuleMethods,
        NULL, NULL, NULL, NULL
    };

    PyObject* CreateModule()
    {
        PyObject* module = PyModule_Create(&pythonModule);
        PyModule_AddIntConstant(module, "ViewMode_Normal", 0);
        PyModule_AddIntConstant(module, "ViewMode_UV", 1);
        PyModule_AddIntConstant(module, "ViewMode_Solid", 2);
        return module;
    }
};
