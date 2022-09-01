#include "pygrflow.h"
#include "pylogger.h"
#include <QDebug>

PyGrFlow::PyGrFlow( PythonInterpreter *py) :
    started(false),
    pName(nullptr),
    pModule(nullptr),
    pClass(nullptr),
    pArgs(nullptr),
    pFunc(nullptr),
    pDict(nullptr),
    py(py)
{
}

PyGrFlow::~PyGrFlow() {
    if (pDict) {
        Py_DECREF(pDict);
    }
    if (pFunc) {
        Py_DECREF(pFunc);
    }
    if (pArgs) {
        Py_DECREF(pArgs);
    }
    if (pClass) {
        Py_DECREF(pClass);
    }
    if (pModule) {
        Py_DECREF(pModule);
    }
    if (pName) {
        Py_DECREF(pName);
    }
}

bool PyGrFlow::callFunction(QString str) {
    PyObject *result;
    // might need to call "wait" here after "stop"
    pFunc = PyObject_GetAttrString(pClass, str.toStdString().c_str());
    result = PyObject_CallObject(pFunc, pArgs);
    if (result == NULL) {
        pylogger_printError();
        return false;
    }
    return true;
}

bool PyGrFlow::importGrFlow(QString str, QString classname) {
    QString _m_moduleName = str;
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();
    pName = PyUnicode_FromString(_m_moduleName.toStdString().c_str());

    if(pModule){
        PyObject *m = pModule;
        Py_DECREF(m);
        pModule = PyImport_ReloadModule(pModule);
    }  else {
        pModule = PyImport_Import(pName);
    }

    if (pModule != NULL) {
        //argv2 is the method name
        pDict = PyModule_GetDict(pModule);
        pClass = PyDict_GetItemString(pDict, classname.toStdString().c_str()); //name of the python class

        PyObject *pClassInstance = PyObject_CallObject(pClass, NULL);
        if (pClassInstance == NULL) {
            qDebug()<<"err1";
            PyErr_Print();
            return false;
        }

        pArgs = PyTuple_New(1);
        PyTuple_SetItem(pArgs, 0, pClassInstance);
        bool ret = callFunction("show");
        if(!ret)
            return false;

    } else {
        qDebug()<<"err3";
        pylogger_printError();
        return false;
    }
    PyGILState_Release(gstate);
    return true;
}
