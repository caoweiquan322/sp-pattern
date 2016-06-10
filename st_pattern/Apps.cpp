/* Copyright © 2015 DynamicFatty. All Rights Reserved. */

#include "Apps.h"
#include "SpatialTemporalException.h"
#include "DotsException.h"
#include "birch/CFTree.h"
#include "mainwindow.h"
#include "Trajectory.h"
#include "SpatialTemporalPoint.h"
#include <QException>
#include <vector>
#include <QDebug>
#include "RobustnessTester.h"
#include "Helper.h"
#include <QFile>
#include <QDataStream>
#include <QApplication>

const QString Apps::tinsSuffix(".tins");
const QString Apps::segSuffix(".seg");
const QString Apps::s2cSuffix(".s2c");
const QString Apps::clusterSuffix(".cluster");
const QString Apps::tincSuffix(".tinc");
const QString Apps::patternSuffix(".stp");

Apps::Apps()
{

}

Apps::~Apps()
{

}

void Apps::segmentTrajectories(const QString &fileDir, const QString &suffix,
                               const QString &outputFile,
                               double segStep, bool useTemporal, double minLength)
{
    // Retrieve all the files.
    QStringList files = Helper::retrieveFilesWithSuffix(fileDir, suffix);
    qDebug()<<"The folder "<<fileDir<<" contains "<<files.count()<<" trajectory file(s).";
    if (files.isEmpty())
    {
        return;
    }

    // Retrieve one file to estimate the reference point.
    SpatialTemporalPoint reference;
    try {
        Trajectory ref(files.first());
        //ref.validate();
        reference = ref.estimateReferencePoint();
    } catch (SpatialTemporalException &e) {
        qDebug()<<"Error occurs while estimating reference point. Details:\n"<<e.getMessage();
        return;
    } catch (...) {
        qDebug()<<"Unknown error occurs while estimating reference point.";
    }

    // Do segmentation.
    QFile segFile(outputFile + segSuffix), trajFile(outputFile + tinsSuffix);
    if (!segFile.open(QIODevice::WriteOnly)) {
        SpatialTemporalException(QString("Open file %1 error.").arg(segFile.fileName())).raise();
    }
    QDataStream segOut(&segFile);
    if (!trajFile.open(QIODevice::WriteOnly)) {
        segFile.close();
        SpatialTemporalException(QString("Open file %1 error.").arg(trajFile.fileName())).raise();
    }
    QDataStream trajOut(&trajFile);
    foreach (QString file, files) {
        try {
            qDebug()<<"Processing "<<file;
            Trajectory traj(file);
            // Preprocessing.
            traj.setReferencePoint(reference);
            traj.doMercatorProject();
            traj.validate();
            traj.doNormalize();
            // Do multi-threshold segmentation.
            QVector<Trajectory> subTrajs = traj.simplifyWithSEST(segStep, useTemporal);
            qDebug()<<"Used "<<subTrajs.count()<<" thresholds.";
            foreach (Trajectory sim, subTrajs) {
//                QString curveName = QString("M=%1").arg(sim.count());
//                sim.visualize(Qt::red, curveName);
                QVector<SegmentLocation> segments = sim.getSegmentsAsEuclidPoints();
                segments = filterSegments(segments, minLength);
                if (segments.isEmpty()) {
                    qDebug()<<"Segments become empty after filtered.";
                    continue;
                }
                // Serialize the trajectory and its segments.
                trajOut<<segments.count();
                foreach (SegmentLocation l, segments) {
                    segOut<<l;
                    trajOut<<l.id;
                }
            }
        } catch (SpatialTemporalException &e) {
            qDebug()<<"Error occurs while segmenting trajectory: "<<file<<"\nDetails: "
                   << e.getMessage();
        } catch (DotsException &e) {
            qDebug()<<"Error occurs while simplifying trajectory: "<<file<<"\nDetails: "
                   <<e.getMessage();
        } catch (...) {
            qDebug()<<"Unknown error occurs while segmenting trajectory: "<<file;
        }
    }
    segFile.close();
    trajFile.close();
}

