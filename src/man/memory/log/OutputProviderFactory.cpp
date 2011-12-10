
#include <string>

#include <iostream>

#include "OutputProviderFactory.h"
#include "NaoPaths.h"
#include "io/FileOutProvider.h"
#include "io/SocketOutProvider.h"
#include "memory/MemoryCommon.h"
#include "CommDef.h"

using namespace std;
using namespace common::io;
using namespace common::paths;

namespace man {
namespace memory {
namespace log {

void OutputProviderFactory::AllFileOutput(LoggingBoard::ptr loggingBoard) {
	for (MObject_ID id = FIRST_OBJECT_ID; id != LAST_OBJECT_ID; id++) {
		string file_name = NAO_LOG_DIR +
				MObject_names[static_cast<int>(id)] + NAO_LOG_EXTENSION;
		loggingBoard->newOutputProvider(
				OutProvider::ptr(new FileOutProvider(file_name)), id);
	}
}

void OutputProviderFactory::AllSocketOutput(LoggingBoard::ptr loggingBoard) {
    for (MObject_ID id = FIRST_OBJECT_ID; id != LAST_OBJECT_ID; id++) {

        loggingBoard->newOutputProvider(
                OutProvider::ptr(new SocketOutProvider(STREAMING_PORT_BASE +
                        static_cast<short>(id))), id);
    }
}

}
}
}
