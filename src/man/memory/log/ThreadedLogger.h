/*
 * @class ThreadedLogger
 *
 * Puts the logger in a thread of its own and puts in some
 * aio functionality so that the writes don't block anymore
 * (a blocking write will block the ENTIRE process and not only
 * one thread)
 *
 * @author Octavian Neamtu
 *
 */


#include "Logger.h"
#include "synchro/synchro.h"
#include "Subscriber.h"
#include "ClassHelper.h"

namespace man {
namespace memory {
namespace log {

class ThreadedLogger : public Logger, public Thread, public Subscriber {

ADD_SHARED_PTR(ThreadedLogger)

public:
    ThreadedLogger(OutProvider::ptr out_provider, std::string name) :
                   Logger(out_provider), Thread(name) {
    }

    virtual ~ThreadedLogger(){
        this->stop();
    }

    virtual void writeToLog() = 0;

    virtual void run() {
        while (running) {

            if (!out_provider->opened()) {
                //blocking for socket fds, (almost) instant for other ones
                out_provider->openCommunicationChannel();
                std::cout << "writing head out" << std::endl;
                this->writeHead();
            }

            this->waitForSignal();
            this->writeToLog();

            while(out_provider->writingInProgress()) {
                this->yield();
            }
        }
    }

    void signalToLog() {
        this->signalToResume();
    }

    void update() {
    	this->signalToLog();
    }

};

}
}
}
