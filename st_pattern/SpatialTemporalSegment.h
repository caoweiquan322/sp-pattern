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
        this->theta = other.theta;
        this->scale = other.scale;
        this->start = other.start;
        this->duration = other.duration;
    }
    // Calculate the difference.
    double distance(const SegmentLocation &other, const SegmentWeight &weightSquare) {
        double err = 0;
        err += (this->x - other.x)*(this->x - other.x)*(weightSquare.x);
        err += (this->y - other.y)*(this->y - other.y)*(weightSquare.y);
        err += (this->scale - other.scale)*(this->scale - other.scale)*(weightSquare.scale);
        err += (this->start - other.start)*(this->start - other.start)*(weightSquare.start);
        err += (this->duration - other.duration)*(this->duration - other.duration)*(weightSquare.duration);
        double diffTheta = this->theta - other.theta;
        while (diffTheta > M_PI) {
            diffTheta -= M_PI*2.0;
        }
        while (diffTheta < -M_PI) {
            diffTheta += M_PI*2.0;
        }
        err += diffTheta*diffTheta*weightSquare.theta;
        return qSqrt(err);
    }

    double x;
    double y;
    double theta;
    double scale;
    double start;
    double duration;
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
