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

Apps::Apps()
{

}

Apps::~Apps()
{

}

void Apps::segmentTrajectories(const QString &fileDir, const QString &suffix,
                               const QString &outputFile)
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
        ref.validate();
        reference = ref.estimateReferencePoint();
    } catch (SpatialTemporalException &e) {
        qDebug()<<"Error occurs while estimating reference point. Details:\n"<<e.getMessage();
        return;
    } catch (...) {
        qDebug()<<"Unknown error occurs while estimating reference point.";
    }

    // Do segmentation.
    QFile segFile(outputFile + ".seg"), trajFile(outputFile + ".traj");
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
            Trajectory traj(file);
            traj.validate();
            traj.setReferencePoint(reference);
            traj.doMercatorProject();
            traj.doNormalize();
            traj = traj.simplify(5e6);
            QVector<SegmentLocation> segments = traj.getSegmentsAsEuclidPoints();
            qDebug()<<"Trajectory "<<file<<" contains "<<segments.count()<<" segments";

            // Serialize the trajectory and its segments.
            trajOut<<segments.count();
            foreach (SegmentLocation l, segments) {
                segOut<<l;
                trajOut<<l.id;
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
    traj.doNormalize();
    //traj.visualize(Qt::red, "Original trajectory");
    qDebug()<<"Original trajectory size: "<<traj.count();

    //RobustnessTester::testSegmentRobustness(traj, 5e7, 5e3, 0, 20, RobustnessTester::DOTS);
    QVector<int> sampleRates;
    sampleRates<<1<<3<<7<<11<<23;
    RobustnessTester::testMaximalStableSegmentation(traj, sampleRates, 100000);
}

void Apps::clusterSegments(const QString &segmentsFile, const QVector<double> &weights,
                           const QString &outputFile)
{
    SpatialTemporalException("Apps::clusterSegments() is not implemented yet.").raise();
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

void Apps::testCluster()
{
    // Prepare data.
    QVector<ItemND> allData;
    double _inf1[CFTreeND::fdim] = {-1.0, 1.0}, _sup1[CFTreeND::fdim] = {2.5, 3.0};
    allData += random(_inf1, _sup1, 100);
    double _inf2[CFTreeND::fdim] = {1.0, 2.5}, _sup2[CFTreeND::fdim] = {4.0, 5.0};
    allData += random(_inf2, _sup2, 200);

    // Do clustering.
    double birchThresh = 2.0;
    int memoryLimit = 1000000;
    CFTreeND tree(birchThresh, memoryLimit);
    // phase 1 and 2: building, compacting when overflows memory limit
    foreach (ItemND item, allData) {
        tree.insert(&item[0]);
    }
    // phase 2 or 3: compacting? or clustering?
    // merging overlayed sub-clusters by rebuilding true
    tree.rebuild(memoryLimit > 0);

    // phase 3: clustering sub-clusters using the existing clustering algorithm
    CFTreeND::cfentry_vec_type entries;
    tree.cluster(entries);
    qDebug()<<"Number of entries: "<<entries.size();
    for (std::size_t i = 0; i<entries.size(); ++i) {
        qDebug()<<"Entry "<<i<<": "<<entries[i].sum[0]/entries[i].n<<", "<<entries[i].sum[1]/entries[i].n;
        qDebug()<<"Size is: "<<entries[i].n;
    }

    // phase 4: redistribution
    // @comment ts - it is also possible to another clustering algorithm hereafter
    //				for example, we have k initial points for k-means clustering algorithm
    //tree.redist_kmeans( items, entries, 0 );
    std::vector<int> item_cids;
    tree.redist( allData.begin(), allData.end(), entries, item_cids );
    for( std::size_t i = 0 ; i < item_cids.size() ; i++ )
        allData[i].cid() = item_cids[i];
    //print_items( argc >=4 ? argv[3] : "item_cid.txt" , items);

    // Visualize the data.
    QVector<Qt::GlobalColor> colors;
    colors<<Qt::red<<Qt::blue<<Qt::green<<Qt::black<<Qt::magenta<<Qt::cyan<<Qt::yellow;
    MainWindow *figure = new MainWindow();
    for (std::size_t i=0; i<entries.size(); ++i)
    {
        QVector<double> _x, _y;
        foreach (ItemND item, allData) {
            if (item.cid() == i) {
                _x.append(item[0]);
                _y.append(item[1]);
            }
        }
        figure->plot(_x, _y, colors.at(i % colors.count()), "");
    }
    figure->show();
}

void Apps::scpm(const QString &clusterFileName, const QString &tincFileName,
                const QString &outputFileName, double continuityRadius, int minSup)
{
    QVector<SegmentLocation> clusters = retrieveClusters(clusterFileName);
    QHash<unsigned int, QVector<unsigned int> > scMap =
            getSpatialContinuityMap(clusters, continuityRadius);
    QVector<QVector<unsigned int> > tinc = retrieveTinC(tincFileName);
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
    qDebug()<<"Prefix: "<<currPrefix<<", tocheck: "<<toCheck;
    if (toCheck.isEmpty())
        return;
    qDebug()<<"Prefix: "<<currPrefix<<", tocheck: "<<toCheck;
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
    qDebug()<<"Prefix: "<<currPrefix<<", freq: "<<freq;
    // Store patterns and invoke PrefixSpan recursively.
    foreach (unsigned int c, freq) {
        QVector<unsigned int> newPrefix = currPrefix;
        newPrefix.append(c);
        allPatterns <<  newPrefix;
        QVector<QVector<unsigned int> > newProjs;
        QVector<int> newProjsFrom;
        for (int i=0; i<projs.count(); ++i) {
            int idx = projs[i].indexOf(c, projsFrom[i]);
            if (idx >= 0 && idx < projs[i].count()-1) {
                newProjs << projs[i];
                newProjsFrom << (idx+1);
            }
        }
        qDebug()<<"New projs and new from for"<<c<<" is "<<newProjs<<", "<<newProjsFrom;
        if (newProjs.count() >= minSup) {
            prefixSpan(newPrefix, newProjs, newProjsFrom, scMap, allPatterns, minSup);
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

