#pragma once

#include "DebugConfig.h"

#include "RoboGrams.h"
#include "Common.h"
#include <math.h>
#include "FieldConstants.h"

#include <iostream>

#include "WorldModel.pb.h"
#include "BallModel.pb.h"
#include "RobotLocation.pb.h"

/*
 * Module
 *
 */

namespace man {
namespace naive {

const float ALPHA = .7f;
const float STATIONARY_THRESHOLD = 3.0f; // Any velocity w/ abs below this is stationary
const float FRICTION = .9f;   // Help?????
const int NUM_FRAMES = 30;  // Num frames to keep a buffer of; 30 frames ~ 1 sec
const int MAX_FRAMES_OFF = 5;   // If don't see the ball for 5 frames, clear buffer
const int AVGING_FRAMES = 5; // Number of frames to take an avg position estimate from
const int NUM_DEST = 10; // Number of destination predictions to make

class NaiveBallModule : public portals::Module
{
public:
    struct BallState;
    NaiveBallModule();
    virtual ~NaiveBallModule();

    virtual void run_();

    portals::InPortal<messages::FilteredBall> ballIn;
    portals::OutPortal<messages::NaiveBall> naiveBallOutput;

private:
    float calcSumSquaresSQRT(float a, float b);
    bool checkIfStationary();
    void updateBuffers();
    void clearBuffers();
    void calcPath();
    float calculateVelocity(bool x);
    BallState avgFrames(int startingIndex);

    void print();
    void printBallState(BallState x);
    void printBuffer();

    BallState *position_buffer;
    BallState *dest_buffer;
    float *vel_x_buffer;
    float *vel_y_buffer;

    int currentIndex;
    int frameOffCount;

    float direction;
    float velocityEst;
    float xVelocityEst;
    float yVelocityEst;
    float yIntercept;

    bool bufferFull;
    bool velBufferFull;

    messages::WorldModel worldMessages[NUM_PLAYERS_PER_TEAM];
    messages::FilteredBall myBall;

};

}
}
