/* This software is developed by caoweiquan322 OR DynamicFatty.
 * All rights reserved.
 *
 * Author: caoweiquan322
 */

#ifndef TRAJECTORY_H
#define TRAJECTORY_H

#include <QObject>
#include "SpatialTemporalPoint.h"
#include <QVector>
#include <QString>

class Trajectory : public QObject
{
    Q_OBJECT
public:
    explicit Trajectory(QObject *parent = 0);
    Trajectory(const Trajectory &traj);
    Trajectory(QString filePath);

    void setReferencePoint(const SpatialTemporalPoint &referenceInLL);
    void setPoints(const QVector<double> &longitude, const QVector<double> &latitude,
                   const QVector<double> &timestamp);
    int count();
    void doMercatorProject();
    SpatialTemporalPoint doMercatorProject(const SpatialTemporalPoint& p);
    void doNormalize();
    Trajectory simplify(double threshold);
    void visualize(Qt::GlobalColor color = Qt::red, QString curveName = QString(""));

public:
    enum CoordinateType {
        LongitudeLatitude,
        XandY
    };
    static const double MERCATOR_LATITUDE_LB;
    static const double MERCATOR_LATITUDE_UB;
    static const double SCALE_FACTOR_PRECISION;
    static const double EARCH_RADIUS;

signals:

public slots:

protected:
    double getMercatorScaleFactor();

protected:
    SpatialTemporalPoint referencePointInLL;
    SpatialTemporalPoint referencePointInXY;
    QVector<SpatialTemporalPoint> points;
    CoordinateType coordinateType;
    bool normalized;
};

#endif // TRAJECTORY_H
