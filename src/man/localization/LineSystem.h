/**
 * @brief A class responsible for maintaing knowedge of lines on the field
 *        and comparing projectinos and observations
 *
 * @author EJ Googins <egoogins@bowdoin.edu>
 * @date   June 2013
 */

#pragma once

#include "Particle.h"
#include "Vision.pb.h"
#include "FieldConstants.h"
#include "../vision/Homography.h"

namespace man {
namespace localization {

// HIGH PRIORITY
// TODO LocLineID
//
// LOW PRIORITY
// TODO refactor Vision.pb.h
// TODO add side goalbox lines
class LineSystem {
public:
    LineSystem();
    ~LineSystem();

    // LocLineID matchObservation(const messages::FieldLine& observation, const messages::RobotLocation& loc);
    double scoreObservation(const messages::FieldLine& observation, const messages::RobotLocation& loc);
    // messages::RobotLocation constructPosition(const messages::FieldLine& observation, const messages::RobotLocation& loc);

    static vision::GeoLine relRobotToAbsolute(const messages::FieldLine& observation, const messages::RobotLocation& loc);
    static bool shouldUse(const messages::FieldLine& observation); 

private:
    void addLine(float r, float t, float ep0, float ep1);

    std::vector<vision::GeoLine> lines;
};

} // namespace localization
} // namespace man