QVector<SegmentLocation> Apps::filterSegments(const QVector<SegmentLocation> &segments, double minLength)
{
    QVector<SegmentLocation> filtered;
    foreach (SegmentLocation l, segments) {
        if (l.getLength() > minLength) {
            filtered.append(l);
        }
    }
    return filtered;
}

void Apps::testSegmentation()
{
    //QStringList geoLifeList = retrieveFilesWithSuffix("E:\\Geolife Trajectories 1.3", ".plt");
    //qDebug()<<"There are "<<geoLifeList.count()<<" GeoLife *.plt files.";

    QString filePath = "E:\\Geolife Trajectories 1.3\\Data\\000\\Trajectory\\20090703002800.plt";
    //filePath = "E:\\MyProjects\\QtCreator\\st-pattern\\test_files\\r6.txt";
    filePath = "/Users/fatty/Code/st-pattern/test_files/r6.txt";
    Trajectory traj(filePath);
    qDebug()<<"Trajectory start time: "<<traj.getStartTime();
    traj.setReferencePoint(traj.estimateReferencePoint());
    traj.doMercatorProject();
    traj.validate();
    traj.doNormalize();
    //traj.visualize(Qt::red, "Original trajectory");
    qDebug()<<"Original trajectory size: "<<traj.count();

    //RobustnessTester::testSegmentRobustness(traj, 5e7, 5e3, 0, 20, RobustnessTester::DOTS);
    QVector<int> sampleRates;
    sampleRates<<1<<3<<7<<11<<23;
    RobustnessTester::testMaximalStableSegmentation(traj, sampleRates, 100000);
}

void Apps::visualizeDataset(const QString &fileDir, const QString &suffix,
                            double range, const QString &patternFile)
{
    // Retrieve all the files.
    QStringList files = Helper::retrieveFilesWithSuffix(fileDir, suffix);
    qDebug()<<"The folder "<<fileDir<<" contains "<<files.count()<<" trajectory file(s).";
    if (files.isEmpty())
    {
        return;
    }

    // Retrieve one file to estimate the reference point.
    SpatialTemporalPoint reference;
    try {
        Trajectory ref(files.first());
        //ref.validate();
        reference = ref.estimateReferencePoint();
    } catch (SpatialTemporalException &e) {
        qDebug()<<"Error occurs while estimating reference point. Details:\n"<<e.getMessage();
        return;
    } catch (...) {
        qDebug()<<"Unknown error occurs while estimating reference point.";
    }

    MainWindow *figure = new MainWindow();
    QStringList colors;
    colors<<"r"<<"g"<<"b"<<"k"<<"c"<<"m"<<"y";
    int counter = 0;
    int visualizeNum = qMin(400, files.count());
    qDebug()<<"Visualize number is limited to "<<visualizeNum;
    foreach (QString file, files) {
        try {
            Trajectory traj(file);
            // Preprocessing.
            traj.setReferencePoint(reference);
            traj.doMercatorProject();
            traj.validate();
            traj.doNormalize();
            // Do multi-threshold segmentation.
            QVector<Trajectory> subTrajs = traj.simplifyWithSEST(1.6, true);
            QVector<SpatialTemporalPoint> points = subTrajs.last().getPoints();
            QVector<double> x,y;
            foreach (SpatialTemporalPoint p, points) {
                x<<p.x;
                y<<p.y;
            }
            figure->plot(x, y, colors[counter%colors.count()] + "--", "");//file.split("/").last());
            ++counter;
            if (counter > visualizeNum)
                break;
        } catch (SpatialTemporalException &e) {
            qDebug()<<"Error occurs while segmenting trajectory: "<<file<<"\nDetails: "
                   << e.getMessage();
        } catch (DotsException &e) {
            qDebug()<<"Error occurs while simplifying trajectory: "<<file<<"\nDetails: "
                   <<e.getMessage();
        } catch (...) {
            qDebug()<<"Unknown error occurs while segmenting trajectory: "<<file;
        }
    }
    figure->xRange(-range, range);
    figure->yRange(-range, range);
    figure->show();
    qApp->exec();
}

