#include "SensorsModule.h"
#include "Common.h"

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>

namespace man {
namespace sensors {

SensorsModule::SensorsModule()
    : portals::Module(),
      jointsOutput_(base()),
      currentsOutput_(base()),
      temperatureOutput_(base()),
      chestboardButtonOutput_(base()),
      footbumperOutput_(base()),
      inertialsOutput_(base()),
      sonarsOutput_(base()),
      fsrOutput_(base()),
      batteryOutput_(base()),
      stiffStatusOutput_(base()),
      lastPrint(0)
{
    shared_fd = shm_open(NBITES_MEM, O_RDWR, 0600);
    if (shared_fd < 0) {
        // TODO error
    }
    if (ftruncate(shared_fd, sizeof(SharedData)) == -1) {
        // TODO error
    }

    shared = (SharedData*) mmap(NULL, sizeof(SharedData),
                                PROT_READ | PROT_WRITE,
                                MAP_SHARED, shared_fd, 0);

    if (shared == MAP_FAILED) {
        // TODO error
    }
}

SensorsModule::~SensorsModule()
{
    // Close shared memory
    munmap(shared, sizeof(SharedData));
    close(shared_fd);
    //sem_close(semaphore);
}


// Helper method so that we can print out a Sweet Moves joint angle
// tuple directly when we want to (ie 5 button presses)
std::string SensorsModule::makeSweetMoveTuple(const messages::JointAngles* angles)
{
    char output[240];

    sprintf(output, "((%3.1f, %3.1f, %3.1f, %3.1f),\n(%3.1f, %3.1f, %3.1f, %3.1f, %3.1f, %3.1f),\n(%3.1f, %3.1f, %3.1f, %3.1f, %3.1f, %3.1f),\n(%3.1f, %3.1f, %3.1f, %3.1f))\n",
            TO_DEG*angles->l_shoulder_pitch(),
            TO_DEG*angles->l_shoulder_roll(),
            TO_DEG*angles->l_elbow_yaw(),
            TO_DEG*angles->l_elbow_roll(),
            TO_DEG*angles->l_hip_yaw_pitch(),
            TO_DEG*angles->l_hip_roll(),
            TO_DEG*angles->l_hip_pitch(),
            TO_DEG*angles->l_knee_pitch(),
            TO_DEG*angles->l_ankle_pitch(),
            TO_DEG*angles->l_ankle_roll(),
            TO_DEG*angles->l_hip_yaw_pitch(),
            TO_DEG*angles->r_hip_roll(),
            TO_DEG*angles->r_hip_pitch(),
            TO_DEG*angles->r_knee_pitch(),
            TO_DEG*angles->r_ankle_pitch(),
            TO_DEG*angles->r_ankle_roll(),
            TO_DEG*angles->r_shoulder_pitch(),
            TO_DEG*angles->r_shoulder_roll(),
            TO_DEG*angles->r_elbow_yaw(),
            TO_DEG*angles->r_elbow_roll());

    return std::string(output);
}

void SensorsModule::updateSensorValues()
{
    // TODO grab semaphore
    uint8_t index = shared->sensorSwitch;
    values = shared->sensors[index];
    sensorIndex = values.writeIndex;
    shared->sensorReadIndex = sensorIndex;
    // TODO Release semaphore

    portals::Message<messages::JointAngles> joints(&(values.joints));
    portals::Message<messages::JointAngles> currents(&(values.currents));
    portals::Message<messages::JointAngles> temps(&(values.temperature));
    portals::Message<messages::ButtonState> chestButton(&(values.chestButton));
    portals::Message<messages::FootBumperState> footBumper(&(values.footBumper));
    portals::Message<messages::InertialState> inertials(&(values.inertials));
    portals::Message<messages::SonarState> sonars(&(values.sonars));
    portals::Message<messages::FSR> fsrs(&(values.fsr));
    portals::Message<messages::BatteryState> battery(&(values.battery));
    portals::Message<messages::StiffStatus> stiffness(&(values.stiffStatus));

    jointsOutput_.setMessage(joints);
    currentsOutput_.setMessage(currents);
    temperatureOutput_.setMessage(temps);
    chestboardButtonOutput_.setMessage(chestButton);
    footbumperOutput_.setMessage(footBumper);
    inertialsOutput_.setMessage(inertials);
    sonarsOutput_.setMessage(sonars);
    fsrOutput_.setMessage(fsrs);
    batteryOutput_.setMessage(battery);
    stiffStatusOutput_.setMessage(stiffness);
}

void SensorsModule::run_()
{
    printInput.latch();

    // Simply update all sensor readings from shared memory
    updateSensorValues();

    if(printInput.message().toggle() != lastPrint)
    {
        std::cout << makeSweetMoveTuple(jointsOutput_.getMessage(true).get())
                  << std::endl;
        lastPrint = !lastPrint;
    }
}

} // namespace sensors
} // namespace man
