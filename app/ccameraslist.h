#pragma once

#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QCameraInfo>
#include <QDialog>
RESTORE_COMPILER_WARNINGS

namespace Ui {
class CCamerasList;
}

class CCamerasList : public QDialog
{
public:
	CCamerasList(QWidget *parent, const QList<QCameraInfo>& cameras, const QString& currentCameraName);
	~CCamerasList();

private:
	Ui::CCamerasList *ui;
};
