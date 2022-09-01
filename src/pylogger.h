#ifndef PYLOGGER_H
#define PYLOGGER_H

#include <Python.h>
#include <QTextEdit>
#include <iostream>


#define PYCUSTOMLOGGER_EXCEPTION_PRINTER
#define PYTHON_EXCEPTION_PRINTER

#ifdef __ANDROID__
#define log(x) __android_log_write(ANDROID_LOG_DEBUG,"qt-test-py",x)
//#elseif
#else
#define log(x) std::cout<<(x);std::fflush(stdout)
#endif


PyObject* pycustomlogger_write(PyObject* self, PyObject* args);
PyObject* pycustomlogger_flush(PyObject* self, PyObject* args);
extern PyMethodDef pycustomlogger_methods[];
extern PyModuleDef pycustomlogger_module;
PyMODINIT_FUNC PyInit_pycustomlogger(void);

void pylogger_printError();
void pylogger_printObjectAsString(PyObject *obj);

void pycustomlogger_setWidget(QTextEdit *w);
QTextEdit* pycustomlogger_getWidget();
#endif // PYLOGGER_H
