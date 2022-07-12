#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include <libm2k/contextbuilder.hpp>
#include <libm2k/context.hpp>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QDebug>
#include <fstream>
#include <sstream>

#define PY_SSIZE_T_CLEAN
#include <Python.h>

using namespace std;
MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow),
	m_current_status(0),
	pName(nullptr),
	pModule(nullptr),
	pClass(nullptr),
	pArgs(nullptr),
	pFunc(nullptr),
	pDict(nullptr),
	m_init(false)
{
	ui->setupUi(this);
	ui->textBrowseScript->setReadOnly(true);
	connect(ui->btnBrowse, SIGNAL(clicked(bool)), this, SLOT(onBtnBrowseClicked(bool)));
	connect(this, SIGNAL(updateUi()), this, SLOT(onUpdateUi()));
	connect(ui->btnOverwrite, SIGNAL(clicked(bool)), this, SLOT(onBtnOverwriteClicked(bool)));
	connect(ui->btnSaveAs, SIGNAL(clicked(bool)), this, SLOT(onBtnSaveAsClicked(bool)));
	connect(ui->btnInitPython, SIGNAL(clicked(bool)), this, SLOT(onBtnInitPythonClicked(bool)));
	connect(ui->textScript, SIGNAL(textChanged()), this, SLOT(onTextScriptChanged()));
	connect(ui->btnStartFlow, SIGNAL(pressed()), this, SLOT(onBtnStartFlowPressed()));
	connect(ui->btnStopFlow, SIGNAL(pressed()), this, SLOT(onBtnStopFlowPressed()));
}

int MainWindow::path_add_to_search_path(std::string path)
{
	PyObject *py_cur_path, *py_item;
	// TBD: PyGILPath for threads running in C
	PyGILState_STATE gstate;
	gstate = PyGILState_Ensure();

	py_cur_path = PySys_GetObject("path");
	if (!py_cur_path) {
		qDebug() << "cannot check current python sys path\n";
		PyGILState_Release(gstate);
		return 1;
	}

	py_item = PyUnicode_FromString(path.c_str());
	if (!py_item) {
		qDebug() << "cannot save the new path into a py object\n";
		PyGILState_Release(gstate);
		return 1;
	}

	if (PyList_Insert(py_cur_path, 0, py_item) < 0) {
		Py_DECREF(py_item);
		qDebug() << "Failed to insert path to search path\n";
		PyGILState_Release(gstate);
		return 1;
	}
	Py_DECREF(py_item);
	PyGILState_Release(gstate);

	return 0;
}

bool MainWindow::validateAndUpdate(QString fileName)
{
	bool ok = true;
	bool needsUpdate = false;
	QFileInfo info(fileName);
	if (info.isFile()) {
		QString ext = info.completeSuffix();
		if (ext != "py") {
			// TBD err
			ok = false;
		}
		if (fileName != m_fullFileName) {
			m_fullFileName = fileName;
			m_moduleName = info.completeBaseName();
			m_dirName = info.absoluteDir().absolutePath();
			needsUpdate = true;
		}
	} else {
		ok = false;
	}
	if (ok && needsUpdate) {
		Q_EMIT updateUi();
	}
	return ok;
}

void MainWindow::onBtnBrowseClicked(bool clicked)
{
	QString fileName = QFileDialog::getOpenFileName(this,
	    tr("Browse Python script"), "", tr("Python script (*.py);;"), nullptr);

	bool ok = validateAndUpdate(fileName);
}

void MainWindow::onBtnOverwriteClicked(bool clicked)
{
	std::fstream file = std::fstream(m_fullFileName.toStdString(), std::ios::out);
	if (!file.is_open()) {
		// TBD err
	}
	file << ui->textScript->toPlainText().toStdString();
	file.close();
	updateStatusLabel(1);
}

