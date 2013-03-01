#include "FakeLocInputModule.h"

namespace man
{
    namespace fakeInput
    {
        FakeLocInputModule::FakeLocInputModule()
            :portals::Module(),
             fMotionOutput(base()),
             fVisionOutput(base())
        {
            // Set the initial, current, and final Location
            initialLocation.set_x(INITIAL_X);
            initialLocation.set_y(INITIAL_Y);
            initialLocation.set_h(INITIAL_H);

            currentLocation.set_x(initialLocation.x());
            currentLocation.set_y(initialLocation.y());
            currentLocation.set_h(initialLocation.h());

            finalLocation.set_x(FINAL_X);
            finalLocation.set_y(FINAL_Y);
            finalLocation.set_h(FINAL_H);

            timestamp = 0;
            frames = NUM_FRAMES;

            calcDeltaMotion();
        }

        FakeLocInputModule::~FakeLocInputModule(){}

        void FakeLocInputModule::calcDeltaMotion()
        {
            odometry.set_x((finalLocation.x() + initialLocation.x()) / frames);
            odometry.set_y((finalLocation.y() + initialLocation.y()) / frames);
            odometry.set_h((finalLocation.h() + initialLocation.h()) / frames);
        }

        void FakeLocInputModule::genNoisyOdometry()
        {
            // Set the new timestamp
            noisyMotion.set_timestamp((google::protobuf::int64) timestamp);

            //Determine how much noise to add (10%)
            float xVariance = (float) std::abs(odometry.x() * .1);
            float yVariance = (float) std::abs(odometry.y() * .1);
            float hVariance = (float) std::abs(odometry.h() * .1);

            boost::uniform_real<float> xVarRange(odometry.x() - xVariance, odometry.x() + xVariance);
            boost::uniform_real<float> yVarRange(odometry.y() - yVariance, odometry.y() + yVariance);
            boost::uniform_real<float> hVarRange(odometry.h() - hVariance, odometry.h() + hVariance);
            boost::variate_generator<boost::mt19937&, boost::uniform_real<float> > xNoise(rng, xVarRange);
            boost::variate_generator<boost::mt19937&, boost::uniform_real<float> > yNoise(rng, yVarRange);
            boost::variate_generator<boost::mt19937&, boost::uniform_real<float> > hNoise(rng, hVarRange);

            noisyOdometry.set_x(xNoise());
            noisyOdometry.set_y(yNoise());
            noisyOdometry.set_h(hNoise());

            // copy the new noisy odometry to the noisyMotion member
            noisyMotion.mutable_odometry()->CopyFrom(noisyOdometry);

        }

        void FakeLocInputModule::addGoalObservation(bool rightGoal)
        {
            messages::PVisualGoalPost goalObservation;

            // Each observation needs to have a visual detection
            messages::PVisualDetection goalVisualDetection;
            genVisualDetection(goalVisualDetection);

            // add the visual detection to the visual goal post
            goalObservation.mutable_visual_detection()->CopyFrom(goalVisualDetection);

            // add the visual goal post to the noisyVision message
            if(rightGoal)
                noisyVision.mutable_goal_post_r()->CopyFrom(goalObservation);
            else
                noisyVision.mutable_goal_post_l()->CopyFrom(goalObservation);
        }


