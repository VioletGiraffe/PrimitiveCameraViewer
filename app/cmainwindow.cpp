#include "cmainwindow.h"
#include "ccameraslist.h"
#include "aboutdialog/caboutdialog.h"
#include "version.h"

#include "settings/settings.h"
#include "settings/csettings.h"
#include "settingsui/csettingsdialog.h"
#include "settings/csettingspagecamera.h"

DISABLE_COMPILER_WARNINGS
#include "ui_cmainwindow.h"

#include <QCameraInfo>
#include <QDebug>
#include <QImageReader>
#include <QPainter>
#include <QTimer>
RESTORE_COMPILER_WARNINGS

#define PROBING_ENABLED_SETTING QStringLiteral("UI/ProbingEnabled")
#define LAST_UPDATE_CHECK_TIMESTAMP QStringLiteral("UI/LastUpdateCheckTimestamp")
#define SHOW_TOOLBAR_SETTING QStringLiteral("UI/ShowToolbar")

CMainWindow::CMainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::CMainWindow)
{
	ui->setupUi(this);
	ui->_displayWidget->installEventFilter(this);

	connect(&_frameGrabber, &ProxyVideoSurface::frameReceived, this, &CMainWindow::processFrame);

	initActions();

	if (ui->actionProbing_enabled->isChecked())
	{
		// Some streaming devices seem to malfunction if they are connected to too soon after Windows startup. This is a safeguard against that.
		QTimer::singleShot(7000, [this](){
			startCamera();
		});
	}

	ui->toolBar->setVisible(CSettings().value(SHOW_TOOLBAR_SETTING, false).toBool());
}

CMainWindow::~CMainWindow()
{
	stopCamera();
	delete ui;
}

bool CMainWindow::eventFilter(QObject* /*object*/, QEvent* event)
{
	if (event->type() == QEvent::Paint)
	{
		QWidget * widget = ui->_displayWidget;
		QPainter painter(widget);
		// Fill the background according the the color assigned to this widget
		painter.fillRect(widget->rect(), painter.background());
		if (!_frame.isNull())
		{
			const QSize scaledImagesize = _frame.size().scaled(widget->size(), Qt::KeepAspectRatio);
			QRect imageDrawRect = widget->geometry();
			imageDrawRect.setSize(scaledImagesize);
			imageDrawRect.translate((widget->width() - scaledImagesize.width()) / 2, (widget->height() - scaledImagesize.height()) / 2);

			CSettings s;
			const bool mirrorH = s.value(IMAGE_HORIZONTAL_MIRROR_SETTING, false).toBool(), mirrorV = s.value(IMAGE_VERTICAL_MIRROR_SETTING, false).toBool();
			painter.drawImage(imageDrawRect, mirrorH || mirrorV ? _frame.mirrored(mirrorH, mirrorV) : _frame);
		}

		if (_currentFrameContentsMetric != -1)
		{
			painter.setRenderHint(QPainter::TextAntialiasing, false);
			painter.setPen(QColor(255, 0, 255));
			painter.drawText(5, painter.fontMetrics().height()+5, QString::number(_currentFrameContentsMetric));
		}

		painter.end();
		return true;
	}

	return false;
}

void CMainWindow::closeEvent(QCloseEvent* e)
{
	CSettings().setValue(SHOW_TOOLBAR_SETTING, ui->toolBar->isVisible());
	QMainWindow::closeEvent(e);
}

void CMainWindow::initActions()
{
	ui->actionProbing_enabled->setChecked(CSettings().value(PROBING_ENABLED_SETTING, true).toBool());
	connect(ui->actionProbing_enabled, &QAction::triggered, [](bool checked){
		CSettings().setValue(PROBING_ENABLED_SETTING, checked);
	});

	connect(ui->actionConnect, &QAction::triggered, [this](bool connect){
		ui->actionConnect->setChecked(!connect);
		if (connect)
			startCamera();
		else
			stopCamera();
	});

	connect(ui->actionSettings, &QAction::triggered, [this](){
		CSettingsDialog(this)
			.addSettingsPage(new CSettingsPageCamera())
			.exec();
	});

	connect(ui->action_View_available_cameras, &QAction::triggered, [this](){
		CCamerasList(this, QCameraInfo::availableCameras(), QString()).exec();
	});

	connect(ui->action_About, &QAction::triggered, [this](){
		CAboutDialog(VERSION_STRING, this).exec();
	});
}

