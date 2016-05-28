#pragma once

#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QAbstractVideoSurface>
#include <QImage>
RESTORE_COMPILER_WARNINGS

class ProxyVideoSurface: public QAbstractVideoSurface {
	Q_OBJECT
public:
	ProxyVideoSurface(QObject * parent = nullptr);

	QList<QVideoFrame::PixelFormat>	supportedPixelFormats(QAbstractVideoBuffer::HandleType /*type*/) const override;

	bool present(const QVideoFrame& frame) override;

signals:
	void frameReceived(QImage);
};
