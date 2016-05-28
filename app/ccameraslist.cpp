#include "ccameraslist.h"

DISABLE_COMPILER_WARNINGS
#include "ui_ccameraslist.h"
RESTORE_COMPILER_WARNINGS

CCamerasList::CCamerasList(QWidget *parent, const QList<QCameraInfo>& cameras, const QString& currentCameraName) :
	QDialog(parent),
	ui(new Ui::CCamerasList)
{
	ui->setupUi(this);

	for (const auto& camera : cameras)
	{
		const QString name = camera.deviceName();
		QListWidgetItem * item = new QListWidgetItem(name);
		if (name == currentCameraName)
		{
			QFont f = item->font();
			f.setBold(true);
			item->setFont(f);
		}

		ui->_list->addItem(item);
	}
}

CCamerasList::~CCamerasList()
{
	delete ui;
}
