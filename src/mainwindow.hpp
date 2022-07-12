#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "Python.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = nullptr);
	~MainWindow();

	int path_add_to_search_path(std::string path);

private:
	bool validateAndUpdate(QString fileName);
	/*
	 * 0 - Loaded
	 * 1 - Updated (overwrite)
	 * 2 - Saved As
	 * 3 - Syntax Error
	 * 4 - Modified
	 */
	void updateStatusLabel(int status_option);
	void cleanup();

private Q_SLOTS:
	void onBtnStartFlowPressed();
	void onBtnStopFlowPressed();
	void onBtnBrowseClicked(bool clicked);
	void onBtnOverwriteClicked(bool clicked);
	void onBtnSaveAsClicked(bool clicked);
	void onBtnInitPythonClicked(bool clicked);
	void onTextScriptChanged();
	void onUpdateUi();


Q_SIGNALS:
	void updateUi();
private:
	Ui::MainWindow *ui;
	QString m_fullFileName;
	QString m_moduleName;
	QString m_dirName;
	int m_current_status;
	PyObject *pName, *pModule, *pClass, *pArgs, *pFunc, *pDict;
	bool m_init;
};

#endif // MAINWINDOW_H
