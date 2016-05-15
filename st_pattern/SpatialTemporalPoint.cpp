/* This software is developed by caoweiquan322 OR DynamicFatty.
 * All rights reserved.
 *
 * Author: caoweiquan322
 */

#include "SpatialTemporalPoint.h"

SpatialTemporalPoint::SpatialTemporalPoint(double x, double y, double t)
{
    this->x = x;
    this->y = y;
    this->t = t;
}

SpatialTemporalPoint::SpatialTemporalPoint(const SpatialTemporalPoint &p)
{
    this->x = p.x;
    this->y = p.y;
    this->t = p.t;
}

void SpatialTemporalPoint::operator-=(const SpatialTemporalPoint &other)
{
    this->x -= other.x;
    this->y -= other.y;
    this->t -= other.t;
}

