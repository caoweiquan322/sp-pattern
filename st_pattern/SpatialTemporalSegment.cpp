/* This software is developed by caoweiquan322 OR DynamicFatty.
 * All rights reserved.
 *
 * Author: caoweiquan322
 */

#include "SpatialTemporalSegment.h"
#include <QtMath>

QDataStream & operator<< (QDataStream& stream, const SegmentLocation& l) {
    return (stream<<l.x<<l.y<<l.rx<<l.ry<<l.start<<l.duration<<l.id);
}

QDataStream & operator>> (QDataStream& stream, SegmentLocation& l) {
    return (stream>>l.x>>l.y>>l.rx>>l.ry>>l.start>>l.duration>>l.id);
}

unsigned int SpatialTemporalSegment::idCounter = 0;

SpatialTemporalSegment::SpatialTemporalSegment()
{
    // This should never be called explicitly.
}

SpatialTemporalSegment::SpatialTemporalSegment(const SpatialTemporalPoint &start,
                                               const SpatialTemporalPoint &end)
{
    this->start = start;
    this->end = end;
    this->id = (++idCounter);
}

SpatialTemporalSegment::SpatialTemporalSegment(const double &startX, const double &startY, const double &startT,
                                               const double &endX, const double &endY, const double &endT)
    : start(startX, startY, startT), end(endX, endY, endT)
{
    this->id = (++idCounter);
}

SpatialTemporalSegment::SpatialTemporalSegment(const SpatialTemporalSegment &other)
{
    this->start = other.start;
    this->end = other.end;
    this->id = other.id;
}

SegmentLocation SpatialTemporalSegment::toEuclidPoint()
{
    SegmentLocation location(this->id);
    location.x = start.x;
    location.y = start.y;
    location.rx = end.x - start.x;
    location.ry = end.y - start.y;
    location.start = start.t;
    location.duration = end.t - start.t;

    return location;
}

