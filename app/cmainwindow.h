#pragma once

#include "compiler/compiler_warnings_control.h"
#include "cproxyvideosurface.h"

DISABLE_COMPILER_WARNINGS
#include <QAbstractVideoSurface>
#include <QCamera>
#include <QMainWindow>
RESTORE_COMPILER_WARNINGS

#include <assert.h>
#include <deque>
#include <memory>

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

private: // UI setup
	void initActions();

private:
	void startCamera();
	void stopCamera();

	void processFrame(QImage frame);

private:
	Ui::CMainWindow *ui;

	ProxyVideoSurface _frameGrabber;
	std::shared_ptr<QCamera> _camera;
	QImage _frame;
	Filter _frameScanFilter;
	int _currentFrameContentsMetric = -1;
};

