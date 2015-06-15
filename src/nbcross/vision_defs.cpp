#include "nbfuncs.h"

#include "RoboGrams.h"
#include "Images.h"
#include "vision/VisionModule.h"
#include "vision/FrontEnd.h"
#include "ParamReader.h"
#include "NBMath.h"

#include <cstdlib>
#include <netinet/in.h>
#include <assert.h>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>

using nblog::Log;
using nblog::SExpr;

man::vision::Colors* getColorsFromSExpr(SExpr* params);
void updateSavedColorParams(std::string sexpPath, SExpr* params, bool top);
SExpr getSExprFromSavedParams(int color, std::string sexpPath, bool top);
std::string getSExprStringFromColorJSonNode(boost::property_tree::ptree tree);
SExpr treeFromBall(man::vision::Ball& b);
SExpr treeFromBlob(man::vision::Blob& b);

int Vision_func() {
    assert(args.size() == 1);

    printf("Vision_func()\n");

    Log* copy = new Log(args[0]);
    size_t length = copy->data().size();
    uint8_t buf[length];
    memcpy(buf, copy->data().data(), length);

    // Parse YUVImage S-expression
    bool topCamera = copy->tree().find("contents")->get(1)->
                                  find("from")->get(1)->value() == "camera_TOP";
    int width = 2*atoi(copy->tree().find("contents")->get(1)->
                                        find("width")->get(1)->value().c_str());
    int height = atoi(copy->tree().find("contents")->get(1)->
                                        find("height")->get(1)->value().c_str());

    // Location of lisp text file with color params
    std::string sexpPath = std::string(getenv("NBITES_DIR"));
    sexpPath += "/src/man/config/colorParams.txt";

    // Read number of bytes of image, inertials, and joints if exist
    messages::JointAngles joints;
    if (copy->tree().find("contents")->get(2)) {
        int numBytes[3];
        for (int i = 0; i < 3; i++)
            numBytes[i] = atoi(copy->tree().find("contents")->get(i+1)->
                                            find("nbytes")->get(1)->value().c_str());
        uint8_t* ptToJoints = buf + (numBytes[0] + numBytes[1]);
        joints.ParseFromArray((void *) ptToJoints, numBytes[2]);
    }

    // Empty messages to pass to Vision Module so it doesn't freak
    messages::YUVImage image(buf, width, height, width);

    // Setup and run module
    portals::Message<messages::YUVImage> imageMessage(&image);
    portals::Message<messages::JointAngles> jointsMessage(&joints);

    // VisionModule default constructor loads color params from Lisp in config/colorParms.txt
    man::vision::VisionModule module;

    module.topIn.setMessage(imageMessage);
    module.bottomIn.setMessage(imageMessage);
    module.jointsIn.setMessage(jointsMessage);

    // If log included color parameters in description, have module use those
    SExpr* params = args[0]->tree().find("Params");
    if (params != NULL) {
        // Set new parameters as frontEnd colorParams
        module.setColorParams(getColorsFromSExpr(params), topCamera);

        // Look for atom value "SaveParams", i.e. "save" button press
        SExpr* save = params->get(1)->find("SaveParams");
        if (save != NULL) {
            // Save attached parameters to txt file
            updateSavedColorParams(sexpPath, params, topCamera);
        }
    }

    module.run();

    // Run and retrieve front end from vision module
    //man::vision::ImageFrontEnd* frontEnd = module.runAndGetFrontEnd(topCamera);

    // -----------
    //   Y IMAGE
    // -----------
    man::vision::ImageFrontEnd* frontEnd = module.getFrontEnd(topCamera);

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

    // Read params from Lisp and attach to image 
    whiteRet->setTree(getSExprFromSavedParams(0, sexpPath, topCamera));

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

    // Read params from JSon and attach to image 
    greenRet->setTree(getSExprFromSavedParams(1, sexpPath, topCamera));

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

    // Read params from JSon and attach to image 
    orangeRet->setTree(getSExprFromSavedParams(2, sexpPath, topCamera));

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
    man::vision::EdgeList* edgeList = module.getEdges(topCamera);

    Log* edgeRet = new Log();
    std::string edgeBuf;

    man::vision::AngleBinsIterator<man::vision::Edge> abi(*edgeList);
    for (const man::vision::Edge* e = *abi; e; e = *++abi) {
        uint32_t x = htonl(e->x() + 160);
        edgeBuf.append((const char*) &x, sizeof(uint32_t));
        uint32_t y = htonl(-e->y() + 120);
        edgeBuf.append((const char*) &y, sizeof(uint32_t));
        uint32_t mag = htonl(e->mag());
        edgeBuf.append((const char*) &mag, sizeof(uint32_t));
        uint32_t angle = htonl(e->angle());
        edgeBuf.append((const char*) &angle, sizeof(uint32_t));
    }

    edgeRet->setData(edgeBuf);
    rets.push_back(edgeRet);

    //-------------------
    //  LINES
    //-------------------
    man::vision::HoughLineList* lineList = module.getHoughLines(topCamera);

    Log* lineRet = new Log();
    std::string lineBuf;

    //std::cout << std::endl << "Hough lines in image coordinates:" << std::endl;

    for (auto it = lineList->begin(); it != lineList->end(); it++) {
        man::vision::HoughLine& line = *it;
        double r = line.r();
        double t = line.t();
        double ep0 = line.ep0();
        double ep1 = line.ep1();
        int houghIndex = line.index();
        int fieldIndex = line.fieldLine();

        // Java uses big endian representation
        endswap<double>(&r);
        endswap<double>(&t);
        endswap<double>(&ep0);
        endswap<double>(&ep1);
        endswap<int>(&houghIndex);
        endswap<int>(&fieldIndex);

        lineBuf.append((const char*) &r, sizeof(double));
        lineBuf.append((const char*) &t, sizeof(double));
        lineBuf.append((const char*) &ep0, sizeof(double));
        lineBuf.append((const char*) &ep1, sizeof(double));
        lineBuf.append((const char*) &houghIndex, sizeof(int));
        lineBuf.append((const char*) &fieldIndex, sizeof(int));

        //std::cout << line.print() << std::endl;
    }

    //std::cout << std::endl << "Hough lines in field coordinates:" << std::endl;
    int i = 0;
    for (auto it = lineList->begin(); it != lineList->end(); it++) {
        man::vision::HoughLine& line = *it;
        //std::cout << line.field().print() << std::endl;
    }

    //std::cout << std::endl << "Field lines:" << std::endl;
    man::vision::FieldLineList* fieldLineList = module.getFieldLines(topCamera);
    for (int i = 0; i < fieldLineList->size(); i++) {
        man::vision::FieldLine& line = (*fieldLineList)[i];
        //std::cout << line.print() << std::endl;
    }

    //std::cout << std::endl << "Goalbox and corner detection:" << std::endl;
    man::vision::GoalboxDetector* box = module.getBox(topCamera);
    man::vision::CornerDetector* corners = module.getCorners(topCamera);
    if (box->first != NULL)
        //std::cout << box->print() << std::endl;
    for (int i = 0; i < corners->size(); i++) {
        const man::vision::Corner& corner = (*corners)[i];
        //std::cout << corner.print() << std::endl;
    }

    lineRet->setData(lineBuf);
    rets.push_back(lineRet);

    //-----------
    //  BALL
    //-----------
    man::vision::BallDetector* detector = module.getBallDetector(topCamera);

    Log* ballRet = new Log();
    std::vector<man::vision::Ball> balls = detector->getBalls();

    SExpr allBalls;
    int count=0;
    for (auto i=balls.begin(); i!=balls.end(); i++) {
        SExpr ballTree = treeFromBall(*i);
        SExpr next = SExpr::keyValue("ball" + std::to_string(count), ballTree);
        allBalls.append(next);
        count++;
    }

    ballRet->setTree(allBalls);
    rets.push_back(ballRet);

    return 0;
}

