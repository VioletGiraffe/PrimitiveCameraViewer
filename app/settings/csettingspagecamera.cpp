#include "csettingspagecamera.h"

#include "settings/csettings.h"
#include "settings.h"

DISABLE_COMPILER_WARNINGS
#include "ui_csettingspagecamera.h"
RESTORE_COMPILER_WARNINGS

CSettingsPageCamera::CSettingsPageCamera(QWidget *parent) :
	CSettingsPage(parent),
	ui(new Ui::CSettingsPageCamera)
{
	ui->setupUi(this);

	CSettings s;
	ui->_leCameraFilter->setText(s.value(CAMERA_NAME_FILTER_SETTING).toString());
	ui->_sbThreshold->setValue(s.value(IMAGE_PIXEL_VALUE_THRESHOLD_SETTING, IMAGE_PIXEL_VALUE_THRESHOLD_DEFAULT).toInt());
	ui->_cbMirrorH->setChecked(s.value(IMAGE_HORIZONTAL_MIRROR_SETTING, false).toBool());
	ui->_cbMirrorV->setChecked(s.value(IMAGE_VERTICAL_MIRROR_SETTING, false).toBool());
	ui->_sbProbingInterval->setValue(s.value(CAMERA_PROBING_INTERVAL_SETTING, CAMERA_PROBING_INTERVAL_DEFAULT).toInt());
}

CSettingsPageCamera::~CSettingsPageCamera()
{
	delete ui;
}

void CSettingsPageCamera::acceptSettings()
{
	CSettings s;
	s.setValue(CAMERA_NAME_FILTER_SETTING, ui->_leCameraFilter->text());
	s.setValue(IMAGE_PIXEL_VALUE_THRESHOLD_SETTING, ui->_sbThreshold->value());
	s.setValue(IMAGE_HORIZONTAL_MIRROR_SETTING, ui->_cbMirrorH->isChecked());
	s.setValue(IMAGE_VERTICAL_MIRROR_SETTING, ui->_cbMirrorV->isChecked());
	s.setValue(CAMERA_PROBING_INTERVAL_SETTING, ui->_sbProbingInterval->value());
}
