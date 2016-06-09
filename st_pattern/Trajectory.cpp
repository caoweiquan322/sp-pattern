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
#include <QSet>

const double Trajectory::MERCATOR_LATITUDE_LB = 2.5*2.0-M_PI/2;
const double Trajectory::MERCATOR_LATITUDE_UB = 87.5*2.0-M_PI/2;
const double Trajectory::SCALE_FACTOR_PRECISION = 1e-4;
const double Trajectory::EARCH_RADIUS = 6378100.0;
const double Trajectory::MAX_SPEED = 130.0/3.6;
const double Trajectory::MAX_EXCEED_TO_FIX = 0.05;

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
        SpatialTemporalException(QString("Unrecognized file type: [%1]").
                                 arg(filePath)).raise();
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

void Trajectory::validate(bool autoFix)
{
    // Checking time is monotonous.
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
    // Checking maximum speed.
    double speed;
    SpatialTemporalPoint p, lastP = points.at(0);
    QSet<int> exceed;
    for (int i=1; i<count(); ++i) {
        p = points.at(i);
        p -= lastP;
        speed = qSqrt(p.x*p.x+p.y*p.y)/(p.t);
        if (speed > MAX_SPEED) {
            exceed<<i;
            //qDebug()<<"Exceed speed: "<<(speed*3.6)<<" km/h.";
        } else {
            lastP = points.at(i);
        }
    }
    if (exceed.count() > count()*MAX_EXCEED_TO_FIX || !autoFix) {
        SpatialTemporalException(QString("Number of segments that exceed maximum speed 130 km/h: (%1, %2)").
                                 arg(exceed.count()).arg(count()-1)).raise();
    } else if (exceed.count()>0) {
        // Fix the trajectory.
        qDebug()<<"Auto-fix the trajectory since malformed rate is "
               <<((double)exceed.count()/((double)count())*100.0)<<"%";
        QVector<SpatialTemporalPoint> npoints;
        for (int i=0; i<count(); ++i) {
            if (!exceed.contains(i))
                npoints<<points.at(i);
        }
        points = npoints;
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
    if (count() == 0)
        SpatialTemporalException("The trajectory contains no points.").raise();

    double xSum = 0, ySum = 0;
    foreach (SpatialTemporalPoint p, points) {
        xSum += p.x;
        ySum += p.y;
    }
    return SpatialTemporalPoint(xSum/count(), ySum/count(), 0);
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
    Helper::checkIntEqual(this->coordinateType, Trajectory::LongitudeLatitude);

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
    if (normalized)
        SpatialTemporalException("The trajectory has already been normalized.").raise();

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
    // Checking.
    if (!normalized)
        SpatialTemporalException("The trajectory has not been normalized yet.").raise();
    if (count() <= 2)
        SpatialTemporalException("The trajectory contains number of points less than 2.").raise();

    // Preparing data.
    double dotsScale = 0.001; // This scale is used to prevent numerical errors.
    QVector<double> _x, _y, _t;
    double refX = points.at(0).x;
    double refY = points.at(0).y;
    double refT = points.at(0).t;
    foreach (SpatialTemporalPoint p, points) {
        _x.append((p.x - refX)*dotsScale);
        _y.append((p.y - refY)*dotsScale);
        _t.append((p.t - refT)*dotsScale);
    }

    // Simplify.
    QVector<int> indices;
    if (useCascade) {
        DotsSimplifier::batchDotsCascadeByIndex(_x, _y, _t, indices, threshold*dotsScale*dotsScale);
    } else {
        DotsSimplifier::batchDotsByIndex(_x, _y, _t, indices, threshold*dotsScale*dotsScale);
    }

    return slice(indices);
}

QVector<Trajectory> Trajectory::simplifyWithSEST(double step, bool useTemporal) const
{
    // Checking.
    if (!normalized)
        SpatialTemporalException("The trajectory has not been normalized yet.").raise();
    if (count() <= 2)
        SpatialTemporalException("The trajectory contains number of points less than 2.").raise();

    // Preparing data.
    double dotsScale = 0.001; // This scale is used to prevent numerical errors.
    QVector<double> _x, _y, _t;
    double refX = points.at(0).x;
    double refY = points.at(0).y;
    double refT = points.at(0).t;
    foreach (SpatialTemporalPoint p, points) {
        _x.append((p.x - refX)*dotsScale);
        _y.append((p.y - refY)*dotsScale);
        _t.append((p.t - refT)*dotsScale);
    }

    // Simplify by cascade structure.
    qDebug()<<"Use temporal info: "<<useTemporal;
    QVector<Trajectory> subTrajs;
    static const double DEFAULT_START_THRESHOLD = 100;// Start by 100 meters as a threshold by default.
    static const double DEFAULT_END_THRESHOLD = 1e8;//
    double startThreshold = qMin(DEFAULT_START_THRESHOLD, DEFAULT_END_THRESHOLD/8.0);
    int cascadeCount = qFloor(qLn(DEFAULT_END_THRESHOLD/startThreshold)/qLn(step))+1;
    double k = qPow(DEFAULT_END_THRESHOLD/startThreshold, 1.0/(cascadeCount-1));
    double th = startThreshold;
    QObject *root = new QObject();
    QVector<DotsSimplifier *> cascade;
    for (int i=0; i<cascadeCount; ++i) {
        DotsSimplifier *s = new DotsSimplifier(root, i==0 ? NULL : cascade[0]);
        s->setParameters(th*dotsScale*dotsScale, k);
        th*=k;
        cascade.append(s);
    }

    // Run DOTS in cascade manner.
    int pointCount = this->count();
    double px, py, pt;
    DotsSimplifier *first = cascade[0];
    for (int i=0; i<pointCount; ++i) {
        px = _x.at(i);
        py = _y.at(i);
        pt = _t.at(i);
        first->feedData(px, py, pt);
        int index = -1;
        if (first->readOutputIndex(index)) {
            for (int j=1; j<cascadeCount; ++j) {
                DotsSimplifier *s = cascade[j];
                s->feedIndex(index);
                if (!(s->readOutputIndex(index))) {
                    break;
                }
            }
        }
    }

    // Finish simplifiers from front to end.
    for (int i=0; i<cascadeCount-1; ++i) {
        DotsSimplifier *s = cascade[i];
        DotsSimplifier *n = cascade[i+1];
        s->finish();
        int index = -1;
        while (s->readOutputIndex(index))
            n->feedIndex(index);
    }
    // Read output from the last simplifier.
    DotsSimplifier *last = cascade.last();
    last->finish();
    int index = -1;
    while (last->readOutputIndex(index)) ;

    // Get the output.
    QVector<double> icrs;// Incremental compression rates.
    foreach (DotsSimplifier *s, cascade) {
        icrs.append((s->getInputCount())/1.0/(s->getOutputCount()));
    }
    for (int i=0; i<icrs.count(); ++i)
    {
        if (i>0 && i<icrs.count()-1) {
            if (icrs[i] <= icrs[i-1] && icrs[i] <= icrs[i+1] && icrs[i-1]>1.0) {
                DotsSimplifier *s = cascade[i];
                QVector<int> indices;
                for (int j=0; j<s->getOutputCount(); ++j)
                    indices.append(s->getSimplifiedIndex(j));
                subTrajs.append(this->slice(indices));
            }
        }
    }

    // Destroy the total cascade.
    delete root;
    return subTrajs;
}

Trajectory Trajectory::slice(const QVector<int> &indices) const
{
    Trajectory traj(*this);
    Helper::slice<SpatialTemporalPoint>(this->points, indices, traj.points);
    return traj;
}

void Trajectory::visualize(const QString &plotOption, QString curveName) const
{
    MainWindow *figure = new MainWindow();
    QVector<double> _x, _y;
    foreach (SpatialTemporalPoint p, points) {
        _x.append(p.x);
        _y.append(p.y);
    }
    figure->plot(_x, _y, plotOption, curveName);
    figure->show();
}

double Trajectory::getMercatorScaleFactor() const
{
    double sf = 1.0/qCos(qDegreesToRadians(this->referencePointInLL.y));
    return qRound(sf/SCALE_FACTOR_PRECISION)*SCALE_FACTOR_PRECISION;
}

