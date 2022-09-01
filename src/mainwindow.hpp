#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "Python.h"
#include "pythoninterpreter.h"
#include "pygrflow.h"
#ifdef __ANDROID__
#include <QAndroidJniEnvironment>
#endif

#define ORG_NAME "ADI"
#define ORG_DOMAIN "analog.com"
#define APP_NAME "Gnuradio FlowGraph Runner"
#define SCRIPT_LENGTH_IN_BYTES 10000000

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
    void readGrFlowPyFile();
    QString getGrFlowPyClassName();

private Q_SLOTS:
    void onBtnStartStopFlowPressed();
	void onBtnBrowseClicked(bool clicked);
	void onBtnInitPythonClicked(bool clicked);
    void onBtnRunCmd();
    void onBtnClearConsole();

    void getSettings();
    void saveSettings();

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