void Apps::clusterSegments(const QString &segmentsFile, const QVector<double> &weights,
                           const QString &outputFile, double thresh, int memoryLim)
{
    // Checking.
    if (weights.count() != 6) {
        SpatialTemporalException("We need a weights of exactly dimesion 6.").raise();
    }

    // Open file for scanning.
    QFile segFile(segmentsFile + segSuffix);
    if (!segFile.open(QIODevice::ReadOnly)) {
        SpatialTemporalException(QString("Open file %1 error.").arg(segFile.fileName())).raise();
    }
    QDataStream segIn(&segFile);
    QFile clusterFile(outputFile + clusterSuffix);
    if (!clusterFile.open(QIODevice::WriteOnly)) {
        segFile.close();
        SpatialTemporalException(QString("Open file %1 error.").arg(clusterFile.fileName())).raise();
    }
    QDataStream clusterOut(&clusterFile);
    QFile s2cFile(outputFile + s2cSuffix);
    if (!s2cFile.open(QIODevice::WriteOnly)) {
        segFile.close();
        clusterFile.close();
        SpatialTemporalException(QString("Open file %1 error.").arg(s2cFile.fileName())).raise();
    }
    QDataStream s2cOut(&s2cFile);

    // Do clustering.
    try {
        qDebug()<<"Running BIRCH with thresh: "<<thresh
               <<", memory limit: "<<memoryLim<<" bytes.";
        CFTreeND tree(thresh, memoryLim);
        // phase 1 and 2: building, compacting when overflows memory limit
        unsigned int numSegments = 0;
        {
            static const int DIM = CFTreeND::fdim;
            double item[DIM];
            int lid = 0;
            while (!segIn.atEnd()) {
                for (int i=0; i<DIM; ++i)
                {
                    segIn>>item[i];
                    item[i] *= weights.at(i);
                }
                segIn>>lid;
                ++numSegments;
                tree.insert(&item[0]);
            }
            qDebug()<<"#segments: "<<numSegments;
        }

        // phase 2 or 3: compacting? or clustering?
        // merging overlayed sub-clusters by rebuilding true
        tree.rebuild(memoryLim > 0);

        // phase 3: clustering sub-clusters using the existing clustering algorithm
        CFTreeND::cfentry_vec_type entries;
        tree.cluster(entries);
        {
            // Visualize the clusters.
            bool drawClusters = entries.size() < 400;
            qDebug()<<"#clusters: "<<entries.size();
            QStringList styles;
            styles<<"ro-"<<"gx-"<<"bd-"<<"m+-"<<"c*-"<<"k^-"<<"yv-";
            MainWindow *figure = NULL;
            if (drawClusters)figure = new MainWindow();
            for (unsigned int i=0; i<entries.size(); ++i)
            {
                double length = 0;
                double avg[CFTreeND::fdim];
                for (int j=0; j<CFTreeND::fdim; ++j) {
                    avg[j] = entries[i].sum[j]/entries[i].n/weights[j];
                    clusterOut<<avg[j];
                }
                clusterOut << i;
                length = qSqrt(avg[2]*avg[2]+avg[3]*avg[3]);
                qDebug()<<"Cluster "<<i<<" has "<<entries[i].n<<" segments. Average length: "<<length;
                QVector<double> _x, _y;
                _x<<avg[0]<<(avg[0]+avg[2]);
                _y<<avg[1]<<(avg[1]+avg[3]);
                if (drawClusters)
                    figure->plot(_x, _y, styles.at(i % styles.count()), "");//QString("CLS %1").arg(i)
            }
            qDebug()<<"#clusters: "<<entries.size();
            if (drawClusters) {
                double range = 40000; // 40 KM
                figure->xRange(-range, range);
                figure->yRange(-range, range);
                figure->show();
                qApp->exec();
            } else {
                qDebug("The clusters is too many to draw.");
            }
        }

        // phase 4: redistribution
        // @comment ts - it is also possible to another clustering algorithm hereafter
        //				for example, we have k initial points for k-means clustering algorithm
        //tree.redist_kmeans( items, entries, 0 );
        {
            std::vector<int> item_cids;
            static const int BUFFER_SIZE = 1024;
            static const int DIM = CFTreeND::fdim;
            segFile.seek(0);
            ItemND itemND;
            QVector<ItemND> buffer;
            QVector<int> segIds;
            int lid;
            int redistNum = 0;
            while (!segIn.atEnd()) {
                for (int i=0; i<DIM; ++i)
                {
                    segIn>>itemND[i];
                    itemND[i] *= weights.at(i);
                }
                buffer<<itemND;
                segIn>>lid;
                segIds<<lid;

                if (buffer.count() == BUFFER_SIZE || segIn.atEnd()) {
                    tree.redist( buffer.begin(), buffer.end(), entries, item_cids );
                    for( unsigned int i = 0 ; i < item_cids.size() ; i++ ) {
                        s2cOut<<segIds.at(i)<<item_cids[i];
                    }
                    redistNum += buffer.count();
                    qDebug()<<"Redist ("<<redistNum<<", "<<numSegments<<") of segments.";
                    item_cids.clear();
                    buffer.clear();
                    segIds.clear();
                }
            }
        }
        // Done.
    } catch (std::exception &e) {
        qDebug()<<"Failed to do clustering. Details:"<<e.what();
    }

    // Close files.
    segFile.close();
    clusterFile.close();
    s2cFile.close();

}

