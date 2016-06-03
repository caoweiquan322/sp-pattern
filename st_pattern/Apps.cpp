/* Copyright © 2015 DynamicFatty. All Rights Reserved. */

#include "Apps.h"
#include "SpatialTemporalException.h"
#include "birch/CFTree.h"
#include "mainwindow.h"
#include <QDir>
#include <QFileInfoList>
#include <QFileInfo>
#include "Trajectory.h"
#include <QException>
#include <vector>
#include <QDebug>
#include "RobustnessTester.h"

Apps::Apps()
{

}

Apps::~Apps()
{

}

QStringList retrieveFilesWithSuffix(QString dirPath, QString suffix,
                                    QStringList currentList)
{
    try {
        QDir dir(dirPath);
        QFileInfoList files = dir.entryInfoList(QDir::Dirs | QDir::Files);
        foreach (QFileInfo info, files) {
            if (info.isDir() && info.fileName().compare(".") && info.fileName().compare("..")) {
                //qDebug()<<"Processing folder: "<<info.fileName();
                currentList = retrieveFilesWithSuffix(info.absoluteFilePath(), suffix, currentList);
            } else if (info.isFile() && info.fileName().endsWith(suffix)) {
                //qDebug()<<"Adding file: "<<info.absoluteFilePath();
                currentList.append(info.absoluteFilePath());
            }
        }
        return currentList;
    } catch (QException &) {
        return currentList;
    }
}

void Apps::segmentTrajectories(const QString &fileDir, const QString &outputFile)
{
    SpatialTemporalException("Apps::segmentTrajectories() is not implemented yet.").raise();
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

