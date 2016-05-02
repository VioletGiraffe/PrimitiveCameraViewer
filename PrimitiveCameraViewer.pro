#-------------------------------------------------
#
# Project created by QtCreator 2016-05-01T19:43:44
#
#-------------------------------------------------

QT       = core gui multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = PrimitiveCameraViewer
TEMPLATE = app

QMAKE_CXXFLAGS += /MP


SOURCES += main.cpp\
        cmainwindow.cpp

HEADERS  += cmainwindow.h

FORMS    += cmainwindow.ui
