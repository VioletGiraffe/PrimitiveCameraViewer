#include "cmainwindow.h"
#include "ui_cmainwindow.h"

#include <QCameraInfo>
#include <QImageReader>
#include <QPainter>
#include <QTime>
#include <QTimer>

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

	const int result = (int)(pixelsValueSum / ((uint64_t) w / sampleStrideW * (uint64_t) h / sampleStrideH * 3ull));
	qDebug() << "Frame metric:" << result;
	return result;
}

CMainWindow::CMainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::CMainWindow)
{
	ui->setupUi(this);

	ui->_displayWidget->installEventFilter(this);

	connect(&_frameGrabber, &ProxyVideoSurface::frameReceived, [this](QImage frame) {

		if (!frame.isNull() && ui->actionProbing_enabled->isChecked())
		{
			// This is the first frame upon connecting to the camera
			if (_frame.isNull())
			{
				_currentFrameContentsMetric = analyzeFrame(frame);
				const auto frameScanResult = _frameScanFilter.processSample(_currentFrameContentsMetric >= ui->_threshold->value());
				if (frameScanResult == Filter::Invalid)
				{
					// Disconnect and schedule re-check
					stopCamera();
					QTimer::singleShot(5000, [this](){
						startCamera();
					});

					return;
				}
				else if (frameScanResult == Filter::Undefined)
					return; // To avoid assigning _frame, which would prevent the following frames from being scanned
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
					if (_currentFrameContentsMetric < ui->_threshold->value())
					{
						// Disconnect and schedule re-check
						stopCamera();
						QTimer::singleShot(5000, [this](){
							startCamera();
						});

						return;
					}
				}
			}
		}

		_frame = frame;
		ui->_displayWidget->update();
	});

	connect(ui->actionConnect, &QAction::triggered, [this](bool connect){
		ui->actionConnect->setChecked(!connect);
		if (connect)
			startCamera();
		else
			stopCamera();
	});

	startCamera();
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
		if (!_frame.isNull())
		{
			const QSize scaledImagesize = _frame.size().scaled(widget->size(), Qt::KeepAspectRatio);
			QRect imageDrawRect = widget->geometry();
			imageDrawRect.setSize(scaledImagesize);
			imageDrawRect.translate((widget->width() - scaledImagesize.width()) / 2, (widget->height() - scaledImagesize.height()) / 2);
			painter.drawImage(imageDrawRect, _frame);
		}
		else
			painter.fillRect(widget->geometry(), Qt::darkGray);

		painter.setRenderHint(QPainter::TextAntialiasing, false);
		painter.setPen(QColor(0, 255, 255));
		QFont font = painter.font();
		font.setBold(true);
		font.setPointSize(14);
		painter.setFont(font);
		painter.drawText(widget->mapTo(this, QPoint(10, 20)), QString::number(_currentFrameContentsMetric));
		painter.end();

		return true;
	}

	return false;
}

void CMainWindow::startCamera()
{
	if (!_camera)
	{
		const auto cameras = QCameraInfo::availableCameras();
		for (const QCameraInfo& cameraInfo : cameras)
		{
			if (cameraInfo.isNull())
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
				if (state == QCamera::ActiveState)
				{
					ui->actionConnect->setChecked(true);
					setWindowTitle("Connected");
				}
				else
				{
					ui->actionConnect->setChecked(false);
					setWindowTitle("Not Connected");
				}
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

	setWindowTitle("Not Connected");
}
