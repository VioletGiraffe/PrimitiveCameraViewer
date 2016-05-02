#pragma once

#include <QAbstractVideoSurface>
#include <QCamera>
#include <QDebug>
#include <QImage>
#include <QMainWindow>

#include <assert.h>
#include <deque>
#include <memory>

class ProxyVideoSurface: public QAbstractVideoSurface {
	Q_OBJECT
public:
	ProxyVideoSurface(QObject * parent = nullptr) : QAbstractVideoSurface(parent) {}

	QList<QVideoFrame::PixelFormat>	supportedPixelFormats(QAbstractVideoBuffer::HandleType /*type*/) const override
	{
		static const QList<QVideoFrame::PixelFormat> list {
			QVideoFrame::Format_ARGB32,
			QVideoFrame::Format_ARGB32_Premultiplied,
			QVideoFrame::Format_RGB32,
			QVideoFrame::Format_RGB24,
			QVideoFrame::Format_RGB565,
			QVideoFrame::Format_RGB555,
			QVideoFrame::Format_ARGB8565_Premultiplied,
			QVideoFrame::Format_BGRA32,
			QVideoFrame::Format_BGRA32_Premultiplied,
			QVideoFrame::Format_BGR32,
			QVideoFrame::Format_BGR24,
			QVideoFrame::Format_BGR565,
			QVideoFrame::Format_BGR555,
			QVideoFrame::Format_BGRA5658_Premultiplied,
			QVideoFrame::Format_AYUV444,
			QVideoFrame::Format_AYUV444_Premultiplied,
			QVideoFrame::Format_YUV444,
			QVideoFrame::Format_YUV420P,
			QVideoFrame::Format_YV12,
			QVideoFrame::Format_UYVY,
			QVideoFrame::Format_YUYV,
			QVideoFrame::Format_NV12,
			QVideoFrame::Format_NV21,
			QVideoFrame::Format_IMC1,
			QVideoFrame::Format_IMC2,
			QVideoFrame::Format_IMC3,
			QVideoFrame::Format_IMC4,
			QVideoFrame::Format_Y8,
			QVideoFrame::Format_Y16,
			QVideoFrame::Format_Jpeg,
			QVideoFrame::Format_CameraRaw,
			QVideoFrame::Format_AdobeDng
		};

		return list;
	}

	bool present(const QVideoFrame& frame) override
	{
		if (frame.isValid())
		{
			QVideoFrame cloneFrame(frame);
			if (!cloneFrame.map(QAbstractVideoBuffer::ReadOnly))
			{
				qDebug() << "MAP FAILED";
				return true;
			}
			const QImage img = QImage(cloneFrame.bits(),
						 cloneFrame.width(),
						 cloneFrame.height(),
						 QVideoFrame::imageFormatFromPixelFormat(cloneFrame.pixelFormat())).
					mirrored(false, true);

			cloneFrame.unmap();

			if (!img.isNull())
				emit frameReceived(img);

			return true;
		}

		return false;
	}

signals:
	void frameReceived(QImage);
};

class Filter
{
public:
	enum State{Undefined, Invalid, Valid};

	inline Filter() : _samples(6) {
		reset();
	}

	inline State processSample(const bool sample) {
		_samples.pop_front();
		_samples.push_back(sample);

		bool allSamplesTrue = true, allSamplesFalse = true;
		for (const bool s :_samples)
		{
			allSamplesTrue = allSamplesTrue && s == true;
			allSamplesFalse = allSamplesFalse && s == false;
		}

		assert(!allSamplesTrue || !allSamplesFalse);
		if (allSamplesTrue)
			return Valid;
		else if (allSamplesFalse)
			return Invalid;
		else
			return Undefined;
	}

	void reset() {
		bool value = false;
		for (bool& sample: _samples)
		{
			sample = value;
			value = !value;
		}
	}

private:
	std::deque<bool> _samples;
};

namespace Ui {
class CMainWindow;
}

class CMainWindow : public QMainWindow
{
public:
	explicit CMainWindow(QWidget *parent = 0);
	~CMainWindow();

protected:
	bool eventFilter(QObject *, QEvent *) override;

private:
	void startCamera();
	void stopCamera();

private:
	Ui::CMainWindow *ui;

	ProxyVideoSurface _frameGrabber;
	std::shared_ptr<QCamera> _camera;
	QImage _frame;
	Filter _frameScanFilter;
};

