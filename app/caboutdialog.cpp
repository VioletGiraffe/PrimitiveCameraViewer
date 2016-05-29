#include "caboutdialog.h"
#include "version.h"

DISABLE_COMPILER_WARNINGS
#include "ui_caboutdialog.h"

#include <QApplication>
RESTORE_COMPILER_WARNINGS

CAboutDialog::CAboutDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::CAboutDialog)
{
	ui->setupUi(this);

	ui->lblProgramName->setText(QApplication::applicationDisplayName());
	ui->lblVersion->setText(tr("Version %1 (%2 %3), Qt version %4").arg(VERSION_STRING).arg(__DATE__).arg(__TIME__).arg(QT_VERSION_STR));
}

CAboutDialog::~CAboutDialog()
{
	delete ui;
}
