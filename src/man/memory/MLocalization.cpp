/*
 *MLocalization.cpp
 *
 *@author EJ Googins
 */

#include "MLocalization.h"

#include <vector>

namespace man {
namespace memory {

using boost::shared_ptr;
using namespace proto;
using namespace std;

MLocalization::MLocalization(shared_ptr<LocSystem> locSystem,
                             PLoc_ptr data) :
        MObject(id, data), locSystem(locSystem), data(data) {
}

//Destructor
MLocalization::~MLocalization() {
}

void MLocalization::updateData() {
    this->data->set_timestamp(time_stamp());

    this->data->set_x_est(locSystem->getXEst());
    this->data->set_y_est(locSystem->getYEst());
    this->data->set_h_est(locSystem->getHEst());
    this->data->set_x_uncert(locSystem->getXUncert());
    this->data->set_y_uncert(locSystem->getYUncert());
    this->data->set_h_uncert(locSystem->getHUncert());

    // Get the particles, and update the protobuf accordingly. 
    PF::ParticleSet particles = boost::static_pointer_cast<PF::ParticleFilter>(locSystem)->getParticles();

    PLoc::Particle *particle;
    for(PF::ParticleIt iter = particles.begin(); iter != particles.end(); ++iter)
    {
      particle = this->data->add_particles();
      particle->set_x((*iter).getLocation().x);
      particle->set_y((*iter).getLocation().y);
      particle->set_h((*iter).getLocation().heading);
      particle->set_w((*iter).getWeight());
    }
}
}
}
