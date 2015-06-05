#include "Kinematics.h"

#include "NBMath.h"

#include <iostream>

namespace man {
namespace vision {
    
Kinematics::Kinematics(bool topCamera)
    : needCompute(false),
      constants(topCamera),
      joints(),
      tilt_(0),
      wz0_(0)
{}

void Kinematics::setJointAngles(const messages::JointAngles& joints_)
{
    needCompute = true;
    joints = joints_;
}

void Kinematics::compute()
{
    // Do not compute more than once
    if (!needCompute) return;
    needCompute = false;

    // Convert to radians
    double la = joints.l_ankle_pitch();
    double lk = joints.l_knee_pitch();
    double lh = joints.l_hip_pitch();

    double ra = joints.r_ankle_pitch();
    double rk = joints.r_knee_pitch();
    double rh = joints.r_hip_pitch();

    double h = joints.head_pitch();

    // Compute kinematics for left and right leg
    std::pair<double, double> leftLeg = computeForLeg(la, lk, lh, h);
    std::pair<double, double> rightLeg = computeForLeg(ra, rk, rh, h);
    double leftCameraHeight = leftLeg.second;
    double rightCameraHeight = rightLeg.second;

    // Compute tilt and wz0 for whatever leg we are standing on
    if (leftCameraHeight > rightCameraHeight) {
        tilt_ = 0.5 * M_PI - (leftLeg.first + constants.opticalAxisOffset);
        wz0_ = leftCameraHeight;
    } else {
        tilt_ = 0.5 * M_PI - (rightLeg.first + constants.opticalAxisOffset);
        wz0_ = rightCameraHeight;
    }

    std::cout << "Tilt: " << tilt_ << std::endl;
    std::cout << "Height: " << wz0_ << std::endl;
}

std::pair<double, double> Kinematics::computeForLeg(double anklePitch,
                                                    double kneePitch,
                                                    double hipPitch,
                                                    double neckPitch) const
{
    std::pair<double, double> headPitchAndCameraHeight;

    double tibiaPitch = anklePitch;
    double thighPitch = tibiaPitch + kneePitch;
    double torsoPitch = thighPitch + hipPitch;
    double headPitch = torsoPitch + neckPitch;

    double cameraHeight = constants.groundToAnkle +
                          cos(tibiaPitch) * constants.ankleToKnee +
                          cos(thighPitch) * constants.kneeToHip +
                          cos(torsoPitch) * constants.hipToNeck +
                          cos(headPitch) * constants.neckToCameraZ -
                          sin(headPitch) * constants.neckToCameraX;

    headPitchAndCameraHeight.first = headPitch;
    headPitchAndCameraHeight.second = cameraHeight;
    return headPitchAndCameraHeight;
}

}
}
