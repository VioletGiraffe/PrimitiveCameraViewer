#include "cmainwindow.h"

#include "assert/advanced_assert.h"
#include "settings/csettings.h"
#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QApplication>
#include <QDebug>
RESTORE_COMPILER_WARNINGS

int main(int argc, char *argv [])
{
	QApplication app(argc, argv);

	AdvancedAssert::setLoggingFunc([](const char* message) {
		qDebug() << message;
	});

	app.setOrganizationName("GitHubSoft");
	app.setApplicationName("Primitive Camera Viewer");

	CSettings::setApplicationName(app.applicationName());
	CSettings::setOrganizationName(app.organizationName());

	CMainWindow w;
	w.showMinimized();

	return app.exec();
}
