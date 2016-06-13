/* Copyright © 2015 DynamicFatty. All Rights Reserved. */

#ifndef APPS_H
#define APPS_H

#include <QString>
#include <QVector>
#include <QSet>
#include "birch/CFTree.h"
#include <algorithm>
#include "SpatialTemporalSegment.h"
#include "TrieNode.h"

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
                                    double segStep, bool useTemporal, double minLength, bool useSEST, double dotsTh);
    static QVector<SegmentLocation> filterSegments(const QVector<SegmentLocation> &segments, double minLength);
    static void testSegmentation();

    // The visualization.
    static void visualizeDataset(const QString &fileDir, const QString &suffix, double range,
                                 const QString &patternFile="");

    // The clustering phase.
    static void clusterSegments(const QString &segmentsFile, const QVector<double> &weights,
                                const QString &outputFile, double thresh, int memoryLim);
    static void myRedist(const CFTreeND::cfentry_vec_type &entries,
                         const QVector<ItemND> &buffer,
                         std::vector<int> &item_cids);
    static QVector<ItemND> random(ItemND _inf, ItemND _sup,
                                  int num);
    static void testCluster(double thresh, int memoryLim = 0);

    // The translate phase.
    static void transTrajectories(const QString &tins, const QString &s2c,
                                  const QString &tinc);
    static void storeTinCToTxt(const QVector<QVector<unsigned int> > &allTinC,
                               const QString &filePath);
    static const QVector<QVector<unsigned int> > retrieveTinCFromTxt(const QString &filePath);
    static void visualizePatternsFromSPMF(const QString &patterFileName,
                                          const QString &clusterFileName, int minLen);

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

    // Remove SUFFIX/PREFIX pattern.
    static void testTrie();

    // Return a random order version of the original oldVec.
    template<typename T>
    static QVector<T> randomOrder(const QVector<T> &oldVec) {
        QVector<T> vec = oldVec;
        for (int i=0; i<vec.count(); ++i) {
            int r = i+ qrand()%(vec.count()-i);
            T x = vec[i];
            vec[i] = vec[r];
            vec[r] = x;
        }
        return vec;
    }

    // Clean the patterns so that only those that do not belong to others would keep left.
    template<typename T>
    static QVector<QVector<T> > cleanShortPatterns(
            const QVector<QVector<T> > &patterns) {
        QVector<QVector<T> > nonPrefixPatterns = cleanPrefixPatterns(patterns);
        QVector<QVector<T> > inversed = inversePatterns(nonPrefixPatterns);
        inversed = cleanPrefixPatterns(inversed);
        return inversePatterns(inversed);
    }

    // Clean the patterns so that those which are prefix of others are totally removed.
    template<typename T>
    static QVector<QVector<T> > cleanPrefixPatterns(
            const QVector<QVector<T> > &patterns) {
        TrieNode<T> *root = new TrieNode<T>();
        foreach (QVector<T> pattern, patterns) {
            root->insert(pattern, 0);
        }
        QVector<QVector<T> > newPatterns;
        root->collectLeaves(newPatterns);
        delete root;

        return newPatterns;
    }

    // Inverse each element of the patterns.
    template<typename T>
    static QVector<QVector<T> > inversePatterns(
            const QVector<QVector<T> > &patterns) {
        QVector<QVector<T> > inversed;
        foreach (QVector<T> pattern, patterns) {
            QVector<T> ipattern;
            for (int i=pattern.count()-1; i>=0; --i)
                ipattern.append(pattern.at(i));
            inversed.append(ipattern);
        }
        return inversed;
    }

    // Generate synthetic data.
public:
    static void generateDataSet(const QString &originalDataPath,
                                const QString &strNoiseLevel,
                                const QString &strSampleInterval,
                                const QString &outputDir);
private:
    static void _generateDataSet(const QString &originalDataPath,
                                const QVector<double> &noiseLevel,
                                const QVector<double> &sampleInterval,
                                const QString &outputDir);

public:
    static const QString tinsSuffix;
    static const QString segSuffix;
    static const QString s2cSuffix;
    static const QString clusterSuffix;
    static const QString tincSuffix;
    static const QString patternSuffix;
};

#endif // APPS_H
