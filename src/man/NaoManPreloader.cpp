
#include "NaoManPreloader.h"

#include <dlfcn.h> //dlopen, dlsym
#include <stdlib.h> //exit
#include <iostream> //cout
#include <assert.h>

#include "ALSpeech.h"

using namespace std;
using namespace AL;

START_FUNCTION_EXPORT

//This is what Aldebaran will call when it loads this module
//Note: this is the point of entry for our code
int _createModule(ALPtr<ALBroker> pBroker) {
    ALModule::createModule<NaoManPreloader>(pBroker, "NaoManPreloader");
    return 0;
}

//Aldebaran apparently never calls this - Octavian
int _closeModule() {
    return 0;
}

END_FUNCTION_EXPORT

static string LIBMAN_NAME="libman.so";
static string LOAD_MAN_METHOD_NAME="loadMan";

NaoManPreloader::NaoManPreloader(AL::ALPtr<AL::ALBroker> pBroker,
                                 const std::string& pName) :
                ALModule(pBroker, pName), broker(pBroker) {

    this->setModuleDescription("A module that kicks ass.");
    preloadMan();
}

NaoManPreloader::~NaoManPreloader() {
    cout << "Destroying the man preloader" << endl;
}

void NaoManPreloader::preloadMan() {
    importMan();
    linkManLoaderMethod();
    launchMan();
}

void NaoManPreloader::importMan() {
    cout << "Importing " + LIBMAN_NAME + " ... ";
    libman_handle = dlopen(LIBMAN_NAME.c_str(),
                           RTLD_LAZY);
    if (!libman_handle)
    {
        cout << dlerror() << endl;
        std::exit(1);
    }
    cout << "done"<<endl;
}

void NaoManPreloader::linkManLoaderMethod() {
    cout << "Linking to " + LOAD_MAN_METHOD_NAME + " ... ";
    loadMan = reinterpret_cast<loadManMethod>(
            dlsym(libman_handle, "loadMan"));
    if (loadMan == NULL)
    {
        cout << dlerror() << endl;
        std::exit(1);
    }
    cout << "done" << endl;
}

void NaoManPreloader::launchMan() {
    cout << "Launching man loader ... ";
    assert(loadMan != NULL);
    (*loadMan)(broker);
    cout << "done" << endl;
}

