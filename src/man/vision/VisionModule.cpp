#include "VisionModule.h"
#include "Edge.h"
#include "HighResTimer.h"
#include "NBMath.h"
#include "../control/control.h"
#include "../log/logging.h"

#include <fstream>
#include <iostream>

namespace man {
namespace vision {

VisionModule::VisionModule()
    : Module(),
      topIn(),
      bottomIn(),
      jointsIn()
{
    // NOTE Constructed on heap because some of the objects below do
    //      not have default constructors, all class members must be initialized
    //      after the initializer list is run, which requires calling default
    //      constructors in the case of C-style arrays, limitation theoretically

    //      removed in C++11

    std:: string colorPath, cameraPath;
    #ifdef OFFLINE
        colorPath =  cameraPath = std::string(getenv("NBITES_DIR"));
        colorPath += "/src/man/config/colorParams.txt";
        cameraPath += "/src/man/config/cameraParams.txt";
    #else
        colorPath = "/home/nao/nbites/Config/colorParams.txt";
        cameraPath = "/home/nao/nbites/Config/cameraParams.txt";
    #endif

    // Get SExpr from string
    nblog::SExpr* colors = nblog::SExpr::read(getStringFromTxtFile(colorPath));
    cameraLisp = nblog::SExpr::read(getStringFromTxtFile(cameraPath));


    // Set module pointers for top then bottom images
    for (int i = 0; i < 2; i++) {
        colorParams[i] = getColorsFromLisp(colors, i);
        frontEnd[i] = new ImageFrontEnd();
        edgeDetector[i] = new EdgeDetector();
        edges[i] = new EdgeList(32000);
        houghLines[i] = new HoughLineList(128);
        cameraParams[i] = new CameraParams();
        kinematics[i] = new Kinematics(i == 0);
        homography[i] = new FieldHomography();
        fieldLines[i] = new FieldLineList();
        boxDetector[i] = new GoalboxDetector();

        if (i == 0) {
          hough[i] = new HoughSpace(320, 240);
          cornerDetector[i] = new CornerDetector(320, 240);
        } else {
          hough[i] = new HoughSpace(160, 120);
          cornerDetector[i] = new CornerDetector(160, 120);
        }

        bool fast = true;
        frontEnd[i]->fast(false);
        edgeDetector[i]->fast(fast);
        hough[i]->fast(fast);
    }
}

VisionModule::~VisionModule()
{
    for (int i = 0; i < 2; i++) {
        delete colorParams[i];
        delete frontEnd[i];
        delete edgeDetector[i];
        delete edges[i];
        delete houghLines[i];
        delete hough[i];
        delete kinematics[i];
        delete homography[i];
        delete fieldLines[i];
    }
}

// TODO use horizon on top image
void VisionModule::run_()
{
    // Get messages from inPortals
    topIn.latch();
    bottomIn.latch();
    jointsIn.latch();
    inertsIn.latch();

    // std::cout << "RN: " << robotName_ << "\n"; 
    // Setup
    std::vector<const messages::YUVImage*> images { &topIn.message(),
                                                    &bottomIn.message() };

    // Time vision module
    double topTimes[6];
    double bottomTimes[6];
    double* times[2] = { topTimes, bottomTimes };

    // Loop over top and bottom image and run line detection system
    for (int i = 0; i < images.size(); i++) {
        // Get image
        const messages::YUVImage* image = images[i];

        // Construct YuvLite object for use in vision system
        YuvLite yuvLite(image->width() / 4,
                        image->height() / 2,
                        image->rowPitch(),
                        image->pixelAddress(0, 0));

        HighResTimer timer;

        // Run front end
        frontEnd[i]->run(yuvLite, colorParams[i]);
        ImageLiteU16 yImage(frontEnd[i]->yImage());
        ImageLiteU8 greenImage(frontEnd[i]->greenImage());

        times[i][0] = timer.end();

        // Calculate kinematics and adjust homography
        // std::cout << "Top camera: " << (i == 0) << std::endl;
        kinematics[i]->joints(jointsIn.message());
        homography[i]->wz0(kinematics[i]->wz0());

        
        // homography[i]->roll(homography[i]->roll() + cameraParams[i]->getRoll()*TO_RAD);
        // homography[i]->tilt(kinematics[i]->tilt() + cameraParams[i]->getTilt()*TO_RAD);
        // homography[i]->azimuth(kinematics[i]->azimuth());

        homography[i]->roll(homography[i]->roll());
        homography[i]->tilt(kinematics[i]->tilt());


        // Approximate brightness gradient
        edgeDetector[i]->gradient(yImage);
        
        times[i][1] = timer.end();

        // Run edge detection
        edgeDetector[i]->edgeDetect(greenImage, *(edges[i]));

        times[i][2] = timer.end();

        // Run hough line detection
        hough[i]->run(*(edges[i]), *(houghLines[i]));

        times[i][3] = timer.end();

        // Find field lines
        houghLines[i]->mapToField(*(homography[i]));
        fieldLines[i]->find(*(houghLines[i]));

        times[i][4] = timer.end();

        // Classify field lines
        fieldLines[i]->classify(*(boxDetector[i]), *(cornerDetector[i]));

        times[i][5] = timer.end();
        
        #ifdef USE_LOGGING
        logImage(i);
        #endif
    }

    
    // for (int i = 0; i < 2; i++) {
    //     if (i == 0)
    //         std::cout << "From top camera:" << std::endl;
    //     else
    //         std::cout << std::endl << "From bottom camera:" << std::endl;
    //     std::cout << "Front end: " << times[i][0] << std::endl;
    //     std::cout << "Gradient: " << times[i][1] << std::endl;
    //     std::cout << "Edge detection: " << times[i][2] << std::endl;
    //     std::cout << "Hough: " << times[i][3] << std::endl;
    //     std::cout << "Field lines detection: " << times[i][4] << std::endl;
    //     std::cout << "Field lines classification: " << times[i][5] << std::endl;
    // }
}

void VisionModule::logImage(int i) {
    if (control::flags[control::tripoint]) {
        ++image_index;
        
        messages::YUVImage image;
        std::string image_from;
        if (!i) {
            image = topIn.message();
            image_from = "camera_TOP";
        } else {
            image = bottomIn.message();
            image_from = "camera_BOT";
        }
        
        long im_size = (image.width() * image.height() * 1);
        int im_width = image.width() / 2;
        int im_height= image.height();
        
        messages::JointAngles ja_pb = jointsIn.message();
        messages::InertialState is_pb = inertsIn.message();
        
        std::string ja_buf;
        std::string is_buf;
        std::string im_buf((char *) image.pixelAddress(0, 0), im_size);
        ja_pb.SerializeToString(&ja_buf);
        is_pb.SerializeToString(&is_buf);
        
        im_buf.append(is_buf);
        im_buf.append(ja_buf);
        
        std::vector<nblog::SExpr> contents;
        
        nblog::SExpr imageinfo("YUVImage", image_from, clock(), image_index, im_size);
        imageinfo.append(nblog::SExpr("width", im_width)   );
        imageinfo.append(nblog::SExpr("height", im_height) );
        imageinfo.append(nblog::SExpr("encoding", "[Y8(U8/V8)]"));
        contents.push_back(imageinfo);
        
        nblog::SExpr inerts("InertialState", "tripoint", clock(), image_index, is_buf.length());
        inerts.append(nblog::SExpr("acc_x", is_pb.acc_x()));
        inerts.append(nblog::SExpr("acc_y", is_pb.acc_y()));
        inerts.append(nblog::SExpr("acc_z", is_pb.acc_z()));
        
        inerts.append(nblog::SExpr("gyr_x", is_pb.gyr_x()));
        inerts.append(nblog::SExpr("gyr_y", is_pb.gyr_y()));
        
        inerts.append(nblog::SExpr("angle_x", is_pb.angle_x()));
        inerts.append(nblog::SExpr("angle_y", is_pb.angle_y()));
        contents.push_back(inerts);
        
        nblog::SExpr joints("JointAngles", "tripoint", clock(), image_index, ja_buf.length());
        joints.append(nblog::SExpr("head_yaw", ja_pb.head_yaw()));
        joints.append(nblog::SExpr("head_pitch", ja_pb.head_pitch()));

        joints.append(nblog::SExpr("l_shoulder_pitch", ja_pb.l_shoulder_pitch()));
        joints.append(nblog::SExpr("l_shoulder_roll", ja_pb.l_shoulder_roll()));
        joints.append(nblog::SExpr("l_elbow_yaw", ja_pb.l_elbow_yaw()));
        joints.append(nblog::SExpr("l_elbow_roll", ja_pb.l_elbow_roll()));
        joints.append(nblog::SExpr("l_wrist_yaw", ja_pb.l_wrist_yaw()));
        joints.append(nblog::SExpr("l_hand", ja_pb.l_hand()));

        joints.append(nblog::SExpr("r_shoulder_pitch", ja_pb.r_shoulder_pitch()));
        joints.append(nblog::SExpr("r_shoulder_roll", ja_pb.r_shoulder_roll()));
        joints.append(nblog::SExpr("r_elbow_yaw", ja_pb.r_elbow_yaw()));
        joints.append(nblog::SExpr("r_elbow_roll", ja_pb.r_elbow_roll()));
        joints.append(nblog::SExpr("r_wrist_yaw", ja_pb.r_wrist_yaw()));
        joints.append(nblog::SExpr("r_hand", ja_pb.r_hand()));

        joints.append(nblog::SExpr("l_hip_yaw_pitch", ja_pb.l_hip_yaw_pitch()));
        joints.append(nblog::SExpr("r_hip_yaw_pitch", ja_pb.r_hip_yaw_pitch()));

        joints.append(nblog::SExpr("l_hip_roll", ja_pb.l_hip_roll()));
        joints.append(nblog::SExpr("l_hip_pitch", ja_pb.l_hip_pitch()));
        joints.append(nblog::SExpr("l_knee_pitch", ja_pb.l_knee_pitch()));
        joints.append(nblog::SExpr("l_ankle_pitch", ja_pb.l_ankle_pitch()));
        joints.append(nblog::SExpr("l_ankle_roll", ja_pb.l_ankle_roll()));

        joints.append(nblog::SExpr("r_hip_roll", ja_pb.r_hip_roll() ));
        joints.append(nblog::SExpr("r_hip_pitch", ja_pb.r_hip_pitch() ));
        joints.append(nblog::SExpr("r_knee_pitch", ja_pb.r_knee_pitch() ));
        joints.append(nblog::SExpr("r_ankle_pitch", ja_pb.r_ankle_pitch() ));
        joints.append(nblog::SExpr("r_ankle_roll", ja_pb.r_ankle_roll() ));
        contents.push_back(joints);

        nblog::SExpr camParams("CameraParams", "tripoint", clock(), image_index, 0);
        camParams.append(nblog::SExpr(image_from, cameraParams[i]->getRoll(), cameraParams[i]->getTilt()));
        contents.push_back(camParams);

        nblog::NBLog(NBL_IMAGE_BUFFER, "tripoint",
                   contents, im_buf);
    }
}

const std::string VisionModule::getStringFromTxtFile(std::string path) {
    std::ifstream textFile;
    textFile.open(path);

    // Get size of file
    textFile.seekg (0, textFile.end);
    long size = textFile.tellg();
    textFile.seekg(0);
    
    // Read file into buffer and convert to string
    char* buff = new char[size];
    textFile.read(buff, size);
    std::string sexpText(buff);

    textFile.close();
    return (const std::string)sexpText;
}

/*
 Lisp data in config/colorParams.txt stores 32 parameters. Read lisp and
  load the three compoenets of a Colors struct, white, green, and orange,
  from the 18 values for either the top or bottom image. 
*/
Colors* VisionModule::getColorsFromLisp(nblog::SExpr* colors, int camera) {
    Colors* ret = new man::vision::Colors;
    nblog::SExpr* params;

    if (camera == 0) {
        params = colors->get(1)->find("Top")->get(1);
    } else if (camera == 1) {
        params = colors->get(1)->find("Bottom")->get(1);
    } else {
        params = colors->get(1);
    }

    colors = params->get(0)->get(1);

    
    ret->white. load(std::stof(colors->get(0)->get(1)->serialize()),
                     std::stof(colors->get(1)->get(1)->serialize()),
                     std::stof(colors->get(2)->get(1)->serialize()),
                     std::stof(colors->get(3)->get(1)->serialize()),
                     std::stof(colors->get(4)->get(1)->serialize()),
                     std::stof(colors->get(5)->get(1)->serialize())); 
    
    colors = params->get(1)->get(1);

    ret->green. load(std::stof(colors->get(0)->get(1)->serialize()),
                     std::stof(colors->get(1)->get(1)->serialize()),
                     std::stof(colors->get(2)->get(1)->serialize()),
                     std::stof(colors->get(3)->get(1)->serialize()),
                     std::stof(colors->get(4)->get(1)->serialize()),
                     std::stof(colors->get(5)->get(1)->serialize()));  
    
    colors = params->get(2)->get(1);

    ret->orange.load(std::stof(colors->get(0)->get(1)->serialize()),
                     std::stof(colors->get(1)->get(1)->serialize()),
                     std::stof(colors->get(2)->get(1)->serialize()),
                     std::stof(colors->get(3)->get(1)->serialize()),
                     std::stof(colors->get(4)->get(1)->serialize()),
                     std::stof(colors->get(5)->get(1)->serialize()));

    return ret;
}
void VisionModule::setCameraParams(std::string robotName) {
    setCameraParams(0, robotName);
    setCameraParams(1, robotName);
}

void VisionModule::setCameraParams(int camera, std::string robotName) {
    std::cout << "Here! lisp:\n" << cameraLisp->get(1)->print() << std::endl;

    CameraParams* cp = new CameraParams;
    std::string cam = camera == 0 ? "top" : "bottom";
    if (robotName != "") {
        double roll =  cameraLisp->get(1)->find(robotName)->find(cam)->get(1)->valueAsDouble();
        double pitch = cameraLisp->get(1)->find(robotName)->find(cam)->get(2)->valueAsDouble();
        cameraParams[camera] = new CameraParams(roll, pitch);
    } else {
        std::cout << "Could not set camera params: No Robot Name" << std::endl;
    }
}

}
}