/* Helper function to convert from SExpr to Colors type.
    Use 18 parameters to intialize Colors struct.
 */
man::vision::Colors* getColorsFromSExpr(SExpr* params) {
    man::vision::Colors* ret = new man::vision::Colors;
    int i, j = 0;

    ret->white.load(std::stof(params->get(1)->get(j++ / 6)->get(1)->get(i++ % 6)->get(1)->serialize()),
                    std::stof(params->get(1)->get(j++ / 6)->get(1)->get(i++ % 6)->get(1)->serialize()),
                    std::stof(params->get(1)->get(j++ / 6)->get(1)->get(i++ % 6)->get(1)->serialize()),
                    std::stof(params->get(1)->get(j++ / 6)->get(1)->get(i++ % 6)->get(1)->serialize()),
                    std::stof(params->get(1)->get(j++ / 6)->get(1)->get(i++ % 6)->get(1)->serialize()),
                    std::stof(params->get(1)->get(j++ / 6)->get(1)->get(i++ % 6)->get(1)->serialize())); 

    ret->green.load(std::stof(params->get(1)->get(j++ / 6)->get(1)->get(i++ % 6)->get(1)->serialize()),
                    std::stof(params->get(1)->get(j++ / 6)->get(1)->get(i++ % 6)->get(1)->serialize()),
                    std::stof(params->get(1)->get(j++ / 6)->get(1)->get(i++ % 6)->get(1)->serialize()),
                    std::stof(params->get(1)->get(j++ / 6)->get(1)->get(i++ % 6)->get(1)->serialize()),
                    std::stof(params->get(1)->get(j++ / 6)->get(1)->get(i++ % 6)->get(1)->serialize()),
                    std::stof(params->get(1)->get(j++ / 6)->get(1)->get(i++ % 6)->get(1)->serialize()));  
 
   ret->orange.load(std::stof(params->get(1)->get(j++ / 6)->get(1)->get(i++ % 6)->get(1)->serialize()),
                    std::stof(params->get(1)->get(j++ / 6)->get(1)->get(i++ % 6)->get(1)->serialize()),
                    std::stof(params->get(1)->get(j++ / 6)->get(1)->get(i++ % 6)->get(1)->serialize()),
                    std::stof(params->get(1)->get(j++ / 6)->get(1)->get(i++ % 6)->get(1)->serialize()),
                    std::stof(params->get(1)->get(j++ / 6)->get(1)->get(i++ % 6)->get(1)->serialize()),
                    std::stof(params->get(1)->get(j++ / 6)->get(1)->get(i++ % 6)->get(1)->serialize()));

    return ret;
}