QVector<ItemND> Apps::random(ItemND _inf, ItemND _sup, int num)
{
    QVector<ItemND> data;
    //data.reserve(num);
    int dim = CFTreeND::fdim;
    ItemND item;
    for (int i=0; i<num; ++i)
    {
        for (int j=0; j<dim; ++j)
            item[j] = _inf[j] + (_sup[j]-_inf[j])*qrand()/((double)RAND_MAX);
        //data[i] = item;
        data.append(item);
    }
    return data;
}

void Apps::testCluster(double thresh, int memoryLim)
{
    // Prepare data.
    QVector<ItemND> allData;
    double _inf1[CFTreeND::fdim] = {-1.0, 1.0}, _sup1[CFTreeND::fdim] = {2.5, 3.0};
    allData += random(_inf1, _sup1, 100);
    double _inf2[CFTreeND::fdim] = {3.0, 2.5}, _sup2[CFTreeND::fdim] = {4.0, 5.0};
    allData += random(_inf2, _sup2, 200);

    // Do clustering.
    qDebug()<<"Running BIRCH with thresh: "<<thresh<<", memory limit: "<<memoryLim<<" bytes.";
    double birchThresh = thresh;
    CFTreeND tree(birchThresh, memoryLim);
    // phase 1 and 2: building, compacting when overflows memory limit
    foreach (ItemND item, allData) {
        tree.insert(&item[0]);
    }
    // phase 2 or 3: compacting? or clustering?
    // merging overlayed sub-clusters by rebuilding true
    tree.rebuild(memoryLim > 0);

    // phase 3: clustering sub-clusters using the existing clustering algorithm
    CFTreeND::cfentry_vec_type entries;
    tree.cluster(entries);
    qDebug()<<"Number of entries: "<<entries.size();

    // phase 4: redistribution
    // @comment ts - it is also possible to another clustering algorithm hereafter
    //				for example, we have k initial points for k-means clustering algorithm
    //tree.redist_kmeans( items, entries, 0 );
    std::vector<int> item_cids;
    tree.redist( allData.begin(), allData.end(), entries, item_cids );
    for( unsigned int i = 0 ; i < item_cids.size() ; i++ )
        allData[i].cid() = item_cids[i];
    //print_items( argc >=4 ? argv[3] : "item_cid.txt" , items);

    // Visualize the data.
    QStringList styles;
    styles<<"ro"<<"gx"<<"bd"<<"m+"<<"c*"<<"k^"<<"yv";
    MainWindow *figure = new MainWindow();
    for (unsigned int i=0; i<entries.size(); ++i)
    {
        QVector<double> _x, _y;
        foreach (ItemND item, allData) {
            if (item.cid() == i) {
                _x.append(item[0]);
                _y.append(item[1]);
            }
        }
        figure->plot(_x, _y, styles.at(i % styles.count()), QString("CLS %1").arg(i));
    }
    figure->show();
    qApp->exec();
}

