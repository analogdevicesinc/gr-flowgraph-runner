#include "mainwindow.hpp"
#include "ui_mainwindow.h"

#include <QPushButton>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QDebug>
#include <QSettings>
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

    QSettings settings;
    ui->setupUi(this);
    ui->classname->setReadOnly(true);
    connect(ui->btnBrowse, SIGNAL(clicked(bool)), this, SLOT(onBtnBrowseClicked(bool)));
    connect(ui->btnInitPython, SIGNAL(clicked(bool)), this, SLOT(onBtnInitPythonClicked(bool)));
    connect(ui->btnStartStopFlow, SIGNAL(pressed()), this, SLOT(onBtnStartStopFlowPressed()));
    connect(ui->btnRun,SIGNAL(pressed()),this,SLOT(onBtnRunCmd()));
    connect(ui->btnClearConsole,SIGNAL(pressed()),this,SLOT(onBtnClearConsole()));

    getSettings();

    connect(ui->chkboxAudioPatch,SIGNAL(stateChanged(int)),this,SLOT(saveSettings()));
    connect(ui->chkboxReplaceURI,SIGNAL(stateChanged(int)),this,SLOT(saveSettings()));
    connect(ui->chkboxReplaceRoot,SIGNAL(stateChanged(int)),this,SLOT(saveSettings()));
    connect(ui->lineeditRoot,SIGNAL(editingFinished()),this,SLOT(saveSettings()));
    connect(ui->lineeditUri,SIGNAL(editingFinished()),this,SLOT(saveSettings()));


#ifdef __ANDROID__ // LIBUSB WEAK_AUTHORITY
    libusb_set_option(NULL,LIBUSB_OPTION_ANDROID_JAVAVM,jnienv->javaVM());
    libusb_set_option(NULL,LIBUSB_OPTION_WEAK_AUTHORITY,NULL);
#endif

#ifdef __ANDROID__ // JNI hooks
    registerNativeMethods();
#endif
    requestAndroidPermissions();
    readGrFlowPyFile();
    ui->classname->setText(getGrFlowPyClassName());
}

void MainWindow::getSettings() {
    QSettings settings;

#ifdef __ANDROID__
#define DEFAULT_PATCH_AUDIO true
#else
#define DEFAULT_PATCH_AUDIO false
#endif

     ui->chkboxAudioPatch->setChecked(settings.value(ui->chkboxAudioPatch->objectName(), DEFAULT_PATCH_AUDIO).toBool());
     ui->chkboxReplaceURI->setChecked(settings.value(ui->chkboxReplaceURI->objectName(), false).toBool());
     ui->chkboxReplaceRoot->setChecked(settings.value(ui->chkboxReplaceRoot->objectName(), false).toBool());
     ui->lineeditRoot->setText(settings.value(ui->lineeditRoot->objectName(), "").toString());
     ui->lineeditUri->setText(settings.value(ui->lineeditUri->objectName(), "").toString());
}

void MainWindow::saveSettings() {
    QSettings settings;
    settings.setValue(ui->chkboxAudioPatch->objectName(),ui->chkboxAudioPatch->isChecked());
    settings.setValue(ui->chkboxReplaceURI->objectName(),ui->chkboxReplaceURI->isChecked());
    settings.setValue(ui->chkboxReplaceRoot->objectName(),ui->chkboxReplaceRoot->isChecked());
    settings.setValue(ui->lineeditRoot->objectName(),ui->lineeditRoot->text());
    settings.setValue(ui->lineeditUri->objectName(),ui->lineeditUri->text());
}


void MainWindow::requestAndroidPermissions() {
#if __ANDROID__ // Permissions
    const QStringList permissions({"android.permission.WRITE_EXTERNAL_STORAGE",
                                   "android.permission.READ_EXTERNAL_STORAGE",
                                   "android.permission.RECORD_AUDIO",
                                   "android.permission.CAPTURE_AUDIO_OUTPUT",
                                   "android.permission.MODIFY_AUDIO_SETTINGS",
                                   "android.permission.MANAGE_EXTERNAL_STORAGE",
                                   "android.permission.INTERNET"});

    for(const QString &permission : permissions) {
        auto result = QtAndroid::checkPermission(permission);
        qDebug()<<"Permission: " << permission << " state "  << bool(result) ;
        if(result == QtAndroid::PermissionResult::Denied) {
            auto resultHash = QtAndroid::requestPermissionsSync(permissions);
            if(resultHash[permission] == QtAndroid::PermissionResult::Denied)
                qDebug()<<"Permission " <<permission << "Denied";
            else
                qDebug()<<"Permission " << permission << "Approved";
        }
        else
            qDebug()<<"Permission " << permission << "Approved";
    }
#endif
}


void MainWindow::readGrFlowPyFile() {
    QString grFlowPyFile;
#if __ANDROID__
    grFlowPyFile = (qgetenv("PYTHONPATH")+"/grflow.py");
    qDebug()<<grFlowPyFile;
#else
    grFlowPyFile = "./grflow.py";
    QFileInfo f(grFlowPyFile);
    qDebug()<<f.absoluteFilePath();
#endif

    QFile in(grFlowPyFile);
    in.open(QIODevice::ReadOnly);
    ui->scriptEditor->setText(in.read(SCRIPT_LENGTH_IN_BYTES));
    in.close();

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

QString replaceVariable(QString variable, QString replacement, QString content) {
    return content.replace("self."+variable+" = "+variable+" = ", "self."+variable+" = "+variable+" = \""+replacement+ "\" #");
}

void MainWindow::onBtnBrowseClicked(bool clicked)
{
    QString s = QFileDialog::getOpenFileName(this,
                                             tr("Browse Python script"), "", tr("Python script (*.py);;"), nullptr);
    QFile file(s);    
    file.open(QIODevice::ReadOnly);
    QByteArray content = file.read(SCRIPT_LENGTH_IN_BYTES);
    QString strContent = content;
    if(ui->chkboxAudioPatch->isChecked()) {
        strContent = patchAudioToGrand(strContent);
    }
    if(ui->chkboxReplaceURI->isChecked()) {
        strContent = replaceVariable("uri",ui->lineeditUri->text(),strContent);
    }
    if(ui->chkboxReplaceRoot->isChecked()) {
        strContent = replaceVariable("root",ui->lineeditRoot->text(),strContent);
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

void MainWindow::onBtnStartStopFlowPressed()
{
    if(flow->started) {
        qDebug()<<"stop";
        flow->callFunction("stop");
        ui->btnStartStopFlow->setText("Start");
        flow->started = false;
    }
        else
    {
        qDebug()<<"start";
        flow->callFunction("start");
        ui->btnStartStopFlow->setText("Stop");
        flow->started = true;
    }
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
