#-------------------------------------------------
#
# Project created by QtCreator 2015-03-09T16:48:00
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = JZFS
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    device.cpp \
    file.cpp \
    partitionmetadata.cpp \
    filetable.cpp \
    datablock.cpp

HEADERS += \
    device.h \
    file.h \
    partitionmetadata.h \
    filetable.h \
    datablock.h