        void FakeLocInputModule::genVisualDetection(messages::PVisualDetection &visualDetection)
        {
            // Choose concrete_coords randomly for the observation
            boost::uniform_real<float> observationRange(MIN_OBS_DIST, MAX_OBS_DIST);
            boost::variate_generator<boost::mt19937&, boost::uniform_real<float> > randOffset(rng,
                                                                                              observationRange);
            // Fill in the Visual Detections distance and bearing with noise
            // Use a gaussian since that's the assumption for a particle filter
            float translatedX = randOffset();
            float translatedY = randOffset();

            // @TODO one line? Don't think so but try
            messages::Point concreteCoords;
            concreteCoords.set_x(currentLocation.x() + translatedX);
            concreteCoords.set_y(currentLocation.y() + translatedY);
            visualDetection.add_concrete_coords()->CopyFrom(concreteCoords);

            float calcDistance = std::sqrt(NBMath::square(translatedX) + NBMath::square(translatedY));
            float calcBearing = NBMath::safe_atan2(translatedY, translatedX);

            boost::normal_distribution<float> distDistrib(calcDistance, DIST_STD_DEV);
            boost::normal_distribution<float> bearingDistrib(calcBearing, BEAR_STD_DEV);

            boost::variate_generator<boost::mt19937&,
                                     boost::normal_distribution<float> > distanceGen(rng, distDistrib);
            boost::variate_generator<boost::mt19937&,
                                     boost::normal_distribution<float> > bearingGen(rng, bearingDistrib);

            visualDetection.set_distance(distanceGen());
            visualDetection.set_bearing(bearingGen());

        }

        void FakeLocInputModule::addCrossObservation()
        {
            // NOTE: The visual cross observation is just a visual detection
            messages::PVisualDetection crossVisualDetection;
            genVisualDetection(crossVisualDetection);

            // add the observation to the noisyVision message
            noisyVision.mutable_visual_cross()->CopyFrom(crossVisualDetection);
        }

        void FakeLocInputModule::addCornerObservation()
        {
            messages::PVisualCorner cornerObservation;
            // Each corner observation needs a visual detection
            messages::PVisualDetection cornerVisualDetection;
            genVisualDetection(cornerVisualDetection);

            // add the visual detection to the visual corner
            cornerObservation.mutable_visual_detection()->CopyFrom(cornerVisualDetection);

            // add the visual corner to the noisyVision message
            noisyVision.add_visual_corner()->CopyFrom(cornerObservation);
        }

        void FakeLocInputModule::genNoisyVision()
        {
            // Clear the current noisy vision
            noisyVision.Clear();

            // Increment the timestamp
            noisyVision.set_timestamp((google::protobuf::int64) timestamp);

            // Add corner observations
            for(int i=0; i<NUM_CORNER_OBS; i++)
                addCornerObservation();

            if(CROSS_OBS)
                addCrossObservation();

            // Add goal observations
            for(int i=0; i<NUM_GOAL_OBS; i++)
                addGoalObservation(false);
        }

        void FakeLocInputModule::run_()
        {

            // messages::RobotLocation stupidOdometry;
            // stupidOdometry.set_x(1);
            // stupidOdometry.set_y(2);

            // portals::Message<messages::Motion> odometryMessage(0);
            // *odometryMessage.get() = messages::Motion();
            // odometryMessage.get()->CopyFrom(stupidOdometry);
            // fMotionOutput.setMessage(odometryMessage);


            // Generate a new motion message
            // NOTE: Directly modifies the noisyMotion member
            genNoisyOdometry();

            // Create a Message for the motion portal, set it appropriatly
            portals::Message<messages::Motion> odometryMessage(0);
            *odometryMessage.get() = messages::Motion();
            odometryMessage.get()->CopyFrom(noisyMotion);
            fMotionOutput.setMessage(odometryMessage);

            // Set the correct actual location now that we've 'traveled' there
            currentLocation.set_x(currentLocation.x() + odometry.x());
            currentLocation.set_y(currentLocation.y() + odometry.y());
            currentLocation.set_h(currentLocation.h() + odometry.h());

            // Generate observations for this new location
            // NOTE: Directly modifies the noisyVision memeber
            genNoisyVision();

            // // Create a Message for the vision portal, set it appropriatly
            portals::Message<messages::PVisionField> visionMessage(0);
            *visionMessage.get() = messages::PVisionField();
            visionMessage.get()->CopyFrom(noisyVision);
            fVisionOutput.setMessage(visionMessage);

            // increment timestamp, set negative if gen'd all frames so we terminate
            timestamp++;
            if (timestamp > frames)
                timestamp = -1;
        }


    } //namespace fakeInput
} // namespace man
