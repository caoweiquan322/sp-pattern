/* Copyright © 2015 DynamicFatty. All Rights Reserved. */

#include <QString>
#include <QtTest>
//#include "../st_pattern/Trajectory.h"

class TestPatternMining : public QObject
{
    Q_OBJECT

public:
    TestPatternMining();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void testTrajectory_data();
    void testTrajectory();
};

TestPatternMining::TestPatternMining()
{
}

void TestPatternMining::initTestCase()
{
}

void TestPatternMining::cleanupTestCase()
{
}

void TestPatternMining::testTrajectory_data()
{
    QTest::addColumn<QString>("trajFilePath");
    QTest::newRow("GeoLife") << QString("");
    QTest::newRow("Mopsi") << QString("");
}

void TestPatternMining::testTrajectory()
{
    QFETCH(QString, trajFilePath);
    //Trajectory traj(trajFilePath);
    int trajSize = trajFilePath.count();
    QVERIFY2(trajSize > 0, "The trajectory is empty.");
    QString subTraj = trajFilePath.toUpper();
    QVERIFY2(subTraj.count()*5 <= trajSize, "Trajectory::sample failed.");
}

QTEST_APPLESS_MAIN(TestPatternMining)

#include "tst_TestPatternMining.moc"
