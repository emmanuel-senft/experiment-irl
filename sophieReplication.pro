#-------------------------------------------------
#
# Project created by QtCreator 2015-09-11T15:23:17
#
#-------------------------------------------------

QT       += core gui
CONFIG += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = sophieReplication
TEMPLATE = app


SOURCES += main.cpp\
        interface.cpp \
    utilities.cpp \
    qlearning.cpp \
    logger.cpp \
    display.cpp

HEADERS  += interface.h \
    utilities.h \
    qlearning.h \
    logger.h \
    display.h

FORMS    += interface.ui
