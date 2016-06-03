#-------------------------------------------------
#
# Project created by QtCreator 2016-05-10T11:19:57
#
#-------------------------------------------------

QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport
#QT       -= gui

TARGET = st_pattern
CONFIG   += console
#CONFIG   -= app_bundle

TEMPLATE = app

INCLUDEPATH += /Users/fatty/Downloads/birch-clustering-algorithm/boost_1_61_0

SOURCES += main.cpp \
    DotsException.cpp \
    DotsSimplifier.cpp \
    Helper.cpp \
    qcustomplot/qcustomplot.cpp \
    mainwindow.cpp \
    Trajectory.cpp \
    SpatialTemporalPoint.cpp \
    SpatialTemporalException.cpp \
    RobustnessTester.cpp \
    SpatialTemporalSegment.cpp \
    Apps.cpp

HEADERS += \
    DotsException.h \
    DotsSimplifier.h \
    Helper.h \
    qcustomplot/qcustomplot.h \
    mainwindow.h \
    Trajectory.h \
    SpatialTemporalPoint.h \
    SpatialTemporalException.h \
    RobustnessTester.h \
    SpatialTemporalSegment.h \
    birch/CFTree.h \
    birch/CFTree_Redist.h \
    birch/CFTree_CFCluster.h \
    Apps.h

FORMS += \
    mainwindow.ui
