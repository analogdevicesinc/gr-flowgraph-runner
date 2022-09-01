#ifndef PYGRFLOW_H
#define PYGRFLOW_H

#include <Python.h>
#include "pythoninterpreter.h"


class PyGrFlow
{
public:
    PyGrFlow(PythonInterpreter *py);
    ~PyGrFlow();
    bool callFunction(QString str);
    bool importGrFlow(QString str, QString classname);
    bool started;

private:
    PyObject *pName, *pModule, *pClass, *pArgs, *pFunc, *pDict;
    PythonInterpreter *py;
};

#endif // PYGRFLOW_H
