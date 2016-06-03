/* This software is developed by caoweiquan322 OR DynamicFatty.
 * All rights reserved.
 *
 * Author: caoweiquan322
 */

#ifndef ROBUSTNESSTESTER_H
#define ROBUSTNESSTESTER_H

#include "Trajectory.h"
#include <QVector>
#include "SpatialTemporalSegment.h"

struct SegmentationResult{
    double parameter;
    double avgSED;
    //double avgPD;
    double maxLSSD;
    double compressionRate;
    QVector<SegmentLocation> segments;
    bool isStable;
};


class RobustnessTester
{
public:
    RobustnessTester();
    enum SegmentationMethod {
        DOTS = 0,
        DOTS_CASCADE,
        DP,
        PERSISTENCE,
        SQUISH,
        UNKNOWN
    };

public:
    static void testDouglasPeuckerAgainstSampleRates();
    static double testSegmentRobustness(const Trajectory &traj,
                                        double para1, double para2,
                                        double delay1, double delay2,
                                        SegmentationMethod method);

    static void testMaximalStableSegmentation(const Trajectory &traj, QVector<int> sampleRates,
                                              double finalLssdThreshold);

protected:
    static QVector<SegmentationResult> maximalStableSegmentations(const Trajectory &traj,
                                                                  double finalLssdThreshold);
    static void compareSegmentationResults(QVector<SegmentationResult> resultA,
                                           QVector<SegmentationResult> resultB);
};

#endif // ROBUSTNESSTESTER_H
