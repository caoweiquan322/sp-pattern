/* Copyright © 2015 DynamicFatty. All Rights Reserved. */

#include <QApplication>
#include "DotsException.h"
#include <QException>
#include <QDebug>
#include "SpatialTemporalException.h"
#include "Apps.h"

int main(int argc, char *argv[])
{
    // The application.
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    QApplication::setGraphicsSystem("raster");
#endif
    QApplication a(argc, argv);

    try {
        qDebug()<<"\n============> The st-pattern begins <============";
        //Apps::segmentTrajectories("/Users/fatty/Code/Matlab/oritxt", ".txt", "segments.dat");
        Apps::segmentTrajectories("/Users/fatty/Code/st-pattern/test_files", ".txt", "segments.dat");

        qDebug()<<"\n============>  The st-pattern ends  <============";
        qDebug("\nPress any key to continue ...");
        return a.exec();
    } catch (DotsException &e) {
        qDebug()<<"Error occurs during running DOTS algorithm: "<<e.getMessage();
        //e.raise();
    } catch (SpatialTemporalException &e) {
        qDebug()<<"Error occurs during pattern mining: "<<e.getMessage();
    } catch (QException &) {
        qDebug()<<"Unknown exception.";
    }
}