// Save the new color params to the colorParams.txt file
void updateSavedColorParams(std::string sexpPath, SExpr* params, bool top) {
    std::cout << "Saving params!" << std::endl;
    std::ifstream textFile;
    textFile.open(sexpPath);

    // Get size of file
    textFile.seekg (0, textFile.end);
    long size = textFile.tellg();
    textFile.seekg(0);
    
    // Read file into buffer and convert to string
    char* buff = new char[size];
    textFile.read(buff, size);
    std::string sexpText(buff);

    // Get SExpr from string
    SExpr* savedParams, * savedSExpr = SExpr::read((const std::string)sexpText);
    
    if (top) {
        savedParams = savedSExpr->get(1)->find("Top");
    } else {
        savedParams = savedSExpr->get(1)->find("Bottom");
    }

    // Remove "SaveParams True" pair from expression
    params->get(1)->remove(3);

    const std::vector<SExpr>& newParams = *params->get(1)->getList();
    savedParams->get(1)->setList(newParams);
       
    // Write out
    size = savedSExpr->print().length();
    char* buffer = new char[size + 1];
    std::strcpy(buffer, savedSExpr->print().c_str());
    std::ofstream out;
    out.open(sexpPath);
    out.write(buffer, savedSExpr->print().length());

    delete[] buff;
    delete[] buffer;
    textFile.close();
    out.close();
}

