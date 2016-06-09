/* Copyright © 2015 DynamicFatty. All Rights Reserved. */

#ifndef APPS_H
#define APPS_H

#include <QString>
#include <QVector>
#include "birch/CFTree.h"
#include <algorithm>
#include "SpatialTemporalSegment.h"

// The CF tree of specified dimension.
typedef CFTree<6> CFTreeND;

struct ItemND
{
    ItemND() : id(0) { std::fill( item, item + sizeof(item)/sizeof(item[0]), 0 ); }
    ItemND( double* in_item ) : id(0) { std::copy(in_item, in_item+sizeof(item)/sizeof(item[0]), item); }
    double& operator[]( int i ) { return item[i]; }
    double operator[]( int i ) const { return item[i]; }
    std::size_t size() const { return sizeof(item)/sizeof(item[0]); }

    int& cid() { return id; }
    const int cid() const { return id; }

    double item[CFTreeND::fdim];
    int id;
};

class Apps
{
protected:
    // Prevent the Apps from being instanced.
    Apps();
    ~Apps();

public:

    // The segmentation phase.
    static void segmentTrajectories(const QString &fileDir, const QString &suffix,
                                    const QString &outputFile,
                                    double segStep, bool useTemporal, double minLength);
    static QVector<SegmentLocation> filterSegments(const QVector<SegmentLocation> &segments, double minLength);
    static void testSegmentation();

    // The visualization.
    static void visualizeDataset(const QString &fileDir, const QString &suffix, double range,
                                 const QString &patternFile="");

    // The clustering phase.
    static void clusterSegments(const QString &segmentsFile, const QVector<double> &weights,
                                const QString &outputFile, double thresh, int memoryLim);
    static QVector<ItemND> random(ItemND _inf, ItemND _sup,
                                  int num);
    static void testCluster(double thresh, int memoryLim = 0);

    // The translate phase.
    static void transTrajectories(const QString &tins, const QString &s2c,
                                  const QString &tinc);

    // The SCPM mining phase.
    static void scpm(const QString &clusterFileName, const QString &tincFileName,
                     const QString &outputFileName, double continuityRadius, int minSup,
                     int minLen);
    static void storePatterns(const QVector<QVector<unsigned int> > &allPatterns,
                              const QVector<SegmentLocation> &clusters,
                              const QString &patternFileName);
    static void visualizePatterns(const QVector<QVector<unsigned int> > &allPatterns,
                                  const QVector<SegmentLocation> &clusters,
                                  int minLen);
    static void prefixSpan(const QVector<unsigned int> &currPrefix,
                           const QVector<QVector<unsigned int> > &projs,
                           const QVector<int> &projsFrom,
                           const QHash<unsigned int, QVector<unsigned int> > scMap,
                           QVector<QVector<unsigned int> > &allPatterns,
                           int minSup);
    static void testPrefixSpan();
    static QVector<SegmentLocation> retrieveClusters(const QString &clusterFileName);
    static QHash<unsigned int, QVector<unsigned int> > getSpatialContinuityMap(
            const QVector<SegmentLocation> &clusters, double continuityRadius);
    static QVector<QVector<unsigned int> > retrieveTinC(const QString &tincFileName);

public:
    static const QString tinsSuffix;
    static const QString segSuffix;
    static const QString s2cSuffix;
    static const QString clusterSuffix;
    static const QString tincSuffix;
    static const QString patternSuffix;
};

#endif // APPS_H
