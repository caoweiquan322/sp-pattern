/* This software is developed by caoweiquan322 OR DynamicFatty.
 * All rights reserved.
 *
 * Author: caoweiquan322
 */

#include "Trajectory.h"
#include "Helper.h"
#include "SpatialTemporalPoint.h"
#include <QtMath>
#include "SpatialTemporalException.h"
#include "mainwindow.h"
#include "DotsSimplifier.h"

const double Trajectory::MERCATOR_LATITUDE_LB = 2.5*2.0-M_PI/2;
const double Trajectory::MERCATOR_LATITUDE_UB = 87.5*2.0-M_PI/2;
const double Trajectory::SCALE_FACTOR_PRECISION = 1e-4;
const double Trajectory::EARCH_RADIUS = 6378100.0;

Trajectory::Trajectory(QObject *parent) : QObject(parent)
{
    this->coordinateType = Trajectory::LongitudeLatitude;
    this->normalized = false;
}

Trajectory::Trajectory(const Trajectory &traj)
{
    this->referencePointInLL = traj.referencePointInLL;
    this->referencePointInXY = traj.referencePointInXY;
    this->points = traj.points;
    this->coordinateType = traj.coordinateType;
    this->normalized = traj.normalized;
}

Trajectory::Trajectory(QString filePath)
{
    Helper::checkNotNullNorEmpty("Trajectory file path", filePath);
    QVector<double> longitude, latitude, timestamp;
    if (filePath.endsWith(".txt")) {
        // MOPSI dataset.
        Helper::parseMOPSI(filePath, longitude, latitude, timestamp, false, false);
    } else if (filePath.endsWith(".plt")) {
        // GeoLife dataset.
        Helper::parseGeoLife(filePath, longitude, latitude, timestamp, false, false);
    } else {
        SpatialTemporalException(QString("Unrecognized file type: [%1]").arg(filePath)).raise();
    }
    // Copy the points
    this->coordinateType = Trajectory::LongitudeLatitude;
    this->normalized = false;
    this->setPoints(longitude, latitude, timestamp);
}

Trajectory &Trajectory::operator =(const Trajectory &traj)
{
    this->referencePointInLL = traj.referencePointInLL;
    this->referencePointInXY = traj.referencePointInXY;
    this->points = traj.points;
    this->coordinateType = traj.coordinateType;
    this->normalized = traj.normalized;
    return *this;
}

void Trajectory::setReferencePoint(const SpatialTemporalPoint &referenceInLL)
{
    this->referencePointInLL = referenceInLL;
    this->referencePointInXY = doMercatorProject(referenceInLL);
}

void Trajectory::setPoints(const QVector<double> &longitude, const QVector<double> &latitude,
                           const QVector<double> &timestamp)
{
    Helper::checkIntEqual(longitude.count(), latitude.count());
    Helper::checkIntEqual(latitude.count(), timestamp.count());
    for (int i=0; i<longitude.count(); ++i)
        this->points.append(SpatialTemporalPoint(longitude.at(i), latitude.at(i), timestamp.at(i)));
    this->coordinateType = Trajectory::LongitudeLatitude;
}

void Trajectory::validate()
{
    double lastTime = -Helper::INF;
    int numChecked = 0;
    foreach (SpatialTemporalPoint p, points) {
        if (p.t < lastTime)
            SpatialTemporalException(QString("Trajectory::validate() failed while checking index %1. "\
                                             "Details: Expecting %2 < %3").
                                     arg(numChecked).arg(lastTime).arg(p.t)).raise();
        lastTime = p.t;
        ++numChecked;
    }
    // All checking done.
}

int Trajectory::count() const
{
    return points.count();
}

QVector<SpatialTemporalPoint> Trajectory::getPoints() const
{
    return points;
}

QVector<SpatialTemporalSegment> Trajectory::getSegments() const
{
    QVector<SpatialTemporalSegment> segments;
    for (int i=0; i<count()-1; ++i) {
        segments.append(SpatialTemporalSegment(points.at(i), points.at(i+1)));
    }
    return segments;
}

