#include "LocalizationModule.h"

namespace man
{
    namespace localization
    {
    LocalizationModule::LocalizationModule()
        : portals::Module(),
          output(base())
    {
        particleFilter = new ParticleFilter();
        particleFilter->resetLocTo(100,100,0);
        std::cout << particleFilter->getXEst() << "\t"
                  << particleFilter->getYEst() << "\t"
                  << particleFilter->getHEst() << "\n";
    }

    LocalizationModule::~LocalizationModule()
    {
        delete particleFilter;
    }

    void LocalizationModule::update()
    {
        motionInput.latch();
        visionInput.latch();

        //std::cout << "Prepare to update filter \n";
        particleFilter->update(motionInput.message(), visionInput.message());
        //std::cout << "Updated filter \n";
        portals::Message<messages::RobotLocation> locMessage(0);
        *locMessage.get() = messages::RobotLocation();

        messages::RobotLocation l = particleFilter->getCurrentEstimate();
        // std::cout << "X estimate:\t " << l.x() << "\nY estimate:\t " << l.y()
        //           << "\nH estimate:\t " << l.h() << "\n\n";

        locMessage.get()->CopyFrom(particleFilter->getCurrentEstimate());
        output.setMessage(locMessage);
    }

    void LocalizationModule::run_()
    {
        update();
    }

    } // namespace localization
} // namespace localization
