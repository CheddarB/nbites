#include "nbfuncs.h"

#include "RoboGrams.h"
#include "Images.h"
#include "vision/VisionModule.h"

#include <netinet/in.h>
#include <assert.h>
#include <vector>
#include <string>
#include <iostream>

using nblog::Log;
using nblog::SExpr;

int Vision_func() {
    assert(args.size() == 1);

    printf("Vision_func()\n");

    Log* copy = new Log(args[0]);
    size_t length = copy->data().size();
    uint8_t buf[length];
    memcpy(buf, copy->data().data(), length);

    int width = 2*640;
    int height = 480;
    
    messages::YUVImage image(buf, width, height, width);
    messages::JointAngles emptyJoints;
    messages::InertialState emptyInertials;

    portals::Message<messages::YUVImage> imageMessage(&image);
    portals::Message<messages::JointAngles> emptyJointsMessage(&emptyJoints);
    portals::Message<messages::InertialState> emptyInertialsMessage(&emptyInertials);

    man::vision::VisionModule module;

    module.topIn.setMessage(imageMessage);
    module.bottomIn.setMessage(imageMessage);
    module.jointsIn.setMessage(emptyJointsMessage);
    module.inertialsIn.setMessage(emptyInertialsMessage);

    module.run();

    // -----------
    //   Y IMAGE
    // -----------
    man::vision::ImageFrontEnd* frontEnd = module.getFrontEnd();

    Log* yRet = new Log();
    int yLength = 240*320*2;

    // Create temp buffer and fill with yImage from FrontEnd
    uint8_t yBuf[yLength];
    memcpy(yBuf, frontEnd->yImage().pixelAddr(), yLength);

    // Convert to string and set log
    std::string yBuffer((const char*)yBuf, yLength);
    yRet->setData(yBuffer);

    rets.push_back(yRet);

    // ---------------
    //   WHITE IMAGE
    // ---------------
    Log* whiteRet = new Log();
    int whiteLength = 240*320;;

    // Create temp buffer and fill with white image 
    uint8_t whiteBuf[whiteLength];
    memcpy(whiteBuf, frontEnd->whiteImage().pixelAddr(), whiteLength);

    // Convert to string and set log
    std::string whiteBuffer((const char*)whiteBuf, whiteLength);
    whiteRet->setData(whiteBuffer);

    rets.push_back(whiteRet);

    // ---------------
    //   GREEN IMAGE
    // ---------------
    Log* greenRet = new Log();
    int greenLength = 240*320;

    // Create temp buffer and fill with gree image 
    uint8_t greenBuf[greenLength];
    memcpy(greenBuf, frontEnd->greenImage().pixelAddr(), greenLength);

    // Convert to string and set log
    std::string greenBuffer((const char*)greenBuf, greenLength);
    greenRet->setData(greenBuffer);

    rets.push_back(greenRet);

    // ----------------
    //   ORANGE IMAGE
    // ----------------
    Log* orangeRet = new Log();
    int orangeLength = 240*320;

    // Create temp buffer and fill with orange image 
    uint8_t orangeBuf[orangeLength];
    memcpy(orangeBuf, frontEnd->orangeImage().pixelAddr(), orangeLength);

    // Convert to string and set log
    std::string orangeBuffer((const char*)orangeBuf, orangeLength);
    orangeRet->setData(orangeBuffer);

    rets.push_back(orangeRet);

    //-------------------
    //  SEGMENTED IMAGE
    //-------------------
    Log* colorSegRet = new Log();
    int colorSegLength = 240*320;

    // Create temp buffer and fill with segmented image
    uint8_t segBuf[colorSegLength];
    memcpy(segBuf, frontEnd->colorImage().pixelAddr(), colorSegLength);

    // Convert to string and set log
    std::string segBuffer((const char*)segBuf, colorSegLength);
    colorSegRet->setData(segBuffer);

    rets.push_back(colorSegRet);
    
    //-------------------
    //  EDGES
    //-------------------
    man::vision::EdgeList* edgeList = module.getEdges();

    Log* edges = new Log();
    std::string data;

    // TODO convert from relative to image center to normal image
    man::vision::AngleBinsIterator<man::vision::Edge> abi(*edgeList);
    for (const man::vision::Edge* e = *abi; e; e = *++abi) {
        uint32_t x = htonl(e->x());
        data.append((const char*) &x, 4);
        uint32_t y = htonl(e->y());
        data.append((const char*) &y, 4);
        uint32_t mag = htonl(e->mag());
        data.append((const char*) &mag, 4);
        uint32_t angle = htonl(e->angle());
        data.append((const char*) &angle, 4);
    }

    edges->setData(data);
    rets.push_back(edges);

    return 0;
}