void CMainWindow::startCamera()
{
	if (!_camera)
	{
		const QString cameraNameFilter = CSettings().value(CAMERA_NAME_FILTER_SETTING).toString();
		const auto cameras = QCameraInfo::availableCameras();
		for (const QCameraInfo& cameraInfo : cameras)
		{
			if (cameraInfo.isNull() || (!cameraNameFilter.isEmpty() && !cameraInfo.deviceName().contains(cameraNameFilter)))
				continue;

			qDebug() << "Creating the camera" << cameraInfo.description();
			_camera = std::make_shared<QCamera>(cameraInfo);
			if (!_camera->isAvailable())
			{
				_camera.reset();
				continue;
			}

			_camera->setViewfinder(&_frameGrabber);

			connect(_camera.get(), &QCamera::stateChanged, [this](const QCamera::State state){
					ui->actionConnect->setChecked(state == QCamera::ActiveState);
			});

			break;
		}
	}

	if (_camera)
	{
		qDebug() << "Connecting to the camera";

		_camera->load();
		qDebug() << "Supported resolutions:";
		QSize minSize {31000, 31000};
		for (const QSize& size: _camera->supportedViewfinderResolutions())
		{
			qDebug() << size;
			if (size.width() * size.height() < minSize.width() * minSize.height())
				minSize = size;
		}

		QCameraViewfinderSettings viewFinderSettings =_camera->viewfinderSettings();
		viewFinderSettings.setResolution(minSize);
		_camera->setViewfinderSettings(viewFinderSettings);
		_camera->start();
	}

	_frameScanFilter.reset();
}

void CMainWindow::stopCamera()
{
	if (_camera)
	{
		qDebug() << "Disconnecting from the camera";
		_camera->stop();
		_camera->unload();
		_frame = QImage();
		_currentFrameContentsMetric = -1;
		ui->_displayWidget->update();
	}
}

inline int analyzeFrame(const QImage& frame)
{
	if (frame.depth() != 32)
		return true;

	const int w = frame.width(), h = frame.height();
	const int sampleSquareSize = 20;
	const int sampleStrideW = w / sampleSquareSize, sampleStrideH = h / sampleSquareSize;
	uint64_t pixelsValueSum = 0;
	for (int y = 0; y < h; y += sampleStrideH)
	{
		const uint32_t* scanLine = (const uint32_t*) frame.scanLine(y);
		for (int x = 0; x < w; x += sampleStrideW)
		{
			// TODO: support non-32 bpp images
			// TODO: vectorization
			const uint32_t pixel = scanLine[y];
			pixelsValueSum += ((pixel & 0x00FF0000) >> 16) + ((pixel & 0x0000FF00) >> 8) + (pixel & 0x000000FF);
		}
	}

	const int result = (int) (pixelsValueSum / ((uint64_t) w / sampleStrideW * (uint64_t) h / sampleStrideH * 3ull));
	return result;
}

void CMainWindow::processFrame(QImage frame)
{
	if (!frame.isNull() && ui->actionProbing_enabled->isChecked())
	{
		CSettings s;
		const int threshold = s.value(IMAGE_PIXEL_VALUE_THRESHOLD_SETTING, IMAGE_PIXEL_VALUE_THRESHOLD_DEFAULT).toInt();
		// This is the first frame upon connecting to the camera
		if (_frame.isNull())
		{
			_currentFrameContentsMetric = analyzeFrame(frame);
			const auto frameScanResult = _frameScanFilter.processSample(_currentFrameContentsMetric >= threshold);
			if (frameScanResult == Filter::Invalid)
			{
				// Disconnect and schedule re-check
				stopCamera();
				QTimer::singleShot(s.value(CAMERA_PROBING_INTERVAL_SETTING, CAMERA_PROBING_INTERVAL_DEFAULT).toInt() * 1000, [this](){
					startCamera();
				});

				return;
			}
			else if (frameScanResult == Filter::Undefined)
				return; // To avoid assigning _frame, which would prevent the following frames from being scanned
			else
			{
				// We've detected valid image!
				showNormal(); // showFullScreen doesn't work properly if the window was minimized
				showFullScreen();
				raise();
			}
		}
		else // Not the first frame - apparently, we're streaming a live picture
			// Avoid checking every single frame
		{
			static uint32_t frameCounter = 0;
			++frameCounter;

			if (frameCounter > 30)
			{
				frameCounter = 0;
				_currentFrameContentsMetric = analyzeFrame(frame);
				if (_currentFrameContentsMetric < threshold)
				{
					// Disconnect and schedule re-check
					stopCamera();
					QTimer::singleShot(s.value(CAMERA_PROBING_INTERVAL_SETTING, CAMERA_PROBING_INTERVAL_DEFAULT).toInt() * 1000, [this](){
						startCamera();
					});

					showNormal();
					showMinimized();
					return;
				}
			}
		}
	}

	_frame = frame;
	ui->_displayWidget->update();
}
