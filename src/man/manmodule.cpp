/**
 * \mainpage
 * \section Author
 * @author NorthernBites
 *
 * \section Copyright
 * Version : $Id$
 *
 * \section Description
 *
 * This file was generated by Aldebaran Robotics ModuleGenerator
 */


//NB NOTE: This file contains the interface to Man used for Naoqi
//         See webotsmodule.cpp for the Webots interface.


#ifndef _WIN32
#include <signal.h>
#endif

#include "manmodule.h"

//NBites includes

#ifdef USE_DCM
#  if defined USE_DCM && defined MAN_IS_REMOTE
#    error "DCM not compatible with remote!!!"
#  endif

#include "TMan.h"
#include "NaoEnactor.h"

typedef NaoEnactor EnactorT;
typedef TMan ALMan;
#else
#include "TTMan.h"
#include "ALEnactor.h"
typedef ALEnactor EnactorT;
typedef TTMan ALMan;
#endif

#include "ALTranscriber.h"
#include "ALImageTranscriber.h"

#include "NaoLights.h"
#include "ALSpeech.h"

using namespace std;
using namespace AL;
using boost::shared_ptr;


static shared_ptr<ALMan> man;
static shared_ptr<Sensors> sensors;
static shared_ptr<Synchro> synchro;
static shared_ptr<ALTranscriber> transcriber;
static shared_ptr<ALImageTranscriber> imageTranscriber;
static shared_ptr<EnactorT> enactor;
static shared_ptr<Lights> lights;
static shared_ptr<Speech> speech;

void ALCreateMan( ALPtr<ALBroker> broker){
    try{
        ALSentinelProxy sentinel(broker);
        sentinel.enableDefaultActionSimpleClick(false);
        sentinel.enableDefaultActionDoubleClick(false);
        sentinel.enableDefaultActionTripleClick(false);
    }catch(ALError &e){
        cout << "Failed to access the ALSentinel: "<<e.toString()<<endl;
    }

    synchro = shared_ptr<Synchro>(new Synchro());
    sensors = shared_ptr<Sensors>(new Sensors);
    transcriber = shared_ptr<ALTranscriber>(new ALTranscriber(broker,sensors));
    imageTranscriber =
        shared_ptr<ALImageTranscriber>
        (new ALImageTranscriber(synchro, sensors, broker));

    boost::shared_ptr<Profiler> profiler = shared_ptr<Profiler>(new Profiler(&micro_time));
#ifdef USE_DCM
    enactor = shared_ptr<EnactorT>(new EnactorT(profiler, sensors,
                                                transcriber,broker));
#else
    enactor = shared_ptr<EnactorT>(new EnactorT(sensors,synchro,
                                                transcriber,broker));
#endif
    lights = shared_ptr<Lights>(new NaoLights(broker));

    speech = shared_ptr<Speech>(new ALSpeech(broker));

    //setLedsProxy(AL::ALPtr<AL::ALLedsProxy>(new AL::ALLedsProxy(broker)));

    man = boost::shared_ptr<ALMan> (new ALMan(profiler,
                                              sensors,
                                              transcriber,
                                              imageTranscriber,
                                              enactor,
                                              synchro,
                                              lights,
                                              speech));
    man->startSubThreads();
}

void ALDestroyMan(){
    man->stopSubThreads();
}



#ifndef MAN_IS_REMOTE

#ifdef _WIN32
#define ALCALL __declspec(dllexport)
#else
#define ALCALL
#endif

#ifdef __cplusplus
extern "C"
{
#endif

ALCALL int _createModule( ALPtr<ALBroker> pBroker )
{
#ifdef REDIRECT_C_STDERR
  // Redirect stderr to stdout
  FILE *_syderr = stderr;
#ifndef __APPLE__
  stderr = stdout;
#endif
#endif

  // init broker with the main broker inctance
  // from the parent executable
  ALBrokerManager::setInstance(pBroker->fBrokerManager.lock());
  ALBrokerManager::getInstance()->addBroker(pBroker);

  // create modules instance
  //<OGETINSTANCE> don't remove this comment

  //</OGETINSTANCE> don't remove this comment

  //NBites code
  ALCreateMan(pBroker);
  //man  = boost::shared_ptr<Man>(new Man(pBroker,"Man"));
  //man->manStart();

  return 0;
}

ALCALL int _closeModule(  )
{
  // Delete module instance
  //<OKILLINSTANCE> don't remove this comment
  //ALPtr<ALProxy>
  // man  = pBroker->getProxy("Man");
    ALDestroyMan();
  //</OKILLINSTANCE> don't remove this comment

  return 0;
}

# ifdef __cplusplus
}
# endif

