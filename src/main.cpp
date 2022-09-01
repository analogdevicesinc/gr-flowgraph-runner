#include <QApplication>
#include <QDebug>
#include "mainwindow.hpp"
#pragma push_macro("slots")
#undef slots
#include "Python.h"
#pragma pop_macro("slots")

int main(int argc, char **argv)
{
	QApplication app(argc, argv);
    QCoreApplication::setOrganizationName(ORG_NAME);
    QCoreApplication::setOrganizationDomain(ORG_DOMAIN);
    QCoreApplication::setApplicationName(APP_NAME);

	MainWindow *mainwin = new MainWindow(nullptr);
	mainwin->show();
	int r = 0;
	r = QApplication::exec();
}