void Apps::transTrajectories(const QString &tins, const QString &s2c,
                             const QString &tinc)
{
    // Open file for scanning.
    qDebug()<<"Merging "<<(tins+tinsSuffix)<<" and "<<(s2c + s2cSuffix)<<" into:\n"
           <<(tinc+tincSuffix);
    QFile tinsFile(tins + tinsSuffix);
    if (!tinsFile.open(QIODevice::ReadOnly)) {
        SpatialTemporalException(QString("Open file %1 error.").arg(tinsFile.fileName())).raise();
    }
    QDataStream tinsIn(&tinsFile);
    QFile s2cFile(s2c + s2cSuffix);
    if (!s2cFile.open(QIODevice::ReadOnly)) {
        tinsFile.close();
        SpatialTemporalException(QString("Open file %1 error.").arg(s2cFile.fileName())).raise();
    }
    QDataStream s2cIn(&s2cFile);
    QFile tincFile(tinc + tincSuffix);
    if (!tincFile.open(QIODevice::WriteOnly)) {
        tinsFile.close();
        s2cFile.close();
        SpatialTemporalException(QString("Open file %1 error.").arg(tincFile.fileName())).raise();
    }
    QDataStream tincOut(&tincFile);

    // Do translate. Assume that the tins/s2c were stored in strict order.
    int numSeg = 0;
    unsigned int segId1, segId2, clusterId;
    QVector<unsigned int> clusterIds;
    while (!tinsIn.atEnd()) {
        tinsIn>>numSeg;
        clusterIds.clear();
        while ((--numSeg) >= 0 && !tinsIn.atEnd()) {
            tinsIn>>segId1;
            s2cIn>>segId2>>clusterId;
            if (segId1 != segId2) {
                qDebug()<<"The tins file and s2c file are not strictly formated with order.";
                break;
            }
            // We only store unique cluster ids for each trajectory. Any two consecutive regions will
            // not match exactly.
            if (clusterId != clusterIds.last() || clusterIds.isEmpty()) {
                clusterIds<<clusterId;
            }
        }
        tincOut<<clusterIds.count();
        foreach (unsigned int id, clusterIds) {
            tincOut<<id;
        }
        if (numSeg >= 0) {
            qDebug()<<"Malformed tins file: "<<(tins+tinsSuffix);
            break;
        }
    }

    // Close files.
    tinsFile.close();
    s2cFile.close();
    tincFile.close();
}

void Apps::scpm(const QString &clusterFileName, const QString &tincFileName,
                const QString &outputFileName, double continuityRadius, int minSup,
                int minLen)
{
    QVector<SegmentLocation> clusters = retrieveClusters(clusterFileName + clusterSuffix);
    QHash<unsigned int, QVector<unsigned int> > scMap =
            getSpatialContinuityMap(clusters, continuityRadius);
    QVector<QVector<unsigned int> > tinc = retrieveTinC(tincFileName + tincSuffix);
    QVector<QVector<unsigned int> > allPatterns;
    QVector<int> projsFrom;
    for (int i=0; i<tinc.count(); ++i)
        projsFrom<<0;

    prefixSpan(QVector<unsigned int>(),
               tinc,
               projsFrom,
               scMap,
               allPatterns,
               minSup);
    qDebug()<<"Totally "<<allPatterns.count()<<" patterns were found.";
    storePatterns(allPatterns, clusters, outputFileName);
    visualizePatterns(allPatterns, clusters, minLen);
}

void Apps::storePatterns(const QVector<QVector<unsigned int> > &allPatterns,
                         const QVector<SegmentLocation> &clusters,
                         const QString &patternFileName)
{
    QFile patternFile(patternFileName + patternSuffix);
    if (!patternFile.open(QIODevice::WriteOnly)) {
        SpatialTemporalException(QString("Open st-pattern file %1 error.").arg(patternFileName)).raise();
    }
    QDataStream patternOut(&patternFile);
    foreach (QVector<unsigned int> pattern, allPatterns) {
        patternOut << pattern.count();
        foreach (unsigned int id, pattern) {
            patternOut << clusters[id];
        }
    }
    patternFile.close();
}

