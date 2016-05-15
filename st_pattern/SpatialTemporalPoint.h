/* This software is developed by caoweiquan322 OR DynamicFatty.
 * All rights reserved.
 *
 * Author: caoweiquan322
 */

#ifndef SPATIALTEMPORALPOINT_H
#define SPATIALTEMPORALPOINT_H

class SpatialTemporalPoint
{
public:
    explicit SpatialTemporalPoint(double x = 0, double y = 0, double t = 0);
    SpatialTemporalPoint(const SpatialTemporalPoint& p);
    void operator-= (const SpatialTemporalPoint& other);

public:
    double x, y, t;
};

#endif // SPATIALTEMPORALPOINT_H
