#-------------------------------------------------
#
# Project created by QtCreator 2014-10-05T11:51:04
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = OS
TEMPLATE = app


SOURCES += main.cpp\
        os_mainwindow.cpp \
    processtabledialog.cpp \
    os_simulateMemory.cpp \
    os_functionDefine.cpp \
    os_basic.cpp \
    memorygraphdialog.cpp \
    inputmemorysizedialog.cpp \
    input_createprocessdialog.cpp

HEADERS  += os_mainwindow.h \
    processtabledialog.h \
    os_typedefine.h \
    os_simulateMemory.h \
    os_memoryError.h \
    os_basic.h \
    memorygraphdialog.h \
    inputmemorysizedialog.h \
    input_createprocessdialog.h

FORMS    += os_mainwindow.ui \
    processtabledialog.ui \
    memorygraphdialog.ui \
    inputmemorysizedialog.ui \
    input_createprocessdialog.ui
