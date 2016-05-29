#-------------------------------------------------
#
# Project created by QtCreator 2016-05-01T19:43:44
#
#-------------------------------------------------

QT = core gui widgets multimedia

TARGET = PrimitiveCameraViewer
TEMPLATE = app

CONFIG += c++14

mac* | linux*{
	CONFIG(release, debug|release):CONFIG += Release
	CONFIG(debug, debug|release):CONFIG += Debug
}

Release:OUTPUT_DIR=release
Debug:OUTPUT_DIR=debug

DESTDIR  = ../bin/$${OUTPUT_DIR}
OBJECTS_DIR = ../build/$${OUTPUT_DIR}/app
MOC_DIR     = ../build/$${OUTPUT_DIR}/app
UI_DIR      = ../build/$${OUTPUT_DIR}/app
RCC_DIR     = ../build/$${OUTPUT_DIR}/app

LIBS += -L../bin/$${OUTPUT_DIR} -lqtutils -lcpputils -lautoupdater

win*{
	QMAKE_CXXFLAGS += /MP /wd4251
	QMAKE_CXXFLAGS_WARN_ON = /W4
	DEFINES += WIN32_LEAN_AND_MEAN NOMINMAX _SCL_SECURE_NO_WARNINGS
}

linux*|mac*{

	QMAKE_CXXFLAGS_WARN_ON = -Wall -Wno-c++11-extensions -Wno-local-type-template-args -Wno-deprecated-register

	CONFIG(release, debug|release):CONFIG += Release
	CONFIG(debug, debug|release):CONFIG += Debug

	Release:DEFINES += NDEBUG=1
	Debug:DEFINES += _DEBUG
}

win32*:!*msvc2012:*msvc* {
	QMAKE_CXXFLAGS += /FS
}

mac*|linux*{
	PRE_TARGETDEPS += $${DESTDIR}/libcore.a $${DESTDIR}/libautoupdater.a
}

win*{
	RC_FILE = resources/simplecamviewer.rc
}

INCLUDEPATH += \
	../cpputils/ \
	../qtutils/ \
	../github-releases-autoupdater/src/


SOURCES += main.cpp\
    cmainwindow.cpp \
    cproxyvideosurface.cpp \
    settings/csettingspagecamera.cpp \
    ccameraslist.cpp \
    caboutdialog.cpp

HEADERS  += cmainwindow.h \
    cproxyvideosurface.h \
    settings/csettingspagecamera.h \
    settings/settings.h \
    ccameraslist.h \
    version.h \
    caboutdialog.h

FORMS    += cmainwindow.ui \
    settings/csettingspagecamera.ui \
    ccameraslist.ui \
    caboutdialog.ui
