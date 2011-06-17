#include "LoggingBoard.h"
#include "NaoPaths.h"

namespace memory {

namespace log {

const char* LoggingBoard::MVISION_PATH = NAO_LOG_DIR "/Vision.log";
const char* LoggingBoard::MMOTION_SENSORS_PATH = NAO_LOG_DIR "/MotionSensors.log";
const char* LoggingBoard::MVISION_SENSORS_PATH = NAO_LOG_DIR "/VisionSensors.log";
const char* LoggingBoard::MIMAGE_PATH = NAO_LOG_DIR "/Image.log";

LoggingBoard::LoggingBoard() {
}

LoggingBoard::LoggingBoard(const Memory* _memory) :
    memory(_memory) {
    initLoggingObjects();
}

LoggingBoard::~LoggingBoard() {
    for (ObjectFDProviderMap::iterator it = objectFDProviderMap.begin();
            it != objectFDProviderMap.end(); it++) {
        delete (*it).second;
    }
    objectFDProviderMap.clear();
    for (ObjectLoggerMap::iterator it = objectLoggerMap.begin();
            it != objectLoggerMap.end(); it++) {
        delete (*it).second;
    }
    objectLoggerMap.clear();
}

LoggingBoard* LoggingBoard::NullLoggingBoard() {
    //not thread-safe
    return new LoggingBoard();
}

void LoggingBoard::initLoggingObjects() {

    const MVision* mvision = memory->getMVision();
    FDProvider* mvisionFDprovider = new FileFDProvider(MVISION_PATH);
    objectFDProviderMap[mvision] = mvisionFDprovider;
    objectLoggerMap[mvision] = new CodedFileLogger(mvisionFDprovider,
            MVISION_ID, mvision);

    const MMotionSensors* mmotionSensors = memory->getMMotionSensors();
    FDProvider* mmotionSensorsFDprovider = new FileFDProvider(
            MMOTION_SENSORS_PATH);
    objectFDProviderMap[mmotionSensors] = mmotionSensorsFDprovider;
    objectLoggerMap[mmotionSensors] = new CodedFileLogger(
            mmotionSensorsFDprovider, MMOTION_SENSORS_ID, mmotionSensors);

    const MVisionSensors* mvisionSensors = memory->getMVisionSensors();
    FDProvider* mvisionSensorsFDprovider = new FileFDProvider(
            MVISION_SENSORS_PATH);
    objectFDProviderMap[mvisionSensors] = mvisionSensorsFDprovider;
    objectLoggerMap[mvisionSensors] = new CodedFileLogger(
            mvisionSensorsFDprovider, MVISION_SENSORS_ID, mvisionSensors);

    const MImage* mimage = memory->getMImage();
    FDProvider* mimageFDprovider = new FileFDProvider(MIMAGE_PATH);
    objectFDProviderMap[mimage] = mimageFDprovider;
    objectLoggerMap[mimage] = new ImageFDLogger(mimageFDprovider,
            MIMAGE_ID, mimage);
}

void LoggingBoard::log(const MObject* mobject) {

    //TODO:Octavian use getLogger
    ObjectLoggerMap::iterator it = objectLoggerMap.find(mobject);
    // if this is true, then we found a legitimate logger
    // corresponding to our mobject in the map
    if (it != objectLoggerMap.end()) {
        //it->second is the logger associated with the specified mobject
        it->second->write();
    }
}

const ImageFDLogger* LoggingBoard::getImageLogger(const MImage* mimage) const {
    return dynamic_cast<const ImageFDLogger*>(this->getLogger(mimage));
}

const FDLogger* LoggingBoard::getLogger(const MObject* mobject) const {
    ObjectLoggerMap::const_iterator it = objectLoggerMap.find(mobject);
    // if this is true, then we found a legitimate logger
    // corresponding to our mobject in the map
    if (it != objectLoggerMap.end()) {
        return it->second;
    } else {
        return NULL;
    }
}


}

}
