//
//  nbfuncs.cpp
//  nbcross
//
//  Created by Philip Koch on 11/28/14.
//

#include "nbfuncs.h"
#include <assert.h>
#include <string.h>
#include <sstream>

#include "Images.h"
#include "image/ImageConverterModule.h"
#include "vision/Blobber.h"
#include "vision/Blob.h"
#include "vision/BallDetector.h"
#include "vision/PostDetector.h"
#include "RoboGrams.h"

std::vector<nbfunc_t> FUNCS;

std::vector<logio::log_t> args;
std::vector<logio::log_t> rets;

//Common arg types -- used to check arg types and for human readability.
const char sYUVImage[] = "YUVImage";
const char sParticleSwarm_pbuf[] = "ParticleSwarm";
const char sParticle_pbuf[] = "Particle";
const char sTest[] = "test";

const char stext[] = "text";//No current sources for this data type.

int test_func() {
    assert(args.size() == 2);
    for (int i = 0; i < args.size(); ++i) {
        printf("test_func(): %s\n", args[i].desc);
    }
    
    return 0;
}

int arg_test_func() {
    printf("arg_test_func()\n");
    assert(args.size() == 2);
    for (int i = 0; i < 2; ++i) {
        printf("\t%s\n", args[i].desc);
        rets.push_back(logio::copyLog(&args[i]));
    }
    
    return 0;
}

int CrossBright_func() {
    assert(args.size() == 1);
    printf("CrossBright_func()\n");
    //work on a copy of the arg so we can safely push to rets.
    logio::log_t log = logio::copyLog(&args[0]);
    for (int i = 0; i < log.dlen; i += 2) {
        *(log.data + i) = 240;
    }
    
    printf("[%s] modified.\n", log.desc);
    rets.push_back(log);
    
    return 0;
}

int BlobTest_func() {
    assert(args.size() == 1);
    printf("BlobTest_func()\n");

    const logio::log_t log = args[0];

    // Hardcoded for now. TODO
    int width = 640;
    int height = 480;

    messages::YUVImage image(log.data, width, height, width);
    portals::Message<messages::YUVImage> message(&image);
    man::image::ImageConverterModule module;

    module.imageIn.setMessage(message);
    module.run();

    const messages::PackedImage8* orangeImage = module.orangeImage.getMessage(true).get();

    man::vision::Blobber<uint8_t> b(orangeImage->pixelAddress(0, 0), orangeImage->width(),
                                    orangeImage->height(), 1, orangeImage->width());

    b.run(man::vision::NeighborRule::eight, 90, 100, 100, 100);

    logio::log_t ret1;

    std::string name = "type=YUVImage encoding=[Y16] width=320 height=240";

    ret1.desc = (char*)malloc(name.size() + 1);
    memcpy(ret1.desc, name.c_str(), name.size() + 1);

    ret1.dlen = 320 * 240 * sizeof(short unsigned int);
    ret1.data = (uint8_t*)malloc(640*480* sizeof(short unsigned int));
    memcpy(ret1.data, b.getImage(), ret1.dlen);

    std::vector<man::vision::Blob> results = b.getResult();

    for(int i=0; i<results.size(); i++){
        man::vision::Blob found = results.at(i);
        printf("Blob of size:%f, centered at:(%f, %f), with lengths: %f, %f\n",
               found.area(), found.xCenter(), found.yCenter(),
               found.principalLength1(), found.principalLength2());
    }

    rets.push_back(ret1);
}

int BallImage_func() {
    assert(args.size() == 1);
    printf("BallImage_func()\n");

    logio::log_t log = logio::copyLog(&args[0]);

    int width = 640;
    int height = 480;

    messages::YUVImage image(log.data, width, height, width);
    portals::Message<messages::YUVImage> message(&image);
    man::image::ImageConverterModule module;

    module.imageIn.setMessage(message);
    module.run();

    const messages::PackedImage8* orangeImage = module.orangeImage.getMessage(true).get();

    man::vision::BallDetector detector(orangeImage);
    std::vector<std::pair<man::vision::Circle ,double> > balls = detector.findBalls();
    printf("found: %d balls!\n", balls.size());
    std::string name = "type=YUVImage encoding=[Ball] width=640 height=480";

    for(int i=0; i<balls.size(); i++) {
        std::pair<man::vision::Circle, double> ball = balls.at(i);
        std::stringstream stream;
        stream << " ball" << i << "={" << ball.first.center.x * 2 << "," << ball.first.center.y * 2 << ",";
        stream << ball.first.radius * 2 << "," << ball.second << "}";
        name += stream.str();
        printf("   %s\n", stream.str().c_str());
    }

    free(log.desc);
    log.desc = (char*)malloc(name.size() + 1);
    memcpy(log.desc, name.c_str(), name.size() +1);

    rets.push_back(log);
}

