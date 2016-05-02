#include "cmainwindow.h"
#include "ui_cmainwindow.h"

#include <QCameraInfo>
#include <QImageReader>
#include <QPainter>
#include <QTime>
#include <QTimer>

inline bool analyzeFrame(const QImage& frame)
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

	const auto avgValue = pixelsValueSum / ((uint64_t) w / sampleStrideW * (uint64_t) h / sampleStrideH * 3ull);
	qDebug() << "Frame metric:" << avgValue;
	return avgValue >= 10;
}

CMainWindow::CMainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::CMainWindow)
{
	ui->setupUi(this);

	QWidget * displayWidget = new QWidget(this);
	displayWidget->installEventFilter(this);
	setCentralWidget(displayWidget);

	connect(&_frameGrabber, &ProxyVideoSurface::frameReceived, [this, displayWidget](QImage frame) {
		if (!frame.isNull())
		{
			// This is the first frame upon connecting to the camera
			if (_frame.isNull())
			{
				if (!analyzeFrame(frame))
				{
					// Disconnect and schedule re-check
					disconnectFromCamera();
					QTimer::singleShot(7000, [this](){
						connectToCamera();
					});
				}
			}
			else // Not the first frame - apparently, we're streaming a live picture
				 // Avoid checking every single frame
			{
				static uint32_t frameCounter = 0;
				++frameCounter;

				if (frameCounter > 50)
				{
					frameCounter = 0;
					if (!analyzeFrame(frame))
					{
						// Diisconnect and schedule re-check
						disconnectFromCamera();
						QTimer::singleShot(7000, [this](){
							connectToCamera();
						});
					}
				}
			}
		}

		_frame = frame;
		displayWidget->update();
	});

	connectToCamera();
}

CMainWindow::~CMainWindow()
{
	disconnectFromCamera();
	delete ui;
}

bool CMainWindow::eventFilter(QObject* object, QEvent* event)
{
	QWidget * widget = dynamic_cast<QWidget*>(object);
	if (event->type() == QEvent::Paint && widget)
	{
		QPainter painter(widget);
		if (!_frame.isNull())
			painter.drawImage(widget->geometry(), _frame);
		else
			painter.fillRect(widget->geometry(), Qt::darkGray);
		painter.end();
	}

	return false;
}

void CMainWindow::connectToCamera()
{
	const auto cameras = QCameraInfo::availableCameras();
	if (cameras.empty())
		return;

	const auto& cameraInfo = cameras.front();
	if (cameraInfo.isNull())
		return;


	qDebug() << "Creating the camera" << cameraInfo.description();
	_camera = std::make_shared<QCamera>(cameraInfo);
	_camera->setViewfinder(&_frameGrabber);

	qDebug() << "Connecting to the camera";
	connect(_camera.get(), &QCamera::stateChanged, [this](const QCamera::State state){
		if (state == QCamera::ActiveState)
		{
			setWindowTitle("Connected");
		}
		else
			setWindowTitle("Not Connected");
	});

	_camera->start();
}

void CMainWindow::disconnectFromCamera()
{
	if (_camera)
	{
		_camera->stop();
		_camera->setViewfinder((QAbstractVideoSurface*)nullptr);
		_frame = QImage();
	}

	_camera.reset();
	setWindowTitle("Not Connected");
}