void Apps::visualizePatterns(const QVector<QVector<unsigned int> > &allPatterns,
                             const QVector<SegmentLocation> &clusters,
                             int minLen)
{
    MainWindow *figure = new MainWindow();
    QStringList styles;
    styles<<"ro--"<<"gx--"<<"bd--"<<"m+--"<<"c*--"<<"k^--"<<"yv--";
    SegmentLocation l;
    int counter = 0;
    foreach (QVector<unsigned int> pattern, allPatterns) {
        QVector<double> _x, _y;
        if (pattern.count() < minLen)
            continue;
        foreach (unsigned int id, pattern) {
            l = clusters.at(id);
            _x << l.x << (l.x+l.rx);
            _y << l.y << (l.y+l.ry);
        }
        figure->plot(_x, _y, styles.at(counter % styles.count()));
        ++counter;
        qDebug()<<"Pattern "<<counter<<" has "<<pattern.count()<<" regions.";
    }
//    double range = 5000;
//    figure->xRange(-range, range);
//    figure->yRange(-range, range);
    figure->show();
    qApp->exec();
}

void Apps::prefixSpan(const QVector<unsigned int> &currPrefix,
                      const QVector<QVector<unsigned int> > &projs,
                      const QVector<int> &projsFrom,
                      const QHash<unsigned int, QVector<unsigned int> > scMap,
                      QVector<QVector<unsigned int> > &allPatterns,
                      int minSup)
{
    if (projs.count() < minSup)
        return;
    // Specify items to check.
    QVector<unsigned int> toCheck;
    if (currPrefix.isEmpty()) {
        toCheck = scMap.keys().toVector();
    } else {
        toCheck = scMap[currPrefix.last()];
    }
    //qDebug()<<"Prefix: "<<currPrefix<<", tocheck: "<<toCheck;
    if (toCheck.isEmpty())
        return;
    //qDebug()<<"Prefix: "<<currPrefix<<", tocheck: "<<toCheck;
    // Find frequent items from projections.
    QVector<unsigned int> freq;
    foreach (unsigned int c, toCheck) {
        int counter = 0;
        for (int i=0; i<projs.count(); ++i) {
            if (projs[i].indexOf(c, projsFrom[i]) >= 0)
                ++counter;
        }
        if (counter >= minSup)
            freq << c;
    }
    if (freq.isEmpty())
        return;
    //qDebug()<<"Prefix: "<<currPrefix<<", freq: "<<freq;
    // Store patterns and invoke PrefixSpan recursively.
    foreach (unsigned int c, freq) {
        QVector<unsigned int> newPrefix = currPrefix;
        newPrefix.append(c);
        //allPatterns <<  newPrefix;
        int beforePatternsCount = allPatterns.count();
        QVector<QVector<unsigned int> > newProjs;
        QVector<int> newProjsFrom;
        for (int i=0; i<projs.count(); ++i) {
            int idx = projs[i].indexOf(c, projsFrom[i]);
            if (idx >= 0 && idx < projs[i].count()-1) {
                newProjs << projs[i];
                newProjsFrom << (idx+1);
            }
        }
        //qDebug()<<"New projs and new from for"<<c<<" is "<<newProjs<<", "<<newProjsFrom;
        if (newProjs.count() >= minSup) {
            prefixSpan(newPrefix, newProjs, newProjsFrom, scMap, allPatterns, minSup);
        }
        // Check if this is a leaf node of the prefix-span tree. However we will construct
        // a (suffix) trie to solve this problem.
        if (true) {//beforePatternsCount == allPatterns.count()) {
            allPatterns << newPrefix;
        }
    }
}

