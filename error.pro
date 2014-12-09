#-------------------------------------------------
#
# Project created by QtCreator 2014-12-09T17:35:56
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = error
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    tubeview.cpp

HEADERS  += mainwindow.h \
    tubeview.h

FORMS    += mainwindow.ui \
    tubeview.ui

include(/usr/local/qwt-6.1.1/features/qwt.prf)
CONFIG += qwt

CONFIG += link_pkgconfig
PKGCONFIG += ibex