void MainWindow::onBtnSaveAsClicked(bool clicked)
{
	QString fileName = QFileDialog::getSaveFileName(this,
	    tr("Browse Python script"), "", tr("Python script (*.py);;"), nullptr);

	bool ok = validateAndUpdate(fileName);
	std::fstream file = std::fstream(m_fullFileName.toStdString(), std::ios::out);
	if (!file.is_open()) {
		// TBD err
	}
	// TBD: Should call emit to update the UI for this
	file << ui->textScript->toPlainText().toStdString();
	file.close();
	updateStatusLabel(2);
}

void MainWindow::cleanup()
{
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
	if (m_init) {
		Py_FinalizeEx();
	}
}

void MainWindow::onBtnInitPythonClicked(bool clicked)
{
	// clean up stuff if already initialized
	cleanup();

	PyObject *result;

	QApplication *app = static_cast<QApplication *>(QApplication::instance());

	// Initialize the Python interpreter
	Py_Initialize();
	m_init = true;
	qDebug() << "Python interpreter version:" << QString(Py_GetVersion());
	qDebug() << "Python standard library path:" << QString::fromWCharArray(Py_GetPath());

	// This should be called after initializing the interpreter
	int ret = path_add_to_search_path(m_dirName.toStdString());

	// This will define the name of the "module" - file without extension
	pName = PyUnicode_FromString(m_moduleName.toStdString().c_str());

	pModule = PyImport_Import(pName);

	if (pModule != NULL) {
	    //argv2 is the method name
		pDict = PyModule_GetDict(pModule);
		pClass = PyDict_GetItemString(pDict, "test"); //name of the python class

		PyObject *pClassInstance = PyObject_CallObject(pClass, NULL);
		if (pClassInstance == NULL) {
			PyErr_Print();
			return;
		}

		pArgs = PyTuple_New(1);
		PyTuple_SetItem(pArgs, 0, pClassInstance);



		pFunc = PyObject_GetAttrString(pClass, "show");
		result = PyObject_CallObject(pFunc, pArgs);
		if (result == NULL) {
			PyErr_Print();
		}


		const QWidgetList &list = QApplication::topLevelWidgets();
		for(QWidget * w : list){
			if(w != this && w->parent() == nullptr) {
				w->setParent(this);
				ui->widgetGrcLayout->insertWidget(0, w);
			}
		}
	} else {
		PyErr_Print();
	}
}

void MainWindow::onUpdateUi()
{
	ui->textBrowseScript->setText(m_fullFileName);
	std::fstream file = std::fstream(m_fullFileName.toStdString(), std::ios::in);
	if (!file.is_open()) {
		// TBD err
	}
	std::stringstream buffer;
	buffer << file.rdbuf();
	file.close();
	ui->textScript->setText(QString::fromStdString(buffer.str()));
	updateStatusLabel(0);
}

void MainWindow::onTextScriptChanged()
{
	updateStatusLabel(4);
}

void MainWindow::updateStatusLabel(int status_option)
{
	if (status_option == m_current_status) {
		return;
	}
	QString status = "File ";
	switch (status_option) {
	case 0:
		status += "loaded";
		break;
	case 1:
		status += "updated";
		break;
	case 2:
		status += "saved as...";
		break;
	case 3:
		status = "Syntax error";
		break;
	case 4:
		status += "changed; not saved";
		break;
	default:
		status = "";
	}

	ui->lblStatus->setText(status);
}

void MainWindow::onBtnStartFlowPressed()
{
	PyObject *result;
	pFunc = PyObject_GetAttrString(pClass, "start");
	result = PyObject_CallObject(pFunc, pArgs);
	if (result == NULL) {
		PyErr_Print();
	}
}

void MainWindow::onBtnStopFlowPressed()
{
	PyObject *result;
	// might need to call "wait" here after "stop"
	pFunc = PyObject_GetAttrString(pClass, "stop");
	result = PyObject_CallObject(pFunc, pArgs);
	if (result == NULL) {
		PyErr_Print();
	}
}

MainWindow::~MainWindow()
{
	cleanup();
	delete ui;
}