#else//MAN_IS_REMOTE
void _terminationHandler( int signum )
{
  //ALPtr<ALProxy> man  = pBroker->getProxy("Man");

  if (signum == SIGINT) {
    // no direct exit, main thread will exit when finished
    cerr << "Exiting Man via thread stop." << endl;
    ALDestroyMan();
  }
  else {
    cerr << "Emergency stop -- exiting immediately" << endl;
    // fault, exit immediately
    ::exit(1);
  }
  ALBrokerManager::getInstance()->killAllBroker();
  ALBrokerManager::kill();
  exit(0);
}


int usage( char* progName )
{
  std::cout << progName <<", a remote module of naoqi !" << std::endl

            << "USAGE :" << std::endl
            << "-b\t<ip> : binding ip of the server. Default is 127.0.0.1" << std::endl
            << "-p\t<port> : binding port of the server. Default is 9559" << std::endl
            << "-pip\t<ip> : ip of the parent broker. Default is 127.0.0.1" << std::endl
            << "-pport\t<ip> : port of the parent broker. Default is 9559" << std::endl
            << "-h\t: Display this help\n" << std::endl;
  return 0;
}

int main( int argc, char *argv[] )
{
#ifdef REDIRECT_C_STDERR
  // Redirect stderr to stdout
  FILE *_syderr = stderr;
  stderr = stdout;
#endif

  std::cout << "..::: starting MANMODULE revision " << MANMODULE_VERSION_REVISION << " :::.." << std::endl;
  std::cout << "Copyright (c) 2007, Aldebaran-robotics" << std::endl << std::endl;

  int  i = 1;
  std::string brokerName = "manmodule";
  std::string brokerIP = "0.0.0.0";
  int brokerPort = 0 ;
  // Default parent broker IP
  std::string parentBrokerIP = "127.0.0.1";
  // Default parent broker port
  int parentBrokerPort = kBrokerPort;

  // checking options
  while( i < argc ) {
    if ( argv[i][0] != '-' ) return usage( argv[0] );
    else if ( std::string( argv[i] ) == "-b" )        brokerIP          = std::string( argv[++i] );
    else if ( std::string( argv[i] ) == "-p" )        brokerPort        = atoi( argv[++i] );
    else if ( std::string( argv[i] ) == "-pip" )      parentBrokerIP    = std::string( argv[++i] );
    else if ( std::string( argv[i] ) == "-pport" )    parentBrokerPort  = atoi( argv[++i] );
    else if ( std::string( argv[i] ) == "-h" )        return usage( argv[0] );
    i++;
  }

  // If server port is not set
  if ( !brokerPort )
    brokerPort = FindFreePort( brokerIP );

  std::cout << "Try to connect to parent Broker at ip :" << parentBrokerIP
            << " and port : " << parentBrokerPort << std::endl;
  std::cout << "Start the server bind on this ip :  " << brokerIP
            << " and port : " << brokerPort << std::endl;

  // Starting Broker
 ALPtr<ALBroker> pBroker = ALBroker::createBroker(brokerName, brokerIP, brokerPort, parentBrokerIP,  parentBrokerPort);
 pBroker->setBrokerManagerInstance(ALBrokerManager::getInstance());


  //<OGETINSTANCE> don't remove this comment
 //ALPtr<Man> manptr = ALModule::createModule<Man>(pBroker,"Man" );
  //</OGETINSTANCE> don't remove this comment

#ifndef _WIN32
  struct sigaction new_action;
  /* Set up the structure to specify the new action. */
  new_action.sa_handler = _terminationHandler;
  sigemptyset( &new_action.sa_mask );
  new_action.sa_flags = 0;

  sigaction( SIGINT, &new_action, NULL );
#endif

  //man = boost::shared_ptr<Man>(new Man(pBroker,"Man"));
  ALCreateMan(pBroker);

  //man->getTrigger()->await_off();
//   //   Not sure what the purpose of this modulegenerator code is: //EDIT -JS
   pBroker.reset(); // because of while( 1 ), broker counted by brokermanager
   while( 1)
   {
     SleepMs( 100 );
   }

  cout << "Main method finished" <<endl;


#ifdef _WIN32
  _terminationHandler( 0 );
#endif

  exit( 0 );
}
#endif//MAN_IS_REMOTE
