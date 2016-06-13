/* This software is developed by caoweiquan322 OR DynamicFatty.
 * All rights reserved.
 *
 * Author: caoweiquan322
 */

#ifndef TRAJECTORY_H
#define TRAJECTORY_H

#include <QObject>
#include "SpatialTemporalPoint.h"
#include "SpatialTemporalSegment.h"
#include <QVector>
#include <QString>

class Trajectory : public QObject
{
    Q_OBJECT
public:
    // The constructors.
    explicit Trajectory(QObject *parent = 0);
    Trajectory(const Trajectory &traj);
    Trajectory(QString filePath);
    Trajectory & operator =(const Trajectory &traj);

    // Interfaces for feeding data.
    void setReferencePoint(const SpatialTemporalPoint &referenceInLL);
    void setPoints(const QVector<double> &longitude, const QVector<double> &latitude,
                   const QVector<double> &timestamp);

    // Validation.
    void validate(bool autoFix = true);

    // Interfaces for retrieving trajectory information.
    int count() const;
    QVector<SpatialTemporalPoint> getPoints() const;
    QVector<SpatialTemporalSegment> getSegments() const;
    QVector<SegmentLocation> getSegmentsAsEuclidPoints() const;
    double getStartTime() const;
    SpatialTemporalPoint estimateReferencePoint() const;

    // Refactor the trajectory.
    void doMercatorProject();
    SpatialTemporalPoint doMercatorProject(const SpatialTemporalPoint& p);
    void doNormalize();
    Trajectory sample(int rate) const;
    Trajectory simplify(double threshold, bool useCascade = false) const;
    QVector<Trajectory> simplifyWithSEST(double dotsTh, double step, bool useTemporal = true) const;
    Trajectory slice(const QVector<int> &indices) const;

    // The visualization.
    void visualize(const QString &plotOption, QString curveName = QString("")) const;

public:
    enum CoordinateType {
        LongitudeLatitude,
        XandY
    };
    static const double MERCATOR_LATITUDE_LB;
    static const double MERCATOR_LATITUDE_UB;
    static const double SCALE_FACTOR_PRECISION;
    static const double EARCH_RADIUS;
    static const double MAX_SPEED; // 130 Km/H.
    static const double MAX_EXCEED_TO_FIX; // 5%

signals:

public slots:

protected:
    // Used to estimate the mercator scale factor from the reference point.
    double getMercatorScaleFactor() const;

protected:
    SpatialTemporalPoint referencePointInLL;
    SpatialTemporalPoint referencePointInXY;
    QVector<SpatialTemporalPoint> points;
    CoordinateType coordinateType;
    bool normalized;
};

#endif // TRAJECTORY_H
