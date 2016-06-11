/* Copyright © 2015 DynamicFatty. All Rights Reserved. */

#include <QApplication>
#include "DotsException.h"
#include <QException>
#include <QDebug>
#include "SpatialTemporalException.h"
#include "Apps.h"
#include <QVector>

void printUsage() {
    qDebug()<<"The st-pattern mining tool consists of 3 phases:\n"
           <<"The SEG phase converts trajectory into segments with multi-threshold.\n"
          <<"The CLUSTER-TRANS phase clusters the generated segments and then converts trajectories into transactional data.\n"
        <<"The MINE phase mines frequent pattern from the transactional data.\n";
    qDebug()<<"Usage:\n"
           <<"st_pattern seg dataset_dir dataset_suffix output segmentation_step use_temporal min_seg_length\n"
          <<"e.g.: st_pattern seg path_to_mopsi .txt mopsi_100 1.6 1 100.0\n\n"
         <<"st_pattern cluster segment_file w1:w2:w3:w4:w5:w6 output threshold [mem_lim_in_MB]\n"
        <<"e.g.: st_pattern cluster mopsi_100 0.0001:0.0001:0.0001:0.0001:0:0 mopsi_100_50 50.0 100\n\n"
       //<<"st_pattern trans tins_file s2c_file [output_tinc_file]\n"
      //<<"The output_tinc_file is equal to s2c_file by default.\n"
      //<<"e.g.: st_pattern trans mopsi_100 mopsi_100_50 mopsi_100_50"
     <<"st_pattern mine cluster_file tinc_file output_pattern_file scpm_radius min_sup [min_pattern_length]\n"
    <<"e.g.: st_pattern mine mopsi_100_50 mopsi_100_50 mopsi_100_50_50_5 50.0 5 3";
}

int main(int argc, char *argv[])
{
    // The application.
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    QApplication::setGraphicsSystem("raster");
#endif
    QApplication a(argc, argv);

    // Proprocess argv.
    QStringList args;
    for (int i=0; i<argc; ++i)
        args<<QString(argv[i]);

    try {
        int ret = 0;
        if (args.count() < 2) {
            qDebug()<<"Commands too less";
            printUsage();
            qDebug("\nPress any key to continue ...");
            return 0;
        }
        if (args[1].compare("seg") == 0 && args.count() == 8) {
            qDebug()<<"\n============> The "<<args[1]<<" begins <============";
            Apps::segmentTrajectories(args[2], args[3], args[4],
                    args[5].toDouble(), (bool)(args[6].toInt()), args[7].toDouble());
            qDebug()<<"\n============>  The "<<args[1]<<" ends  <============";
            //ret = a.exec();
        } else if (args[1].compare("visualize") == 0 && args.count() >= 4) {
            qDebug()<<"\n============> The "<<args[1]<<" begins <============";
            Apps::visualizeDataset(args[2], args[3], args.count() > 4 ? args[4].toDouble() : 40000.0);
            qDebug()<<"\n============>  The "<<args[1]<<" ends  <============";
            //ret = a.exec();
        } else if (args[1].compare("cluster") == 0 && args.count() >= 6) {
            qDebug()<<"\n============> The "<<args[1]<<" begins <============";
            QVector<double> weights;
            QStringList strW = args[3].split(":");
            qDebug()<<"Weights to segment locations: "<<strW;
            if (strW.count() != 6) {
                qDebug("The weights must be of dimension 6.");
                return 0;
            }
            foreach (QString w, strW) {
                weights << w.toDouble();
            }
            Apps::clusterSegments(args[2], weights, args[4],
                        args[5].toDouble(), args.count() > 6 ? (args[6].toInt())<<20 : 0);
            Apps::transTrajectories(args[2], args[4], args[4]);
            qDebug()<<"\n============>  The "<<args[1]<<" ends  <============";
            //ret = a.exec();
        } else if (args[1].compare("trans") == 0 && args.count() >= 4) {
            qDebug()<<"\n============> The "<<args[1]<<" begins <============";
            Apps::transTrajectories(args[2], args[3], args.count() > 4 ? args[4] : args[3]);
            qDebug()<<"\n============>  The "<<args[1]<<" ends  <============";
        } else if (args[1].compare("mine") == 0 && args.count() >= 7) {
            qDebug()<<"\n============> The "<<args[1]<<" begins <============";
            Apps::scpm(args[2], args[3], args[4], args[5].toDouble(), args[6].toInt(),
                    args.count() > 7 ? args[7].toInt() : 1);
            qDebug()<<"\n============>  The "<<args[1]<<" ends  <============";
            //ret = a.exec();
        } else if (args[1].compare("generate") == 0 && args.count() == 6) {
            qDebug()<<"\n============> The "<<args[1]<<" begins <============";
            Apps::generateDataSet(args[2], args[3], args[4], args[5]);
            qDebug()<<"\n============>  The "<<args[1]<<" ends  <============";
            //ret = a.exec();
        } else if (args[1].compare("test") == 0 && args.count() == 2) {
            qDebug()<<"\n============> The "<<args[1]<<" begins <============";
            Apps::testPrefixSpan();
            //Apps::testTrie();
            qDebug()<<"\n============>  The "<<args[1]<<" ends  <============";
        } else {
            qDebug()<<"Malformed command "<<args[1];
            printUsage();
        }

        qDebug("\nPress any key to continue ...");
        return ret;
    } catch (DotsException &e) {
        qDebug()<<"Error occurs during running DOTS algorithm: "<<e.getMessage();
        //e.raise();
    } catch (SpatialTemporalException &e) {
        qDebug()<<"Error occurs during pattern mining: "<<e.getMessage();
    } catch (QException &) {
        qDebug()<<"Unknown exception.";
    }
}
