/**
 *
 * @class DataManager
 *
 * This class should handle the notifying all subscribers to new data
 * coming into the system (be it online or offline)
 *
 * @author Octavian Neamtu
 *
 */

#pragma once

#include "DataHandler.h"
#include "memory/Memory.h"
#include "include/Provider.h"
#include "boost/shared_ptr.hpp"

namespace qtool {
namespace data {

class DataManager : public Provider<int> {

public:
    DataManager();

    void getNext() const {
        dataHandler->readNext();
        this->notify(0);
    }

    boost::shared_ptr<const memory::Memory> getMemory() const {
        return memory;}

private:
    boost::shared_ptr<memory::Memory> memory;
    DataHandler* dataHandler;

};

}
}
