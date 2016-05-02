#include "cmainwindow.h"
#include "ui_cmainwindow.h"

#include <QCameraInfo>
#include <QImageReader>
#include <QPainter>

CMainWindow::CMainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::CMainWindow)
{
	ui->setupUi(this);

	QWidget * displayWidget = new QWidget(this);
	displayWidget->installEventFilter(this);
	setCentralWidget(displayWidget);

	connect(&_frameGrabber, &ProxyVideoSurface::frameReceived, [this, displayWidget](QImage frame){
		_frame = frame;
		displayWidget->update();
	});

	const auto cameras = QCameraInfo::availableCameras();
	if (cameras.empty())
		return;

	const auto& cameraInfo = cameras.front();
	if (!cameraInfo.isNull())
	{
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
}

CMainWindow::~CMainWindow()
{
	_camera->stop();
	delete ui;
}

bool CMainWindow::eventFilter(QObject* object, QEvent* event)
{
	QWidget * widget = dynamic_cast<QWidget*>(object);
	if (event->type() == QEvent::Paint && widget)
	{
		QPainter painter(widget);
		if (!_frame.isNull())
			painter.drawImage(QPoint(0, 0), _frame);
		else
			painter.fillRect(widget->geometry(), Qt::darkGray);
		painter.end();
	}

	return false;
}
