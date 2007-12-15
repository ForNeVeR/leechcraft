######################################################################
# Automatically generated by qmake (2.01a) Tue Feb 27 17:02:21 2007
######################################################################

TEMPLATE = lib
CONFIG += plugin release threads
TARGET = leechcraft_http
DESTDIR = ../bin
DEPENDPATH += .
INCLUDEPATH += ../../
INCLUDEPATH += .
QT += network

# Input
TRANSLATIONS = leechcraft_http_en.ts \
	       leechcraft_http_ru.ts
FORMS += fileexistsdialog.ui
HEADERS += httpplugin.h \
	jobadderdialog.h \
	jobparams.h \
	job.h \
	httpimp.h \
	ftpimp.h \
	jobmanager.h \
	joblistitem.h \
	fileexistsdialog.h \
	finishedjob.h \
	settingsmanager.h \
	impbase.h \
	mainviewdelegate.h \
	globals.h
SOURCES += httpplugin.cpp \
	jobadderdialog.cpp \
	jobparams.cpp \
	job.cpp \
	httpimp.cpp \
	ftpimp.cpp \
	jobmanager.cpp \
	joblistitem.cpp \
	fileexistsdialog.cpp \
	finishedjob.cpp \
	impbase.cpp \
	mainviewdelegate.cpp \
	settingsmanager.cpp
RESOURCES = resources.qrc
win32:LIBS += -L../.. -lplugininterface -lexceptions -lsettingsdialog
