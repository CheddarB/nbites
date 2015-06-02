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

int ImageConverter_func() {
    assert(args.size() == 1);

    printf("ImageConverter_func()\n");
    printf("******* If you want to use a color table, define in nbcross/ched_defs.cpp *******\n");

    int width = 640;
    int height = 480;
    int pitch = 640;

    Log * copy = new Log(args[0]);
    size_t length = copy->data().size();
    uint8_t buf[length];
    memcpy(buf, copy->data().data(), length);
    
    messages::YUVImage image(buf, width, height, pitch);
    portals::Message<messages::YUVImage> message(&image);
    man::vision::VisionModule module;

    module.bottomIn.setMessage(message);
    module.run();
    man::vision::ImageFrontEnd* frontEnd = module.getFrontEnd();

    // -----------
    //   Y IMAGE
    // -----------
    Log* yRet = new Log();
    int yLength = 240*320*2;

    // Create temp buffer and fill with yImage 
    uint8_t yBuf[yLength];
    memcpy(yBuf, frontEnd->yImage().pixelAddr(), yLength);

    // Convert to string and set log
    std::string yBuffer((const char*)yBuf, yLength);
    yRet->setData(yBuffer);

    rets.push_back(yRet);

    // ---------------
    //   WHITE IMAGE
    // ---------------
    // Log* whiteRet = new Log();
    // int whiteLength = 240*320;

    // // Get white image from module message
    // const messages::PackedImage<unsigned char>* whiteImage = module.whiteImage.getMessage(true).get();

    // // Create temp buffer and fill with white image 
    // uint8_t whiteBuf[whiteLength];
    // memcpy(whiteBuf, whiteImage->pixelAddress(0, 0), whiteLength);

    // // Convert to string and set log
    // std::string whiteBuffer((const char*)whiteBuf, whiteLength);
    // whiteRet->setData(whiteBuffer);

    // rets.push_back(whiteRet);

    // // ---------------
    // //   GREEN IMAGE
    // // ---------------
    // Log* greenRet = new Log();
    // int greenLength = 240*320;

    // // Get gree image from module message
    // const messages::PackedImage<unsigned char>* greenImage = module.greenImage.getMessage(true).get();

    // // Create temp buffer and fill with gree image 
    // uint8_t greenBuf[greenLength];
    // memcpy(greenBuf, greenImage->pixelAddress(0, 0), greenLength);

    // // Convert to string and set log
    // std::string greenBuffer((const char*)greenBuf, greenLength);
    // greenRet->setData(greenBuffer);

    // rets.push_back(greenRet);

    // // ----------------
    // //   ORANGE IMAGE
    // // ----------------
    // Log* orangeRet = new Log();
    // int orangeLength = 240*320;

    // // Get orange image from module message
    // const messages::PackedImage<unsigned char>* orangeImage = module.orangeImage.getMessage(true).get();

    // // Create temp buffer and fill with orange image 
    // uint8_t orangeBuf[orangeLength];
    // memcpy(orangeBuf, orangeImage->pixelAddress(0, 0), orangeLength);

    // // Convert to string and set log
    // std::string orangeBuffer((const char*)orangeBuf, orangeLength);
    // orangeRet->setData(orangeBuffer);

    // rets.push_back(orangeRet);

    // //-------------------
    // //  SEGMENTED IMAGE
    // //-------------------
    // Log* colorSegRet = new Log();
    // int colorSegLength = 240*320;

    // // Get segmented image from module message
    // const messages::PackedImage<unsigned char>* colorSegImage = module.thrImage.getMessage(true).get();

    // // Create temp buffer and fill with segmented image
    // uint8_t segBuf[colorSegLength];
    // memcpy(segBuf, colorSegImage->pixelAddress(0, 0), colorSegLength);

    // // Convert to string and set log
    // std::string segBuffer((const char*)segBuf, colorSegLength);
    // colorSegRet->setData(segBuffer);

    // rets.push_back(colorSegRet);

    // Done
    return 0;
}

int Edges_func() {
    assert(args.size() == 1);
    printf("Edges_func()\n");

    size_t length = args[0]->data().size();
    uint8_t buf[length];
    memcpy(buf, args[0]->data().data(), length);

    messages::YUVImage image(buf, 640, 480, 640);
    portals::Message<messages::YUVImage> imageMessage(&image);

    messages::YUVImage emptyImage;
    messages::JointAngles emptyJoints;
    messages::InertialState emptyInertials;

    portals::Message<messages::YUVImage> emptyImageMessage(&emptyImage);
    portals::Message<messages::JointAngles> emptyJointsMessage(&emptyJoints);
    portals::Message<messages::InertialState> emptyInertialsMessage(&emptyInertials);

    man::vision::VisionModule module = man::vision::VisionModule();
    module.topIn.setMessage(emptyImageMessage);
    module.bottomIn.setMessage(imageMessage);
    module.jointsIn.setMessage(emptyJointsMessage);
    module.inertialsIn.setMessage(emptyInertialsMessage);

    module.run();
    man::vision::EdgeList* edgeList = module.getEdges();

    Log* edges = new Log();
    std::string data;

    // TODO convert from relative to image center to normal image
    std::cout << "Number of edges: " << edgeList->count() << std::endl;
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
