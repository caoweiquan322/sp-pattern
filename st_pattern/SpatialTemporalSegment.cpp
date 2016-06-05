/* This software is developed by caoweiquan322 OR DynamicFatty.
 * All rights reserved.
 *
 * Author: caoweiquan322
 */

#include "SpatialTemporalSegment.h"
#include <QtMath>

SpatialTemporalSegment::SpatialTemporalSegment()
{

}

SpatialTemporalSegment::SpatialTemporalSegment(const SpatialTemporalPoint &start,
                                               const SpatialTemporalPoint &end)
{
    this->start = start;
    this->end = end;
}

SpatialTemporalSegment::SpatialTemporalSegment(const double &startX, const double &startY, const double &startT,
                                               const double &endX, const double &endY, const double &endT)
    : start(startX, startY, startT), end(endX, endY, endT)
{

}

SpatialTemporalSegment::SpatialTemporalSegment(const SpatialTemporalSegment &other)
{
    this->start = other.start;
    this->end = other.end;
}

SegmentLocation SpatialTemporalSegment::toEuclidPoint()
{
    SegmentLocation location;
    location.x = (start.x + end.x)/2.0;
    location.y = (start.y + end.y)/2.0;
    location.rx = end.x - start.x;
    location.ry = end.y - start.y;
    location.start = start.t;
    location.duration = end.t - start.t;

    return location;
}

