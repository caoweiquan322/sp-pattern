/* Copyright © 2015 DynamicFatty. All Rights Reserved. */

#include <QCoreApplication>
#include "DotsSimplifier.h"
#include "Helper.h"
#include "DotsException.h"
#include <QException>
#include <QDebug>
#include <QDir>
#include <QFileInfoList>
#include <QFileInfo>
#include <QString>
#include <QStringList>
#include"mainwindow.h"
#include "Trajectory.h"

QStringList retrieveFilesWithSuffix(QString dirPath, QString suffix, QStringList currentList = QStringList())
{
    try {
        QDir dir(dirPath);
        QFileInfoList files = dir.entryInfoList(QDir::Dirs | QDir::Files);
        foreach (QFileInfo info, files) {
            if (info.isDir() && info.fileName().compare(".") && info.fileName().compare("..")) {
                //qDebug()<<"Processing folder: "<<info.fileName();
                currentList = retrieveFilesWithSuffix(info.absoluteFilePath(), suffix, currentList);
            } else if (info.isFile() && info.fileName().endsWith(suffix)) {
                //qDebug()<<"Adding file: "<<info.absoluteFilePath();
                currentList.append(info.absoluteFilePath());
            }
        }
        return currentList;
    } catch (QException &) {
        return currentList;
    }
}

int main(int argc, char *argv[])
{
    // The application.
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    QApplication::setGraphicsSystem("raster");
#endif
    QApplication a(argc, argv);

    try {
        //QStringList geoLifeList = retrieveFilesWithSuffix("E:\\Geolife Trajectories 1.3", ".plt");
        //qDebug()<<"There are "<<geoLifeList.count()<<" GeoLife *.plt files.";

        QString filePath = "E:\\Geolife Trajectories 1.3\\Data\\000\\Trajectory\\20090703002800.plt";
        filePath = "E:\\MyProjects\\QtCreator\\st-pattern\\test_files\\r6.txt";
        Trajectory traj(filePath);
        traj.setReferencePoint(SpatialTemporalPoint(29.8, 62.62, 0.0));
        traj.doMercatorProject();
        traj.doNormalize();
        traj.visualize(Qt::red, "Original trajectory");

        Trajectory simp = traj.simplify(1.0);
        simp.visualize(Qt::green, "Simplified trajectory");
        qDebug()<<"Original points count: "<<traj.count();
        qDebug()<<"Simplified points count: "<<simp.count();

        qDebug("\nPress any key to continue ...");
        return a.exec();
    } catch (DotsException &e) {
        qDebug()<<e.getMessage();
        //e.raise();
    } catch (QException &) {
        qDebug()<<"Unknown exception.";
    }
}