int ImageConverter_func() {
    assert(args.size() == 1);

    printf("ImageConverter_func()\n");

    logio::log_t arg1 = args[0];
    std::vector<std::string> kvp = logio::pairs(arg1.desc);
    int width = 640;
    int height = 480;

    messages::YUVImage image(arg1.data, width, height, width);
    portals::Message<messages::YUVImage> message(&image);
    man::image::ImageConverterModule module;

    module.imageIn.setMessage(message);
    module.run();

    // Y image name and data
    const messages::PackedImage<short unsigned int>* yImage = module.yImage.getMessage(true).get();
    logio::log_t yRet;

    std::string yName = "type=YUVImage encoding=[Y16] width=";
    yName += std::to_string(yImage->width());
    yName += " height=";
    yName += std::to_string(yImage->height());

    yRet.desc = (char*)malloc(yName.size()+1);
    memcpy(yRet.desc, yName.c_str(), yName.size() + 1);

    yRet.dlen = yImage->width() * yImage->height() * 2;
    yRet.data = (uint8_t*)malloc(yRet.dlen);
    memcpy(yRet.data, yImage->pixelAddress(0, 0), yRet.dlen);

    rets.push_back(yRet);

    // White image retreval, description, and data
    const messages::PackedImage<unsigned char>* whiteImage = module.whiteImage.getMessage(true).get();
    logio::log_t whiteRet;

    std::string whiteName = "type=YUVImage encoding=[Y8] width=";
    whiteName += std::to_string(whiteImage->width());
    whiteName += " height=";
    whiteName += std::to_string(whiteImage->height());

    whiteRet.desc = (char*)malloc(whiteName.size()+1);
    memcpy(whiteRet.desc, whiteName.c_str(), whiteName.size()+1);

    whiteRet.dlen = whiteImage->width() * whiteImage->height();
    whiteRet.data = (uint8_t*)malloc(whiteRet.dlen);
    memcpy(whiteRet.data, whiteImage->pixelAddress(0, 0), whiteRet.dlen);

    rets.push_back(whiteRet);

    // Orange image retreval, description, and data
    const messages::PackedImage<unsigned char>* orangeImage = module.orangeImage.getMessage(true).get();
    logio::log_t orangeRet;

    std::string orangeName = "type=YUVImage encoding=[Y8] width=";
    orangeName += std::to_string(orangeImage->width());
    orangeName += " height=";
    orangeName += std::to_string(orangeImage->height());

    orangeRet.desc = (char*)malloc(orangeName.size()+1);
    memcpy(orangeRet.desc, orangeName.c_str(), orangeName.size()+1);

    orangeRet.dlen = orangeImage->width() * orangeImage->height();
    orangeRet.data = (uint8_t*)malloc(orangeRet.dlen);
    memcpy(orangeRet.data, orangeImage->pixelAddress(0, 0), orangeRet.dlen);

    rets.push_back(orangeRet);

    // Green image retreval, description, and data
    const messages::PackedImage<unsigned char>* greenImage = module.greenImage.getMessage(true).get();
    logio::log_t greenRet;

    std::cout << "Green width: " << greenImage->width() <<  std::endl;

    std::string greenName = "type=YUVImage encoding=[Y8] width=";
    greenName += std::to_string(greenImage->width());
    greenName += " height=";
    greenName += std::to_string(greenImage->height());

    greenRet.desc = (char*)malloc(greenName.size()+1);
    memcpy(greenRet.desc, greenName.c_str(), greenName.size()+1);

    greenRet.dlen = greenImage->width() * greenImage->height();
    greenRet.data = (uint8_t*)malloc(greenRet.dlen);
    memcpy(greenRet.data, greenImage->pixelAddress(0, 0), greenRet.dlen);

    rets.push_back(greenRet);
    
    // Done
    printf("ImageConverter module ran! W: %d, H: %d\n", yImage->width(), yImage->height());
    return 0;
}

