/*
 * Memory.cpp
 *
 *      Author: oneamtu
 */

#include "Common.h"

#include "Memory.h"

namespace memory {

long long int birth_time; //the time we initialized memory
//everything else is time stamped relative to this

using log::LoggingBoard;

Memory::Memory(shared_ptr<Profiler> profiler_ptr,
        shared_ptr<Vision> vision_ptr,
        shared_ptr<Sensors> sensors_ptr) :
        _profiler(profiler_ptr),
        mvision(new MVision(vision_ptr)),
        msensors(new MSensors(profiler_ptr, sensors_ptr)),
        loggingBoard(new LoggingBoard(this)){
    birth_time = process_micro_time();
    sensors_ptr->addSubscriber(msensors);
}

Memory::~Memory() {
    delete mvision;
    delete msensors;

    delete loggingBoard;
}

void Memory::update(MObject* obj) {
    obj->update();
}

void Memory::updateVision() {
    update(mvision);
    loggingBoard->log(mvision);
}

void Memory::updateMotionSensors() {
    update((MMotionSensors*) msensors);
}

void Memory::updateVisionSensors() {
    update((MVisionSensors*) msensors);
}

}
