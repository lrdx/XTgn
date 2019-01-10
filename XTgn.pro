#-------------------------------------------------
#
# Project created by QtCreator 2018-10-30T17:33:33
#
#-------------------------------------------------

QT       += core gui
QT	 += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QtXTgn
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

DEFINES += \
	WIN32_LEAN_AND_MEAN \
	BOOST_DATE_TIME_NO_LIB \
	BOOST_REGEX_NO_LIB
	

SOURCES += \            
    src/Logger.cpp \
    src/main.cpp \
    src/mainwindow.cpp \
    src/VRWorker.cpp \
    src/XVideoWriter.cpp \
    src/SettingsWindow.cpp \
    src/SettingsHolder.cpp

HEADERS += \            
    src/Logger.h \
    src/mainwindow.h \
    src/VRWorker.h \
    src/XVideoWriter.h \
    src/SettingsWindow.h \
    src/SettingsHolder.h

FORMS += \
        ui/mainwindow.ui \
	ui/settingswindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
