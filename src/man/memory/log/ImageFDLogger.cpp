/**
 * ImageFDLogger.cpp
 *
 *  The structure for a log file:
 *  -- ID number for the type of object logged
 *  -- the birth_time timestamp of the man process that parents the logger
 *
 *  -- for each message
 *  ---- size of serialized message
 *  ---- serialized message
 *
 *      Author: Octavian Neamtu
 *      E-mail: oneamtu@bowdoin.edu
 */

#include <fcntl.h>
#include <unistd.h>
#include <iostream>

#include "Common.h"
#include "ImageFDLogger.h"
#include "ImageAcquisition.h"

namespace memory {

namespace log {

extern long long birth_time;

using namespace std;

ImageFDLogger::ImageFDLogger(string output_file_descriptor,
                             int logTypeID,
                             RoboImage* roboImage) :
        FDLogger(output_file_descriptor.data()),
        current_buffer(new (void*)),
        current_buffer_size(1),
        bytes_written(0),
        logID(logTypeID),
        roboImage(roboImage)
        {
    raw_output = new FileOutputStream(file_descriptor, roboImage->getByteSize());
    cout << raw_output->GetErrno() << endl;
    writeHead();
}

void ImageFDLogger::writeHead() {
//	this->getNextBuffer();
//    // this helps us ID the log
//    *((int*) (*current_buffer)) = logID;
//    // this time stamps the log
//    *((long long*) (*current_buffer) + sizeof(int)) = birth_time;
//
//    raw_output->BackUp(current_buffer_size - sizeof(int) - sizeof(long long));
}

void ImageFDLogger::write() {

    this->getNextBuffer();
    _copy_image(const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(roboImage->getImage())),
    		reinterpret_cast<uint8_t*> (*current_buffer));

}

uint8_t* ImageFDLogger::getCurrentImage() {
	return reinterpret_cast<uint8_t*> (*current_buffer);
}

byte* ImageFDLogger::getNextBuffer() {

    raw_output->Next(current_buffer, &current_buffer_size);
    //we're not guaranteed a non-empty buffer
    //but we're guaranteed one eventually
    while (current_buffer_size == 0) {
        raw_output->Next(current_buffer, &current_buffer_size);
    }
}

ImageFDLogger::~ImageFDLogger() {

    raw_output->Close();
    delete raw_output;
    close(file_descriptor);
}
}
}
