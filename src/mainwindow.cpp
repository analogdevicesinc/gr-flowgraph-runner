#include "mainwindow.hpp"
#include "ui_mainwindow.h"

#include <QPushButton>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QDebug>
#include <fstream>
#include <sstream>
#include <streambuf>
#include <iostream>
#include "pylogger.h"

#if __ANDROID__
#include <QtAndroidExtras/QtAndroid>
#include <QtAndroidExtras/QAndroidJniEnvironment>
#include <android/log.h>
#endif

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#ifdef __ANDROID__
#include <libusb.h>
#endif

using namespace std;
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    py(nullptr),
    m_init(false)
  #ifdef __ANDROID__
  ,jnienv(new QAndroidJniEnvironment())
  #endif
{
    ui->setupUi(this);
    ui->classname->setReadOnly(true);
    connect(ui->btnBrowse, SIGNAL(clicked(bool)), this, SLOT(onBtnBrowseClicked(bool)));
    connect(ui->btnInitPython, SIGNAL(clicked(bool)), this, SLOT(onBtnInitPythonClicked(bool)));
    connect(ui->btnStartFlow, SIGNAL(pressed()), this, SLOT(onBtnStartFlowPressed()));
    connect(ui->btnStopFlow, SIGNAL(pressed()), this, SLOT(onBtnStopFlowPressed()));
    connect(ui->btnRun,SIGNAL(pressed()),this,SLOT(onBtnRunCmd()));
    connect(ui->btnClearConsole,SIGNAL(pressed()),this,SLOT(onBtnClearConsole()));

#ifdef __ANDROID__ // LIBUSB WEAK_AUTHORITY
    libusb_set_option(NULL,LIBUSB_OPTION_ANDROID_JAVAVM,jnienv->javaVM());
    libusb_set_option(NULL,LIBUSB_OPTION_WEAK_AUTHORITY,NULL);
#endif

#ifdef __ANDROID__ // JNI hooks
    registerNativeMethods();
#endif

#if __ANDROID__ // Permissions
    const QVector<QString> permissions({"android.permission.READ_EXTERNAL_STORAGE",
                                        "android.permission.MANAGE_EXTERNAL_STORAGE",
                                        "android.permission.WRITE_EXTERNAL_STORAGE",
                                        "android.permission.INTERNET"});

    for(const QString &permission : permissions) {
        auto result = QtAndroid::checkPermission(permission);
        if(result == QtAndroid::PermissionResult::Denied) {
            auto resultHash = QtAndroid::requestPermissionsSync(QStringList({permission}));
            if(resultHash[permission] == QtAndroid::PermissionResult::Denied)
                return;
        }
    }
#endif

}

int MainWindow::writeGrFlowPyFile() {
    QByteArray content = ui->scriptEditor->toPlainText().toUtf8();
    QString grFlowPyFile;
#if __ANDROID__
    grFlowPyFile = (qgetenv("PYTHONPATH")+"/grflow.py");
    qDebug()<<grFlowPyFile;
#else
    grFlowPyFile = "./grflow.py";
    QFileInfo f(grFlowPyFile);
    py->addToSearchPath(".");
    qDebug()<<f.absoluteFilePath();
#endif

    QFile out(grFlowPyFile);
    out.open(QIODevice::WriteOnly);
    int ret = out.write(content);
    out.flush();
    out.close();
    return ret;
}

QString MainWindow::getGrFlowPyClassName() {
    QByteArray content = ui->scriptEditor->toPlainText().toUtf8();
    QStringList all = QString(content).split(" ",Qt::SkipEmptyParts);
    int i=0;
    QString classname;
    for(i = 0;i<all.length();i++)
    {
        QString s = all[i];
        if(s.contains("top_block_cls"))
        {
            int first = s.indexOf('=');
            int last = s.indexOf(',');
            qDebug()<<s<<first<<last;
            classname = s.mid(first+1, last-first-1);
            break;
        }
    }    
    qDebug()<<all[i];

    return classname;
}

