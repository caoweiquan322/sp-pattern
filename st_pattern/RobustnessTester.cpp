/* This software is developed by caoweiquan322 OR DynamicFatty.
 * All rights reserved.
 *
 * Author: caoweiquan322
 */

#include "RobustnessTester.h"
#include "SpatialTemporalException.h"
#include <QDebug>
#include "DotsSimplifier.h"
#include "DotsSimplifier.h"
#include <QtMath>

RobustnessTester::RobustnessTester()
{

}

void RobustnessTester::testDouglasPeuckerAgainstSampleRates()
{

}

double RobustnessTester::testSegmentRobustness(const Trajectory &traj,
                                               double para1, double para2,
                                               double delay1, double delay2,
                                               RobustnessTester::SegmentationMethod method)
{
    //Trajectory traj1, traj2;
    Trajectory traj1;
    Trajectory traj2;

    switch (method) {
    case DOTS:
        traj1 = traj.simplify(para1, false);
        traj2 = traj.simplify(para2, false);
        traj1.visualize(Qt::green, "Traj1");
        break;
    case DOTS_CASCADE:
        traj1 = traj.simplify(para1, true);
        traj2 = traj.simplify(para2, true);
        traj1.visualize(Qt::green, "Traj1");
        break;
    case DP:
//        QVector<double> _x, _y, _t;
//        foreach (SpatialTemporalPoint p, points) {
//            _x.append(p.x);
//            _y.append(p.y);
//            _t.append(p.t);
//        }

//        QVector<int> indices;
//        DotsSimplifier::batchDotsCascadeByIndex(_x, _y, _t, indices, threshold);
//        Trajectory traj(*this);
//        Helper::slice<SpatialTemporalPoint>(this->points, indices, traj.points);
//        return traj;
        break;
    default:
        SpatialTemporalException("Unsupported segmentation method.").raise();
        break;
    }
    qDebug()<<"Trajectory 1 size is: "<<traj1.count();
    qDebug()<<"Trajectory 2 size is: "<<traj2.count();
    return 0.0;
}

void RobustnessTester::testMaximalStableSegmentation(const Trajectory &traj, QVector<int> sampleRates,
                                                     double finalLssdThreshold)
{
    // Sample the original trajectory and do maximal stable segmentation.
    QVector<QVector<SegmentationResult> > allResults;
    foreach (int r, sampleRates) {
        qDebug()<<"Do maximal stable segmentation for sample rate: "<<r;
        Trajectory sub = traj.sample(r);
        allResults.append(maximalStableSegmentations(sub, finalLssdThreshold));
    }
    // Evaluate the results.
    int numRates = sampleRates.count();
    for (int i=0; i<numRates; ++i)
        for (int j=i+1; j<numRates; ++j)
        {
            qDebug()<<"Similarity between "<<sampleRates.at(i)<<" and "<<sampleRates.at(j)<<":";
            QVector<SegmentationResult> rstI = allResults.at(i);
            QVector<SegmentationResult> rstJ = allResults.at(j);
            compareSegmentationResults(rstI, rstJ);
        }
}

