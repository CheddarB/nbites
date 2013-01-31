#include "NewMan.h"
#include <iostream>

#include "jointenactor/JointEnactorModule.h"

namespace man {

Man::Man(boost::shared_ptr<AL::ALBroker> broker, const std::string &name)
    : AL::ALModule(broker, name)
{
    setModuleDescription("The Northern Bites' soccer player.");
    std::cout << "Man constructor." << std::endl;

    jointenactor::JointEnactorModule jem(broker);
    jem.run();
}

Man::~Man()
{
    std::cout << "Man destructor." << std::endl;
}

}
