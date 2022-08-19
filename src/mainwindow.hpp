#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "Python.h"
#include "pythoninterpreter.h"
#include "pygrflow.h"
#ifdef __ANDROID__
#include <QAndroidJniEnvironment>
#endif

namespace Ui {
class MainWindow;
}


class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = nullptr);
	~MainWindow();

private:

	void cleanup();
    void initPythonInterpreter();
    void registerNativeMethods();
    void requestAndroidPermissions();

    int writeGrFlowPyFile();
    QString getGrFlowPyClassName();

private Q_SLOTS:
	void onBtnStartFlowPressed();
	void onBtnStopFlowPressed();
	void onBtnBrowseClicked(bool clicked);
	void onBtnInitPythonClicked(bool clicked);
    void onBtnRunCmd();
    void onBtnClearConsole();


Q_SIGNALS:
	void updateUi();
private:
	Ui::MainWindow *ui;
    QList<QWidget*> flowWidgets;
    PythonInterpreter *py;
    PyGrFlow *flow;
	bool m_init;
#ifdef __ANDROID__
    QAndroidJniEnvironment *jnienv;
#endif
};


#endif // MAINWINDOW_H
