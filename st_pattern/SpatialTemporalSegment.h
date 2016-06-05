/* This software is developed by caoweiquan322 OR DynamicFatty.
 * All rights reserved.
 *
 * Author: caoweiquan322
 */

#ifndef SPATIALTEMPORALSEGMENT_H
#define SPATIALTEMPORALSEGMENT_H

#include "SpatialTemporalPoint.h"
#include <QtMath>

typedef class SegmentLocation SegmentWeight;

class SegmentLocation
{
public:
    SegmentLocation() {
        // Yield.
    }
    SegmentLocation(const SegmentLocation &other) {
        this->x = other.x;
        this->y = other.y;
        this->rx = other.rx;
        this->ry = other.ry;
        this->start = other.start;
        this->duration = other.duration;
    }
    // Calculate the difference.
    double distance(const SegmentLocation &other, const SegmentWeight &weightSquare) {
        double err = 0;
        err += (this->x - other.x)*(this->x - other.x)*(weightSquare.x);
        err += (this->y - other.y)*(this->y - other.y)*(weightSquare.y);
        err += (this->rx - other.rx)*(this->rx - other.rx)*(weightSquare.rx);
        err += (this->ry - other.ry)*(this->ry - other.ry)*(weightSquare.ry);
        err += (this->start - other.start)*(this->start - other.start)*(weightSquare.start);
        err += (this->duration - other.duration)*(this->duration - other.duration)*(weightSquare.duration);
        return qSqrt(err);
    }

    double x;       // The position x of the start point.
    double y;       // The position y of the start point.
    double rx;      // The relative position x of the end point.
    double ry;      // The relative position y of the end point.
    double start;   // The timestamp of the start point.
    double duration;// The duration of the segment.
};


class SpatialTemporalSegment
{
public:
    // The constructors.
    explicit SpatialTemporalSegment();
    SpatialTemporalSegment(const SpatialTemporalPoint &start, const SpatialTemporalPoint &end);
    SpatialTemporalSegment(const double &startX, const double &startY, const double &startT,
                           const double &endX, const double &endY, const double &endT);
    SpatialTemporalSegment(const SpatialTemporalSegment &other);

    // The distance definitions.
    SegmentLocation toEuclidPoint();

protected:
    SpatialTemporalPoint start;
    SpatialTemporalPoint end;
};

#endif // SPATIALTEMPORALSEGMENT_H
