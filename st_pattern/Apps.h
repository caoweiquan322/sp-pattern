/* Copyright © 2015 DynamicFatty. All Rights Reserved. */

#ifndef APPS_H
#define APPS_H

#include <QString>
#include <QVector>
#include "birch/CFTree.h"
#include <algorithm>

// The CF tree of specified dimension.
typedef CFTree<2> CFTreeND;

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
    static void segmentTrajectories(const QString& fileDir, const QString &suffix,
                                    const QString &outputFile);
    static void testSegmentation();

    // The clustering phase.
    static void clusterSegments(const QString &segmentsFile, const QVector<double> &weights,
                                const QString &outputFile);
    static QVector<ItemND> random(ItemND _inf, ItemND _sup,
                                  int num);
    static void testCluster();
};

#endif // APPS_H
