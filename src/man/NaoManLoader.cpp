#include <signal.h> //for kill
#include <sys/types.h> //for pid_t
#include <dlfcn.h>
#include <iostream>

#include "NaoManLoader.h"

//NBites includes

#include "TMan.h"
#include "NaoEnactor.h"

typedef NaoEnactor EnactorT;
typedef TMan ALMan;

#include "ALTranscriber.h"
#include "ALImageTranscriber.h"

#include "NaoLights.h"

#include "vision/Profiler.h"

using namespace std;
using namespace AL;
using boost::shared_ptr;

static shared_ptr<ALMan> man_pointer;

START_FUNCTION_EXPORT

void loadMan(ALPtr<ALBroker> broker, shared_ptr<Speech> speech,
             shared_ptr<Sensors> sensors, shared_ptr<RoboGuardian> guardian) {

//#ifdef USE_ALSPEECH
//    shared_ptr<Speech> speech(new ALSpeech(broker));
//#else
//    shared_ptr<Speech> speech(new Speech());
//#endif
//    shared_ptr<Sensors> sensors(new Sensors(speech));

    shared_ptr<ALTranscriber>
    transcriber(new ALTranscriber(broker, sensors));

    shared_ptr<ALImageTranscriber>
        imageTranscriber(new ALImageTranscriber(sensors,
                                            broker));
    shared_ptr<Profiler>
        profiler(new Profiler(&thread_micro_time, &process_micro_time,
                          &monotonic_micro_time));
    shared_ptr<EnactorT> enactor(new EnactorT(sensors, transcriber,
                                                     broker));

    shared_ptr<Lights> lights(new NaoLights(broker));

    //setLedsProxy(AL::ALPtr<AL::ALLedsProxy>(new AL::ALLedsProxy(broker)));

    man_pointer = boost::shared_ptr<ALMan>(new ALMan(profiler, sensors,
                                                     guardian,
                                                     transcriber,
                                                     imageTranscriber,
                                                     enactor, lights, speech));
    man_pointer->startSubThreads();
}

void unloadMan() {
    man_pointer->stopSubThreads();
    man_pointer.reset();
}

END_FUNCTION_EXPORT