QString patchAudioToGrand(QString content) {
    content.replace("from gnuradio import audio", "\
from gnuradio import grand\n\
# patchAudioToGrand: replaces audio sink blocks with opensl blocks\n\
class audio:\n\
  def sink(sample_rate, *args):\n\
    return grand.opensl_sink(sample_rate)\n\
  def source(sample_rate, *args):\n\
    return grand.opensl_source(sample_rate)\n\
\n\
");
    return content;
}

void MainWindow::onBtnBrowseClicked(bool clicked)
{
    QString s = QFileDialog::getOpenFileName(this,
                                             tr("Browse Python script"), "", tr("Python script (*.py);;"), nullptr);
    QFile file(s);    
    file.open(QIODevice::ReadOnly);
    QByteArray content = file.read(1000000);
    QString strContent = content;
    if(ui->chkboxAudioPatch->isChecked()) {
        strContent = patchAudioToGrand(strContent);
    }
    ui->scriptEditor->setText(strContent);
    ui->classname->setText(getGrFlowPyClassName());
}


void MainWindow::cleanup()
{

    for(QWidget *w : flowWidgets) {
        PySys_WriteStdout("Deleting widgets\n");
        w->deleteLater();
        flowWidgets.removeAll(w);
    }
    /*if(py) {
        PySys_WriteStdout("Deinit python interpreter\n");
        delete flow;
        delete py;
    }*/

}

void MainWindow::initPythonInterpreter() {
     // Initialize the Python interpreter
     py = (new PythonInterpreter(this));
     py->init();
     py->setStdOutErrWidget(ui->scriptConsole);
     PySys_WriteStdout("Python interpreter version: %s\n",(Py_GetVersion()));
     PySys_WriteStdout("Python standard library path: %ls\n", (Py_GetPath()));

}

void MainWindow::onBtnInitPythonClicked(bool clicked)
{
    // clean up stuff if already initialized
    cleanup();

    //QApplication *app = static_cast<QApplication *>(QApplication::instance());
    initPythonInterpreter();

    qDebug()<<writeGrFlowPyFile();

    QString _m_moduleName = "grflow";
    flow = new PyGrFlow(py);
    bool ret = flow->importGrFlow(_m_moduleName, ui->classname->toPlainText());


    const QWidgetList &list = QApplication::topLevelWidgets();
    for(QWidget * w : list){
        if(w != this && w->parent() == nullptr) {
            w->setParent(this);
            ui->widgetGrcLayout->insertWidget(0, w);
            flowWidgets.append(w);
        }
    }
    if(ret) {
        ui->tabWidget->setTabText(1, ui->classname->toPlainText());
        ui->tabWidget->setCurrentIndex(1);
    }
    qDebug()<<"done";

}

void MainWindow::onBtnStartFlowPressed()
{
    qDebug()<<"start";
    flow->callFunction("start");
}

void MainWindow::onBtnStopFlowPressed()
{
    qDebug()<<"stop";
    flow->callFunction("stop");
}

void MainWindow::onBtnRunCmd() {

    QTextCursor prev_cursor = ui->scriptConsole->textCursor();
    ui->scriptConsole->append (">> "+ ui->scriptCommand->text()+"\n");
    py->runSimpleString(ui->scriptCommand->text());
}

void MainWindow::onBtnClearConsole() {
     ui->scriptConsole->clear();
}

MainWindow::~MainWindow()
{
    cleanup();
    delete ui;
}


#ifdef __ANDROID__
void MainWindow::registerNativeMethods()
{
    return;
    JNINativeMethod methods[] = { };

    QAndroidJniObject activity = QtAndroid::androidActivity();
    QAndroidJniEnvironment env;
    jclass objectClass = env->GetObjectClass(activity.object<jobject>());

    env->RegisterNatives(objectClass,
                         methods,
                         sizeof(methods) / sizeof(methods[0]));
    env->DeleteLocalRef(objectClass);
}
#endif