QVector<SegmentLocation> Trajectory::getSegmentsAsEuclidPoints() const
{
    QVector<SegmentLocation> segmentsAsEuclidPoints;
    QVector<SpatialTemporalSegment> segments = getSegments();
    foreach (SpatialTemporalSegment s, segments) {
        segmentsAsEuclidPoints.append(s.toEuclidPoint());
    }
    return segmentsAsEuclidPoints;
}

double Trajectory::getStartTime() const
{
    if (count() == 0)
        return 0;
    else
        return points.at(0).t;
}

SpatialTemporalPoint Trajectory::estimateReferencePoint() const
{
    double xSum = 0, ySum = 0;
    foreach (SpatialTemporalPoint p, points) {
        xSum += p.x;
        ySum += p.y;
    }
    return SpatialTemporalPoint(xSum/count(), ySum/count(), getStartTime());
}

void Trajectory::doMercatorProject()
{

    double sf = getMercatorScaleFactor();
    double finalFactor = EARCH_RADIUS/sf;
    SpatialTemporalPoint p;
    double ry;
    for (int i=0; i<this->points.count(); ++i)
    {
        p = this->points.at(i);
        this->points[i].x = qDegreesToRadians(p.x)*finalFactor;
        ry = qDegreesToRadians(Helper::limitVal(p.y, MERCATOR_LATITUDE_LB, MERCATOR_LATITUDE_UB));
        ry = qLn(qFabs(qTan(ry)+1.0/qCos(ry)));
        this->points[i].y = ry*finalFactor;
    }
    this->coordinateType = Trajectory::XandY;
}

SpatialTemporalPoint Trajectory::doMercatorProject(const SpatialTemporalPoint &p)
{
    double sf = getMercatorScaleFactor();
    double finalFactor = EARCH_RADIUS/sf;
    SpatialTemporalPoint ret;
    ret.x = qDegreesToRadians(p.x)*finalFactor;
    ret.y = qDegreesToRadians(Helper::limitVal(p.y, MERCATOR_LATITUDE_LB, MERCATOR_LATITUDE_UB));
    ret.y = qLn(qFabs(qTan(ret.y)+1.0/qCos(ret.y)))*finalFactor;
    ret.t = p.t;

    return ret;
}

void Trajectory::doNormalize()
{
    SpatialTemporalPoint reference = this->coordinateType == Trajectory::LongitudeLatitude ?
                this->referencePointInLL : this->referencePointInXY;
    for (int i=0; i<this->points.count(); ++i)
    {
        this->points[i] -= reference;
    }
    this->normalized = true;
}

Trajectory Trajectory::sample(int rate) const
{
    Helper::checkPositive("sample rate", rate);
    QVector<SpatialTemporalPoint> _pts;
    for (int i=0; i<count(); i+=rate) {
        _pts.append(points.at(i));
    }
    Trajectory sampled(*this);
    sampled.points = _pts;
    return sampled;
}

Trajectory Trajectory::simplify(double threshold, bool useCascade) const
{
    QVector<double> _x, _y, _t;
    foreach (SpatialTemporalPoint p, points) {
        _x.append(p.x);
        _y.append(p.y);
        _t.append(p.t);
    }

    QVector<int> indices;
    if (useCascade) {
        DotsSimplifier::batchDotsCascadeByIndex(_x, _y, _t, indices, threshold);
    } else {
        DotsSimplifier::batchDotsByIndex(_x, _y, _t, indices, threshold);
    }

    return slice(indices);
}

Trajectory Trajectory::slice(const QVector<int> &indices) const
{
    Trajectory traj(*this);
    Helper::slice<SpatialTemporalPoint>(this->points, indices, traj.points);
    return traj;
}

void Trajectory::visualize(Qt::GlobalColor color, QString curveName) const
{
    MainWindow *figure = new MainWindow();
    QVector<double> _x, _y;
    foreach (SpatialTemporalPoint p, points) {
        _x.append(p.x);
        _y.append(p.y);
    }
    figure->plot(_x, _y, color, curveName);
    figure->show();
}

double Trajectory::getMercatorScaleFactor() const
{
    double sf = 1.0/qCos(qDegreesToRadians(this->referencePointInLL.y));
    return qRound(sf/SCALE_FACTOR_PRECISION)*SCALE_FACTOR_PRECISION;
}

