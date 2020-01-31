#-------------------------------------------------
#
# Project created by QtCreator 2019-03-18T11:53:40
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = iXonControl
TEMPLATE = app

SOURCES += main.cpp\
    ixoncamera.cpp \
    datahandler.cpp \
    temperaturecontroller.cpp \
    settingloader.cpp \
    ixoncontrol.cpp \
    roipreview.cpp \
    randomtrackinterface.cpp

HEADERS  += \
    ixoncamera.h \
    datahandler.h \
    temperaturecontroller.h \
    settingloader.h \
    ixoncontrol.h \
    overheatmonitor.h \
    roipreview.h \
    previewlabel.h \
    acquisitionmonitor.h \
    randomtrackinterface.h


FORMS    += \
    settingloader.ui \
    ixoncontrol.ui \
    roipreview.ui \
    randomtrackinterface.ui

LIBS += -L$$PWD/'../../Andor SDK/' -latmcd32m




INCLUDEPATH += $$PWD/'../../Andor SDK'
DEPENDPATH += $$PWD/'../../Andor SDK'


unix|win32: LIBS += -L$$PWD/png++/ -llibpng

INCLUDEPATH += $$PWD/png++
DEPENDPATH += $$PWD/png++

