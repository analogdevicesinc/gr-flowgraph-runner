#include "pythoninterpreter.h"
#include "pylogger.h"
#include <QDebug>

PythonInterpreter::PythonInterpreter(QObject *parent) :
    QObject(parent),
    m_init(false)
{
    init();
}
void PythonInterpreter::init() {

    if(m_init)
        return;

    PyImport_AppendInittab("pycustomlogger", PyInit_pycustomlogger);
    Py_InitializeEx(0);
    PyImport_ImportModule("pycustomlogger");

    m_init = true;

}

bool PythonInterpreter::addToSearchPath(QString str) {
    std::string path = str.toStdString();
    PyObject *py_cur_path, *py_item;
    // TBD: PyGILPath for threads running in C
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();

    py_cur_path = PySys_GetObject("path");
    if (!py_cur_path) {
        qDebug() << "cannot check current python sys path\n";
        PyGILState_Release(gstate);
        return false;
    }

    py_item = PyUnicode_FromString(path.c_str());
    if (!py_item) {
        qDebug() << "cannot save the new path into a py object\n";
        PyGILState_Release(gstate);
        return false;
    }

    if (PyList_Insert(py_cur_path, 0, py_item) < 0) {
        Py_DECREF(py_item);
        qDebug() << "Failed to insert path to search path\n";
        PyGILState_Release(gstate);
        return false;
    }
    Py_DECREF(py_item);
    PyGILState_Release(gstate);

    return true;
}

void PythonInterpreter::setStdOutErrWidget(QTextEdit *w) {
    pycustomlogger_setWidget(w);
}

void PythonInterpreter::runSimpleString(QString str) {
    bool result = PyRun_SimpleString(str.toStdString().c_str());

    if (result == 1) {
        PyErr_Print();
        pylogger_printError();
    }
}

void PythonInterpreter::deinit() {

    if (m_init) {
        Py_FinalizeEx();
        m_init = false;
    }
}

PythonInterpreter::~PythonInterpreter() {
    deinit();
}