QVector<SegmentationResult> RobustnessTester::maximalStableSegmentations(const Trajectory &traj,
                                                                         double finalLssdThreshold)
{
    // Prepare the data to process.
    QVector<SpatialTemporalPoint> points = traj.getPoints();
    QVector<double> x,y,t;
    foreach (SpatialTemporalPoint p, points) {
        x.append(p.x);
        y.append(p.y);
        t.append(p.t);
    }

    // Clear output explicitly.
    QVector<int> simplifiedIndex;
    simplifiedIndex.clear();

    // Construct cascade simplifier.
    double lssdThreshold = finalLssdThreshold;
    const double DEFAULT_START_THRESHOLD = 100;// Start by 100 meters as a threshold by default.
    const double DEFAULT_CASCADE_STEP = 1.3;
    double startThreshold = qMin(DEFAULT_START_THRESHOLD, lssdThreshold/8.0);
    int cascadeCount = qFloor(qLn(lssdThreshold/startThreshold)/qLn(DEFAULT_CASCADE_STEP))+1;
    double k = qPow(lssdThreshold/startThreshold, 1.0/(cascadeCount-1));
    double th = startThreshold;
    QObject *root = new QObject();
    QVector<DotsSimplifier *> cascade;
    for (int i=0; i<cascadeCount; ++i)
    {
        DotsSimplifier *s = new DotsSimplifier(root, i==0 ? NULL : cascade[0]);
        s->setParameters(th, k);
        th*=k;
        cascade.append(s);
    }

    // Run DOTS in cascade manner.
    int pointCount = x.count();
    double px, py, pt;
    DotsSimplifier *first = cascade[0];
    for (int i=0; i<pointCount; ++i)
    {
        px = x.at(i);
        py = y.at(i);
        pt = t.at(i);
        first->feedData(px, py, pt);
        int index = -1;
        if (first->readOutputIndex(index))
        {
            bool gotOutput = true;
            for (int j=1; j<cascadeCount; ++j)
            {
                DotsSimplifier *s = cascade[j];
                s->feedIndex(index);
                if (!(s->readOutputIndex(index)))
                {
                    gotOutput = false;
                    break;
                }
            }
            if (gotOutput)
            {
                simplifiedIndex.append(index);
            }
        }
    }

    // Finish simplifiers from front to end.
    for (int i=0; i<cascadeCount-1; ++i)
    {
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
    while (last->readOutputIndex(index))
    {
        simplifiedIndex.append(index);
    }

    // Get the output.
    //qDebug()<<"AvgSED, LSSD thresh, MAX LSSD";
    QVector<double> icrs;// Incremental compression rates.
    foreach (DotsSimplifier *s, cascade) {
        //qDebug()<<s->getLssdThreshold()<<", "<<(s->getInputCount()/1.0/(s->getOutputCount()));
        icrs.append((s->getInputCount())/1.0/(s->getOutputCount()));
    }
    QVector<SegmentationResult> stableSegmentations;
    for (int i=0; i<icrs.count(); ++i)
    {
        qDebug()<<"Output size: "<<(cascade[i]->getOutputCount());
        bool isStable = false;
        if (i>0 && i<icrs.count()-1) {
            if (icrs[i] <= icrs[i-1] && icrs[i] <= icrs[i+1]) {
                qDebug()<<"Output size: "<<(cascade[i]->getOutputCount())<<"    *";
                isStable = true;
            } else {
                qDebug()<<"Output size: "<<(cascade[i]->getOutputCount());
            }
        }

        // Store the results.
        if (isStable) {
            DotsSimplifier *s = cascade[i];
            QVector<int> indices;
            for (int j=0; j<s->getOutputCount(); ++j)
                indices.append(s->getSimplifiedIndex(j));
            QVector<SegmentLocation> segments = traj.slice(indices).getSegmentsAsEuclidPoints();
            SegmentationResult result;
            result.parameter = s->getLssdThreshold();
            result.avgSED = s->getAverageSED();
            result.maxLSSD = s->getMaxLSSD();
            result.compressionRate = (traj.count())/1.0/(s->getOutputCount());
            result.segments = segments;
            result.isStable = isStable;
            stableSegmentations.append(result);
        }
    }

    // Destroy the total cascade.
    delete root;
    return stableSegmentations;
}

void RobustnessTester::compareSegmentationResults(QVector<SegmentationResult> resultA, QVector<SegmentationResult> resultB)
{
    QVector<double> divs;
    divs<<0.1895<<0.1482<<0.1197<<0.0869<<0.0674<<0.0634<<0.0747<<0.0756<<0.057;
    QString title = QString().sprintf("%.2f,", 0.0);
    foreach (SegmentationResult r, resultB) {
        if (r.isStable)
            title += QString().sprintf("%.2f*,", r.parameter);
        else
            title += QString().sprintf("%.2f,", r.parameter);
    }
    qDebug("%s", title.toStdString().c_str());

    // Display the details.
    SegmentLocation weightSquare;
//    weightSquare.x = 1e-4;
//    weightSquare.y = 1e-4;
//    weightSquare.theta = 1e-2;
//    weightSquare.scale = 1e-4;
//    weightSquare.start = 0.0;
//    weightSquare.duration = 0.0;
    weightSquare.x = 0;
    weightSquare.y = 0;
    weightSquare.theta = 0;
    weightSquare.scale = 0;
    weightSquare.start = 1.0;
    weightSquare.duration = 1.0;
    double ZERO = 0.0;
    double VERY_LARGE = 1.0/ZERO;
    int divIdx = 0;
    foreach (SegmentationResult rA, resultA) {
        QString line;
        if (rA.isStable)
            line = QString().sprintf("%.2f*,", rA.parameter);
        else
            line = QString().sprintf("%.2f,", rA.parameter);
        foreach (SegmentationResult rB, resultB) {
            double err = 0;
            foreach (SegmentLocation lA, rA.segments) {
                double minDist = VERY_LARGE;
                double dist = 0.0;
                foreach (SegmentLocation lB, rB.segments) {
                    dist = lA.distance(lB, weightSquare);
                    if (minDist > dist)
                        minDist = dist;
                }
                err += minDist;
            }
            line += QString().sprintf("%.4f,", err/divs[divIdx]);
        }
        ++divIdx;
        qDebug("%s", line.toStdString().c_str());
    }
    qDebug()<<"";
}

