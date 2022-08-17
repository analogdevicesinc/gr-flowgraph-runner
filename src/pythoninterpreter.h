#ifndef PYTHONINTERPRETER_H
#define PYTHONINTERPRETER_H

#include <Python.h>
#include <QObject>
#include <QTextEdit>

class PythonInterpreter : public QObject{
    Q_OBJECT
public:
    PythonInterpreter(QObject *parent = nullptr);
    ~PythonInterpreter();
    void init();
    void deinit();
    void runSimpleString(QString str);
    void setStdOutErrWidget(QTextEdit *w);
    bool addToSearchPath(QString str);
private:
    bool m_init;
};

#endif // PYTHONINTERPRETER_H