int PostDetector_func() {
    assert(args.size() == 1);
    printf("PostDetector_func()\n");

    logio::log_t log = logio::copyLog(&args[0]);

    int width = 640;
    int height = 480;

    messages::YUVImage image(log.data, width, height, width);
    portals::Message<messages::YUVImage> message(&image);
    man::image::ImageConverterModule imageConverter;

    imageConverter.imageIn.setMessage(message);
    imageConverter.run();

    const messages::PackedImage16* yImage = imageConverter.yImage.getMessage(true).get();
    const messages::PackedImage8* whiteImage = imageConverter.orangeImage.getMessage(true).get();

    man::vision::PostDetector detector(*yImage, *whiteImage);
    const std::vector<int>& posts = detector.getCandidates();
    printf("Found %d candidate posts.\n", posts.size());

    for(int i = 0; i < posts.size(); i++)
        printf("Found post at %d column.\n", posts[i]);

    const messages::PackedImage8& postImage(detector.getPostImage());
    logio::log_t postImageRet;

    std::string postImageDesc = "type=YUVImage encoding=[Y8] width=";
    postImageDesc += std::to_string(postImage.width());
    postImageDesc += " height=";
    postImageDesc += std::to_string(postImage.height());

    postImageRet.desc = (char*)malloc(postImageDesc.size()+1);
    memcpy(postImageRet.desc, postImageDesc.c_str(), postImageDesc.size()+1);

    postImageRet.dlen = postImage.width() * postImage.height();
    postImageRet.data = (uint8_t*)malloc(postImageRet.dlen);
    memcpy(postImageRet.data, postImage.pixelAddress(0, 0), postImageRet.dlen);

    rets.push_back(postImageRet);

    // TODO fix saving old histogram bug
    logio::log_t unfiltHistRet;
    std::string unfiltHistDesc = "type=Histogram";

    unfiltHistRet.desc = (char*)malloc(unfiltHistDesc.size()+1);
    memcpy(unfiltHistRet.desc, unfiltHistDesc.c_str(), unfiltHistDesc.size()+1);

    unfiltHistRet.dlen = 8 * detector.getLengthOfHistogram();
    unfiltHistRet.data = (uint8_t*)malloc(unfiltHistRet.dlen);
    memcpy(unfiltHistRet.data, detector.getUnfilteredHistogram(), unfiltHistRet.dlen);

    rets.push_back(unfiltHistRet);

    logio::log_t filtHistRet;
    std::string filtHistDesc = "type=Histogram";

    filtHistRet.desc = (char*)malloc(filtHistDesc.size()+1);
    memcpy(filtHistRet.desc, filtHistDesc.c_str(), filtHistDesc.size()+1);

    filtHistRet.dlen = 8 * detector.getLengthOfHistogram();
    filtHistRet.data = (uint8_t*)malloc(filtHistRet.dlen);
    memcpy(filtHistRet.data, detector.getFilteredHistogram(), filtHistRet.dlen);

    rets.push_back(filtHistRet);
}

void register_funcs() {
    
    /*test func 1*/
    nbfunc_t test;
    test.name = (const char *) "simple test";
    test.args = {sTest, sTest};
    test.func = test_func;
    FUNCS.push_back(test);
    
    /*test func 2*/
    nbfunc_t arg_test;
    arg_test.name = (char *) "arg test";
    arg_test.args = {sYUVImage, sYUVImage};
    arg_test.func = arg_test_func;
    FUNCS.push_back(arg_test);
    
    //CrossBright
    nbfunc_t CrossBright;
    CrossBright.name = "CrossBright";
    CrossBright.args = {sYUVImage};
    CrossBright.func = CrossBright_func;
    FUNCS.push_back(CrossBright);

    //BlobTest
    nbfunc_t BlobTest;
    BlobTest.name = "BlobTest";
    BlobTest.args = {sYUVImage};
    BlobTest.func = BlobTest_func;
    FUNCS.push_back(BlobTest);

    //BallImage
    nbfunc_t BallImage;
    BallImage.name = "BallImage";
    BallImage.args = {sYUVImage};
    BallImage.func = BallImage_func;
    FUNCS.push_back(BallImage);

    //ImageConverter
    nbfunc_t ImageConverter;
    ImageConverter.name = "ImageConverter";
    ImageConverter.args = {sYUVImage};
    ImageConverter.func = ImageConverter_func;
    FUNCS.push_back(ImageConverter);

    //PostDetector
    nbfunc_t PostDetector;
    PostDetector.name = "PostDetector";
    PostDetector.args = {sYUVImage};
    PostDetector.func = PostDetector_func;
    FUNCS.push_back(PostDetector);
}