void Apps::testPrefixSpan()
{
    // Data preparation.
    QVector<QVector<unsigned int> > tinc;
    QVector<unsigned int> t1, t2;
    t1<<1<<2<<3<<4<<5;
    t2<<1<<4<<5;
    tinc<<t1<<t2;
    qDebug()<<"T1: "<<t1;
    qDebug()<<"T2: "<<t2;
    QVector<QVector<unsigned int> > allPatterns;
    QVector<int> projsFrom;
    for (int i=0; i<tinc.count(); ++i)
        projsFrom<<0;
    QHash<unsigned int, QVector<unsigned int> > scMap;
    QVector<unsigned int> n1, n2, n3, n4, n5;
    n1<<2<<3<<4;
    n2<<3<<4;
    n3<<4;
    n4<<5;
    scMap[1] = n1;
    scMap[2] = n2;
    scMap[3] = n3;
    scMap[4] = n4;
    scMap[5] = n5;
    qDebug()<<"Spatial continuity map: "<<scMap;

    // Evaluation.
    prefixSpan(QVector<unsigned int>(),
               tinc,
               projsFrom,
               scMap,
               allPatterns,
               2);
    qDebug()<<"All patterns are list as below:";
    foreach (QVector<unsigned int> p, allPatterns) {
        qDebug()<<p;
    }
}

QVector<SegmentLocation> Apps::retrieveClusters(const QString &clusterFileName)
{
    // Open file.
    QFile clusterFile(clusterFileName);
    if (!clusterFile.open(QIODevice::ReadOnly)) {
        SpatialTemporalException(QString("Open cluster file %1 error.").arg(clusterFileName)).raise();
    }
    QDataStream clusterIn(&clusterFile);
    QVector<SegmentLocation> clusters;
    SegmentLocation l;
    while (!clusterIn.atEnd()) {
        clusterIn >> l;
        clusters << l;
    }

    // Close file.
    clusterFile.close();

    // Return.
    return clusters;
}

QHash<unsigned int, QVector<unsigned int> > Apps::getSpatialContinuityMap(
        const QVector<SegmentLocation> &clusters, double continuityRadius)
{
    QHash<unsigned int, QVector<unsigned int> > scMap;
    foreach (SegmentLocation l1, clusters) {
        QVector<unsigned int> nb;
        foreach (SegmentLocation l2, clusters) {
            if (l1.id != l2.id) {
                double diffX = l2.x-l1.x-l1.rx;
                double diffY = l2.y-l1.y-l1.ry;
                if (qSqrt(diffX*diffX+diffY*diffY) < continuityRadius)
                    nb << l2.id;
            }
        }
        scMap[l1.id] = nb;
    }
    return scMap;
}

QVector<QVector<unsigned int> > Apps::retrieveTinC(const QString &tincFileName)
{
    // Open file.
    QFile tincFile(tincFileName);
    if (!tincFile.open(QIODevice::ReadOnly)) {
        SpatialTemporalException(QString("Open tinc file %1 error.").arg(tincFileName)).raise();
    }
    QDataStream tincIn(&tincFile);
    QVector<QVector<unsigned int> > tincs;
    QVector<unsigned int> traj;
    int numClusters;
    unsigned int cID;
    while (!tincIn.atEnd()) {
        tincIn >> numClusters;
        while ((--numClusters)>=0 && !tincIn.atEnd()) {
            tincIn >> cID;
            traj << cID;
        }
        if (numClusters >= 0) {
            SpatialTemporalException("Malformed tinc file.").raise();
        }
        tincs << traj;
        traj.clear();
    }

    // Close file.
    tincFile.close();

    // Return.
    return tincs;
}

void Apps::testTrie()
{
    // Prepare old patterns.
    QVector<QVector<unsigned int> > oldPatterns;
    unsigned int from = 1, to = 6;
    for (unsigned int i=from; i<to; ++i)
        for (unsigned int j=i; j<to; ++j)
        {
            QVector<unsigned int> pattern;
            for (unsigned int k=i; k<=j; ++k)
                pattern.append(k);
            oldPatterns.append(pattern);
        }
    oldPatterns = randomOrder(oldPatterns);

    // Show old patterns.
    foreach (QVector<unsigned int> pattern, oldPatterns) {
        qDebug()<<pattern;
    }

    // Clean the patterns.
    QVector<QVector<unsigned int> > newPatterns = cleanShortPatterns(oldPatterns);

    // Show new patterns.
    qDebug()<<"New patterns after cleaned:";
    foreach (QVector<unsigned int> pattern, newPatterns) {
        qDebug()<<pattern;
    }
}

