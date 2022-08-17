#include "pylogger.h"

#include <traceback.h>
#include <frameobject.h>

#ifdef __ANDROID__
#include <android/log.h>
#endif
#include <QDebug>
#include <QWidget>
#include <QTextBrowser>

QTextEdit *pycustomlogger_Widget = nullptr;

void pycustomlogger_setWidget(QTextEdit *w) {
    pycustomlogger_Widget = w;
}

QTextEdit* pycustomlogger_getWidget() {
    return pycustomlogger_Widget;
}


PyObject* pycustomlogger_write(PyObject* self, PyObject* args)
{
    const char *what;
    if (!PyArg_ParseTuple(args, "s", &what))
        return NULL;
    log(what);
    if(pycustomlogger_Widget) {
        QTextCursor prev_cursor = pycustomlogger_Widget->textCursor();
        pycustomlogger_Widget->moveCursor (QTextCursor::End);
        pycustomlogger_Widget->insertPlainText (what);
        pycustomlogger_Widget->setTextCursor (prev_cursor);

    }
    return Py_BuildValue("");
}


PyObject* pycustomlogger_flush(PyObject* self, PyObject* args)
{
    return Py_BuildValue("");
}


PyMethodDef pycustomlogger_methods[] =
{
    {"write", pycustomlogger_write, METH_VARARGS, "doc for write"},
    {"flush", pycustomlogger_flush, METH_VARARGS, "doc for flush"},
    {0, 0, 0, 0} // sentinel
};


PyModuleDef aview_module =
{
    PyModuleDef_HEAD_INIT, // PyModuleDef_Base m_base;
    "pycustomlogger",               // const char* m_name;
    "doc for pycustomlogger",       // const char* m_doc;
    -1,                    // Py_ssize_t m_size;
    pycustomlogger_methods,        // PyMethodDef *m_methods
    //  inquiry m_reload;  traverseproc m_traverse;  inquiry m_clear;  freefunc m_free;
};

PyMODINIT_FUNC PyInit_pycustomlogger(void)
{
    PyObject* m = PyModule_Create(&aview_module);
    PySys_SetObject("stdout", m);
    PySys_SetObject("stderr", m);
    return m;
}


void pylogger_printObjectAsString(PyObject *obj) {
    PyObject* repr = PyObject_Repr(obj);
    PyObject* str = PyUnicode_AsEncodedString(repr, "utf-8", "~E~");
    const char *bytes = PyBytes_AS_STRING(str);

    log(bytes);

    Py_XDECREF(repr);
    Py_XDECREF(str);
}

void pylogger_printError() {
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();
    if(PyErr_Occurred()) {
        PyObject *ptype, *pvalue, *ptraceback;
        PyErr_Fetch(&ptype, &pvalue, &ptraceback);
        PyErr_NormalizeException(&ptype, &pvalue, &ptraceback);
        //Get error message
#ifdef PYCUSTOMLOGGER_EXCEPTION_PRINTER
        qDebug()<<ptype<<pvalue<<ptraceback;
        pylogger_printObjectAsString(ptype);
        pylogger_printObjectAsString(pvalue);

        PyTracebackObject* traceback = (PyTracebackObject*)ptraceback;
        pylogger_printObjectAsString(ptraceback);

        while (traceback != NULL) {
            PyFrameObject *frame = traceback->tb_frame;
            pylogger_printObjectAsString(frame->f_code->co_filename);
            qDebug()<<PyFrame_GetLineNumber(frame);
            //reprint(frame->f_code);
            traceback = traceback->tb_next;
        }
#endif
#ifdef PYTHON_EXCEPTION_PRINTER
        PyErr_Restore(ptype, pvalue, ptraceback);
        PyErr_Print();
#endif
    }
    PyGILState_Release(gstate);
}