SExpr getSExprFromSavedParams(int color, std::string sexpPath, bool top) {
    std::ifstream textFile;
    textFile.open(sexpPath);

    // Get size of file
    textFile.seekg (0, textFile.end);
    long size = textFile.tellg();
    textFile.seekg(0);

    // Read file into buffer and convert to string
    char* buff = new char[size];
    textFile.read(buff, size);
    std::string sexpText(buff);

    // Get SExpr from string
    SExpr* savedSExpr = SExpr::read((const std::string)sexpText);

    // Point to required set of 6 params
    if (top)
        savedSExpr = savedSExpr->get(1)->find("Top");
    else
        savedSExpr = savedSExpr->get(1)->find("Bottom");

    if (color == 0)                                         // White
        savedSExpr = savedSExpr->get(1)->find("White");
    else if (color == 1)                                    // Green
        savedSExpr = savedSExpr->get(1)->find("Green");
    else                                                    // Orange
        savedSExpr = savedSExpr->get(1)->find("Orange");


    // Build SExpr from params
    std::vector<SExpr> atoms;
    for (SExpr s : *(savedSExpr->getList()))
        atoms.push_back(s);

    return SExpr(atoms);
}

SExpr treeFromBall(man::vision::Ball& b)
{
    SExpr x(b.x_rel);
    SExpr y(b.y_rel);
    SExpr p = SExpr::list({x, y});
    SExpr bl = treeFromBlob(b.getBlob());

    SExpr rel = SExpr::keyValue("rel", p);
    SExpr blob = SExpr::keyValue("blob", bl);
    SExpr exDiam = SExpr::keyValue("expectedDiam", b.expectedDiam);
    SExpr toRet = SExpr::list({rel, blob, exDiam});

    return toRet;
}

SExpr treeFromBlob(man::vision::Blob& b)
{
    SExpr x(b.centerX());
    SExpr y(b.centerY());
    SExpr p = SExpr::list({x, y});

    SExpr center = SExpr::keyValue("center", p);
    SExpr area = SExpr::keyValue("area", b.area());
    SExpr count = SExpr::keyValue("count", b.count());
    SExpr len1 = SExpr::keyValue("len1", b.firstPrincipalLength());
    SExpr len2 = SExpr::keyValue("len2", b.secondPrincipalLength());
    SExpr ang1 = SExpr::keyValue("ang1", b.firstPrincipalAngle());
    SExpr ang2 = SExpr::keyValue("ang2", b.secondPrincipalAngle());
    SExpr toRet = SExpr::list({center, area, count, len1, len2, ang1, ang2});

    return toRet;
}

int Synthetics_func() {
    assert(args.size() == 1);

    printf("Synthetics_func()\n");

    double x = std::stod(args[0]->tree().find("contents")->get(1)->find("params")->get(1)->value().c_str());
    double y = std::stod(args[0]->tree().find("contents")->get(1)->find("params")->get(2)->value().c_str());
    double h = std::stod(args[0]->tree().find("contents")->get(1)->find("params")->get(3)->value().c_str());
    bool fullres = args[0]->tree().find("contents")->get(1)->find("params")->get(4)->value() == "true";
    bool top = args[0]->tree().find("contents")->get(1)->find("params")->get(5)->value() == "true";

    int wd = (fullres ? 320 : 160);
    int ht = (fullres ? 240 : 120);
    double flen = (fullres ? 544 : 272);

    int size = wd*4*ht*2;
    uint8_t* pixels = new uint8_t[size];
    man::vision::YuvLite synthetic(wd, ht, wd*4, pixels);

    man::vision::FieldHomography homography(top);
    homography.wx0(x);
    homography.wy0(y);
    homography.azimuth(h*TO_RAD);
    homography.flen(flen);

    man::vision::syntheticField(synthetic, homography);

    std::string sexpr("(nblog (version 6) (contents ((type YUVImage) (from camera_TOP) (nbytes ");
    sexpr += std::to_string(size);
    sexpr += ") (width " + std::to_string(2*wd);
    sexpr += ") (height " + std::to_string(2*ht);
    sexpr += ") (encoding \"[Y8(U8/V8)]\"))))";

    Log* log = new Log(sexpr);
    std::string buf((const char*)pixels, size);
    log->setData(buf);

    rets.push_back(log);
}
