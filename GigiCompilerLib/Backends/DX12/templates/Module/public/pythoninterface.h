/*$(CopyrightHeader)*/#pragma once

#include "technique.h"

namespace /*$(Name)*/
{/*$(PythonFunctionDefinitions)*/

    static PyMethodDef pythonModuleMethods[] = {/*$(PythonFunctionDeclarations)*/
        {nullptr, nullptr, 0, nullptr}
    };

    static PyModuleDef pythonModule = {
        PyModuleDef_HEAD_INIT, "/*$(Name)*/", NULL, -1, pythonModuleMethods,
        NULL, NULL, NULL, NULL
    };

    PyObject* CreateModule()
    {
        PyObject* module = PyModule_Create(&pythonModule);/*$(PythonCreateModule)*/
        return module;
    }
};
