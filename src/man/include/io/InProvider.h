/**
 * @class InProvider
 *
 * An abstract class that has the only role of providing means of reading
 * from a file descriptor
 *
 * @author Octavian Neamtu
 */


#pragma once

#include <boost/shared_ptr.hpp>
#include <iostream>
#include <unistd.h>
#include "ClassHelper.h"
#include "IOProvider.h"

namespace common {
namespace io {

class InProvider : public IOProvider {

ADD_SHARED_PTR(InProvider);

public:
    InProvider() {}
    virtual ~InProvider() {};

    virtual std::string debugInfo() const = 0;
    virtual bool rewind(uint64_t offset) const = 0;
    virtual void openCommunicationChannel() = 0;
    virtual bool opened() const = 0;
    //return false because read() blocks until it finishes
    virtual bool readInProgress() const {return false;}
    virtual bool isOfTypeStreaming() const = 0;

    virtual void peekAt(char* buffer, uint32_t size) const = 0;

    template<class T>
    T peekAndGet() const {
        T value;
        peekAt(reinterpret_cast<char *>(&value), sizeof(value));
        return value;
    }

    virtual bool readCharBuffer(char* buffer, uint32_t size) const = 0;

    template <class T>
    T readValue() const {
        T value;
        readCharBuffer(reinterpret_cast<char *>(&value), sizeof(value));
        return value;
    }

};

}
}
