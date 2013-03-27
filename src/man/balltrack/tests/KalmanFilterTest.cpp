#include "gtest/gtest.h"

#include "../KalmanFilter.h"
#include "../NaiveBallFilter.h"

using namespace man::balltrack;

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

TEST (KalmanFilterTest, FilterCanUpdate) {
    KalmanFilter *kf = new KalmanFilter();
    kf->isUpdated();
    bool yes = true;
    bool no = false;
    ASSERT_EQ(no, kf->isUpdated());
    kf->setUpdate(true);
    ASSERT_EQ(yes, kf->isUpdated());

    int a =1;
    int b =1;
    ASSERT_EQ(a,b);
}

TEST (KalmanFilterTest, FilterCanPredict) {
    KalmanFilter *kf = new KalmanFilter();
    kf->initialize();
    float initX = 0.f;
    float initY = 0.f;
    float initXVel = 1.f;
    float initYVel = 0.5f;
    float finalX = 4.f;
    float finalY = 2.f;
    ASSERT_EQ(initX, kf->getRelXPosEst());
    ASSERT_EQ(initY, kf->getRelYPosEst());

    messages::RobotLocation odometry;
    odometry.set_x(0.f);
    odometry.set_y(0.f);
    odometry.set_h(0.f);
    kf->predict(odometry, 4.f);

    ASSERT_EQ(finalX, kf->getRelXPosEst());
    ASSERT_EQ(finalY, kf->getRelYPosEst());
}

TEST (KalmanFilterTest, KalmanFilterCanInitialize) {
    KalmanFilter *kf = new KalmanFilter();
    kf->initialize();
    ASSERT_EQ(kf->getRelXPosEst(), 0.f);
    ASSERT_EQ(kf->getRelYPosEst(), 0.f);
    ASSERT_EQ(kf->getRelXVelEst(), 1.f);
    ASSERT_EQ(kf->getRelYVelEst(), 0.5f);

    ufvector4 initX = boost::numeric::ublas::zero_vector<float> (4);
    initX(0) = 10.f;
    initX(1) = 11.f;
    initX(2) = 12.f;
    initX(3) = 13.f;
    ufmatrix4 initCov = boost::numeric::ublas::identity_matrix<float> (4);
    initCov(0,0) = 1.f;
    initCov(1,1) = 2.f;
    initCov(2,2) = 3.f;
    initCov(3,3) = 4.f;

    kf->initialize(initX, initCov);

    ASSERT_EQ(kf->getRelXPosEst(), 10.f);
    ASSERT_EQ(kf->getRelYPosEst(), 11.f);
    ASSERT_EQ(kf->getRelXVelEst(), 12.f);
    ASSERT_EQ(kf->getRelYVelEst(), 13.f);

    ASSERT_EQ(kf->getCovXPosEst(), 1.f);
    ASSERT_EQ(kf->getCovYPosEst(), 2.f);
    ASSERT_EQ(kf->getCovXVelEst(), 3.f);
    ASSERT_EQ(kf->getCovYVelEst(), 4.f);
}
