
#include "VisionViewer.h"
#include "Camera.h"
#include <vector>

namespace qtool {
namespace viewer {

using namespace data;
using boost::shared_ptr;
using namespace man::memory;
using namespace man::corpus;
using namespace qtool::image;

VisionViewer::VisionViewer(RobotMemoryManager::const_ptr memoryManager) :
                 memoryManager(memoryManager),
                 speech(new Speech()),
                 sensors(new Sensors(speech)),
                 bottomRawImage(new proto::PImage()),
                 topRawImage(new proto::PImage())
{

    memoryManager->getMemory()->getMVisionSensors()->copyTo(sensors);
    pose = shared_ptr<NaoPose> (new NaoPose(sensors));
    vision = shared_ptr<Vision> (new Vision(pose));
    offlineMVision = shared_ptr<MVision> (new MVision(vision));

    imageTranscribe = OfflineImageTranscriber::ptr
        (new OfflineImageTranscriber(sensors,
                                     memoryManager->getMemory()->
                                     getMImage(Camera::TOP),
                                     memoryManager->getMemory()->
                                     getMImage(Camera::BOTTOM)));

    bottomRawImage->set_width(AVERAGED_IMAGE_WIDTH);
    bottomRawImage->set_height(AVERAGED_IMAGE_HEIGHT);
    topRawImage->set_width(AVERAGED_IMAGE_WIDTH);
    topRawImage->set_height(AVERAGED_IMAGE_HEIGHT);

    QToolBar* toolBar = new QToolBar(this);
    QPushButton* loadTableButton = new QPushButton(tr("&Load Table"));
    connect(loadTableButton, SIGNAL(clicked()), this, SLOT(loadColorTable()));
    toolBar->addWidget(loadTableButton);
    this->addToolBar(toolBar);

    QCheckBox* horizonDebug = new QCheckBox(tr("Horizon"));
    connect(horizonDebug, SIGNAL(clicked()), this, SLOT(setHorizonDebug()));
    toolBar->addWidget(horizonDebug);
    QCheckBox* shootingDebug = new QCheckBox(tr("Shooting"));
    connect(shootingDebug, SIGNAL(clicked()), this, SLOT(setShootingDebug()));
    toolBar->addWidget(shootingDebug);
    QCheckBox* openFieldDebug = new QCheckBox(tr("Open Field"));
    connect(openFieldDebug, SIGNAL(clicked()), this, SLOT(setOpenFieldDebug()));
    toolBar->addWidget(openFieldDebug);
    QCheckBox* EdgeDetectionDebug = new QCheckBox(tr("Edge Detection"));
    connect(EdgeDetectionDebug, SIGNAL(clicked()), this, SLOT(setEdgeDetectDebug()));
    toolBar->addWidget(EdgeDetectionDebug);
    QCheckBox* houghDebug = new QCheckBox(tr("Hough"));
    connect(houghDebug, SIGNAL(clicked()), this, SLOT(setHoughDebug()));
    toolBar->addWidget(houghDebug);
    QCheckBox* robotsDebug = new QCheckBox(tr("Robots"));
    connect(robotsDebug, SIGNAL(clicked()), this, SLOT(setRobotsDebug()));
    toolBar->addWidget(robotsDebug);
    QCheckBox* ballDebug = new QCheckBox(tr("Ball"));
    connect(ballDebug, SIGNAL(clicked()), this, SLOT(setBallDebug()));
    toolBar->addWidget(ballDebug);
    QCheckBox* ballDistanceDebug = new QCheckBox(tr("Ball Distance"));
    connect(ballDistanceDebug, SIGNAL(clicked()), this, SLOT(setBallDistDebug()));
    toolBar->addWidget(ballDistanceDebug);
    QCheckBox* crossDebug = new QCheckBox(tr("Cross"));
    connect(crossDebug, SIGNAL(clicked()), this, SLOT(setCrossDebug()));
    toolBar->addWidget(crossDebug);
    QCheckBox* identCornersDebug = new QCheckBox(tr("Identify Corners"));
    connect(identCornersDebug, SIGNAL(clicked()), this, SLOT(setIdentCornersDebug()));
    toolBar->addWidget(identCornersDebug);
    QCheckBox* dangerousBallDebug = new QCheckBox(tr("Dangerous Ball"));
    connect(dangerousBallDebug, SIGNAL(clicked()), this, SLOT(setDangerousBallDebug()));
    toolBar->addWidget(dangerousBallDebug);
    QCheckBox* fieldEdgeDebug = new QCheckBox(tr("Field Edge"));
    connect(fieldEdgeDebug, SIGNAL(clicked()), this, SLOT(setFieldEdgeDebug()));
    toolBar->addWidget(fieldEdgeDebug);

    QCheckBox* postPrintDebug = new QCheckBox(tr("Post Print"));
    connect(postPrintDebug, SIGNAL(clicked()), this, SLOT(setPostPrintDebug()));
    toolBar->addWidget(postPrintDebug);
    QCheckBox* postDebug = new QCheckBox(tr("Post"));
    connect(postDebug, SIGNAL(clicked()), this, SLOT(setPostDebug()));
    toolBar->addWidget(postDebug);
    QCheckBox* postLogicDebug = new QCheckBox(tr("Post Logic"));
    connect(postDebug, SIGNAL(clicked()), this, SLOT(setPostLogicDebug()));
    toolBar->addWidget(postLogicDebug);
    QCheckBox* postCorrectDebug = new QCheckBox(tr("Post Correct"));
    connect(postCorrectDebug, SIGNAL(clicked()), this, SLOT(setPostCorrectDebug()));
    toolBar->addWidget(postCorrectDebug);
    QCheckBox* postSanityDebug = new QCheckBox(tr("Post Sanity"));
    connect(postSanityDebug, SIGNAL(clicked()), this, SLOT(setPostSanityDebug()));
    toolBar->addWidget(postSanityDebug);
    /*
    QCheckBox* vertEdgeDetectDebug = new QCheckBox(tr("Vertical Edge"));
    connect(vertEdgeDetectDebug, SIGNAL(clicked()), this, SLOT(setVertEdgeDetectDebug()));
    toolBar->addWidget(vertEdgeDetectDebug);
    QCheckBox* horEdgeDetectDebug = new QCheckBox(tr("Horizontal Edge"));
    connect(horEdgeDetectDebug, SIGNAL(clicked()), this, SLOT(setHorEdgeDetectDebug()));
    toolBar->addWidget(horEdgeDetectDebug);
    QCheckBox* secondVertEdgeDebug = new QCheckBox(tr("Second Vertical Edge"));
    connect(secondVertEdgeDebug, SIGNAL(clicked()), this, SLOT(setSecondVertDebug()));
    toolBar->addWidget(secondVertEdgeDebug);
    QCheckBox* createLinesDebug = new QCheckBox(tr("Create Lines"));
    connect(createLinesDebug, SIGNAL(clicked()), this, SLOT(setCreateLinesDebug()));
    toolBar->addWidget(createLinesDebug);
    QCheckBox* fitUnusedPointsDebug = new QCheckBox(tr("Fit Unused Points"));
    connect(fitUnusedPointsDebug, SIGNAL(clicked()), this, SLOT(setFitUnusedPointsDebug()));
    toolBar->addWidget(fitUnusedPointsDebug);
    QCheckBox* joinLinesDebug = new QCheckBox(tr("Join Lines"));
    connect(joinLinesDebug, SIGNAL(clicked()), this, SLOT(setJoinLinesDebug()));
    toolBar->addWidget(joinLinesDebug);
    QCheckBox* extendLinesDebug = new QCheckBox(tr("Extend Lines"));
    connect(extendLinesDebug, SIGNAL(clicked()), this, SLOT(setExtendLinesDebug()));
    toolBar->addWidget(extendLinesDebug);
    QCheckBox* intersectLinesDebug = new QCheckBox(tr("Intersect Lines"));
    connect(intersectLinesDebug, SIGNAL(clicked()), this, SLOT(setIntersectLinesDebug()));
    toolBar->addWidget(intersectLinesDebug);
    QCheckBox* CcScanDebug = new QCheckBox(tr("CcScan"));
    connect(CcScanDebug, SIGNAL(clicked()), this, SLOT(setCcScanDebug()));
    toolBar->addWidget(CcScanDebug);
    QCheckBox* riskyCornersDebug = new QCheckBox(tr("Risky Corners"));
    connect(riskyCornersDebug, SIGNAL(clicked()), this, SLOT(setRiskyCornersDebug()));
    toolBar->addWidget(riskyCornersDebug);
    QCheckBox* cornObjDistDebug = new QCheckBox(tr("Corner and Object Distances"));
    connect(cornObjDistDebug, SIGNAL(clicked()), this, SLOT(setCornObjDistDebug()));
    toolBar->addWidget(cornObjDistDebug);
*/

    horizonD = shootD = openFieldD = edgeDetectD = houghD = robotsD = false;
    ballD = ballDistD = crossD = identCornersD = dangerousBallD = fieldEdgeD = false;
    postPrintD = postD = postLogicD = postCorrectD = postSanityD = false;
    vertEdgeD = horEdgeD = secVertD = createLD = fitPointD = joinLD = false;
    intersectLD = CcScanD = riskyCornerD = cornObjDistD = false;

    bottomVisionImage = new ThresholdedImage(bottomRawImage, this);
    topVisionImage = new ThresholdedImage(topRawImage, this);

    VisualInfoImage* shapesBottom = new VisualInfoImage(offlineMVision, Camera::BOTTOM);
    VisualInfoImage* shapesTop = new VisualInfoImage(offlineMVision, Camera::TOP);    

    FastYUVToBMPImage* rawBottomBMP = new FastYUVToBMPImage(memoryManager->
                                                      getMemory()->
                                                  getMImage(Camera::BOTTOM),
                                                      this);
    FastYUVToBMPImage* rawTopBMP = new FastYUVToBMPImage(memoryManager->
                                                         getMemory()->
                                                     getMImage(Camera::TOP),
                                                         this);

    OverlayedImage* comboBottom = new OverlayedImage(rawBottomBMP, shapesBottom, this);
    OverlayedImage* comboTop = new OverlayedImage(rawTopBMP, shapesTop, this);

    BMPImageViewer *bottomImageViewer = new BMPImageViewer(comboBottom, this);
    BMPImageViewer *topImageViewer = new BMPImageViewer(comboTop, this);

    CollapsibleImageViewer* bottomCIV = new
        CollapsibleImageViewer(bottomImageViewer,
                               "Bottom",
                               this);
    CollapsibleImageViewer* topCIV = new CollapsibleImageViewer(topImageViewer,
                                                                "Top",
                                                                this);

    QWidget* rawImages = new QWidget(this);

    QVBoxLayout* layout = new QVBoxLayout(rawImages);

    layout->addWidget(topCIV);
    layout->addWidget(bottomCIV);

    rawImages->setLayout(layout);

    BMPImageViewer *bottomVisViewer = new BMPImageViewerListener(bottomVisionImage,
                                                       this);
    connect(bottomVisViewer, SIGNAL(mouseClicked(int, int, int, bool)),
            this, SLOT(pixelClicked(int, int, int, bool)));
    BMPImageViewer *topVisViewer = new BMPImageViewer(topVisionImage,
                                                      this);

    CollapsibleImageViewer* bottomVisCIV = new
        CollapsibleImageViewer(bottomVisViewer,
                               "Bottom",
                               this);
    CollapsibleImageViewer* topVisCIV = new CollapsibleImageViewer(topVisViewer,
                                                                   "Top",
                                                                   this);

    QWidget* visionImages = new QWidget(this);

    QVBoxLayout* visLayout = new QVBoxLayout(visionImages);

    visLayout->addWidget(topVisCIV);
    visLayout->addWidget(bottomVisCIV);

    visionImages->setLayout(visLayout);

    QTabWidget* imageTabs = new QTabWidget();
    imageTabs->addTab(rawImages, tr("Raw Images"));
    imageTabs->addTab(visionImages, tr("Vision Images"));

    memoryManager->connectSlotToMObject(this, SLOT(update()),
                                        MBOTTOMIMAGE_ID);
    memoryManager->connectSlotToMObject(this, SLOT(update()),
                                        MTOPIMAGE_ID);

    this->setCentralWidget(imageTabs);
    memoryManager->connectSlotToMObject(bottomVisViewer,
					SLOT(updateView()), MBOTTOMIMAGE_ID);
    memoryManager->connectSlotToMObject(topVisViewer,
					SLOT(updateView()), MTOPIMAGE_ID);

    memoryManager->connectSlotToMObject(bottomImageViewer,
					SLOT(updateView()), MBOTTOMIMAGE_ID);
    memoryManager->connectSlotToMObject(topImageViewer,
					SLOT(updateView()), MTOPIMAGE_ID);

    //corner ownership
    this->setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
    this->setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

    std::vector<QTreeView> messageViewers; 
    for (MObject_ID id = FIRST_OBJECT_ID;
            id != LAST_OBJECT_ID; id++) {
        if (id == MVISION_ID) {
            QDockWidget* dockWidget = 
                   new QDockWidget("Offline Vision", this);
            MObjectViewer* view = new MObjectViewer(offlineMVision->getProtoMessage());
	    dockWidget->setWidget(view);
            this->addDockWidget(Qt::RightDockWidgetArea, dockWidget);
            memoryManager->connectSlotToMObject(view, SLOT(updateView()), id);
	}
        if (id != MTOPIMAGE_ID && id != MBOTTOMIMAGE_ID &&
            id != MVISION_ID) {
            QDockWidget* dockWidget =
                    new QDockWidget(QString(MObject_names[id].c_str()), this);
            MObjectViewer* view = new MObjectViewer(
                    memoryManager->getMemory()->
                    getMObject(id)->getProtoMessage());
            dockWidget->setWidget(view);
            this->addDockWidget(Qt::RightDockWidgetArea, dockWidget);
            memoryManager->connectSlotToMObject(view, SLOT(updateView()), id);
        }
    }

    // Make sure one of the images is toggled off for small screens
    bottomCIV->toggle();
}

void VisionViewer::update(){

    //no useless computation
    if (!this->isVisible())
        return;

    imageTranscribe->acquireNewImage();

    // update the vision body angles
    MImage::const_ptr mImage = memoryManager->getMemory()->getMImage(Camera::BOTTOM);
    std::vector<float> body_angles(mImage->get()->vision_body_angles().begin(),
                                   mImage->get()->vision_body_angles().end());

    if (body_angles.empty())
      sensors->updateVisionAngles();
    else
      sensors->setVisionBodyAngles(body_angles);

    vision->notifyImage(sensors->getImage(Camera::TOP), sensors->getImage(Camera::BOTTOM));
    offlineMVision->updateData();
    bottomRawImage->mutable_image()->assign(reinterpret_cast<const char *>
                                            (vision->thresh->thresholdedBottom),
                                            AVERAGED_IMAGE_SIZE);
    topRawImage->mutable_image()->assign(reinterpret_cast<const char *>
                                         (vision->thresh->thresholded),
                                         AVERAGED_IMAGE_SIZE);
}

void VisionViewer::pixelClicked(int x, int y, int brushSize, bool leftClick) {

    estimate pixEst = vision->pose->pixEstimate(x, y, 0.0f);
    std::cout << "x: " << x << "   y: " << y << std::endl;
    std::cout << pixEst << std::endl;
}

void VisionViewer::loadColorTable(){
  QString colorTablePath = QFileDialog::getOpenFileName(this, tr("Open Color Table"),
							"../../data/tables",
							tr("Table Files (*.mtb)"));
  imageTranscribe->initTable(colorTablePath.toStdString());

}

void VisionViewer::setHorizonDebug(){
  if (horizonD == false) horizonD = true;
  else horizonD = false;
  vision->thresh->setHorizonDebug(horizonD);
}
void VisionViewer::setShootingDebug(){
  if (shootD == false) shootD = true;
  else shootD = false;
  vision->thresh->setDebugShooting(shootD);
}
void VisionViewer::setOpenFieldDebug(){
  if (openFieldD == false) openFieldD = true;
  else openFieldD = false;
  vision->thresh->setDebugOpenField(openFieldD);
}
void VisionViewer::setEdgeDetectDebug(){
  if (edgeDetectD == false) edgeDetectD = true;
  else edgeDetectD = false;
  vision->thresh->setDebugEdgeDetection(edgeDetectD);
}
void VisionViewer::setHoughDebug(){
  if (houghD == false) houghD = true;
  else houghD = false;
  vision->thresh->setDebugHoughTransform(houghD);
}
void VisionViewer::setRobotsDebug(){
  if (robotsD == false) robotsD = true;
  else robotsD = false;
  vision->thresh->setDebugRobots(robotsD);
}
void VisionViewer::setBallDebug(){
  if (ballD == false) ballD = true;
  else ballD = false;
  vision->thresh->orange->setDebugBall(ballD);
}
void VisionViewer::setBallDistDebug(){
  if (ballDistD == false) ballDistD = true;
  else ballDistD = false;
  vision->thresh->orange->setDebugBallDistance(ballDistD);
}
void VisionViewer::setCrossDebug(){
  if (crossD == false) crossD = true;
  else crossD = false;
  vision->thresh->cross->setCrossDebug(crossD);
}
void VisionViewer::setIdentCornersDebug(){
  if (identCornersD == false) identCornersD = true;
  else identCornersD = false;
  vision->thresh->context->setDebugIdentifyCorners(identCornersD);
}
void VisionViewer::setFieldEdgeDebug(){
  if (fieldEdgeD == false) fieldEdgeD = true;
  else fieldEdgeD = false;
  vision->thresh->field->setDebugFieldEdge(robotsD);
}

void VisionViewer::setPostPrintDebug(){
  if (postPrintD == false) postPrintD = true;
  else postPrintD = false;
  vision->thresh->yellow->setPrintObjs(postPrintD);
}
void VisionViewer::setPostDebug(){
  if (postD == false) postD = true;
  else postD = false;
  vision->thresh->yellow->setPostDebug(postD);
}
void VisionViewer::setPostLogicDebug(){
  if (postLogicD == false) postLogicD = true;
  else postLogicD = false;
  vision->thresh->yellow->setPostLogic(postLogicD);
}
void VisionViewer::setPostCorrectDebug(){
  if (postCorrectD == false) postCorrectD = true;
  else postCorrectD = false;
  vision->thresh->yellow->setCorrect(postCorrectD);
}
void VisionViewer::setPostSanityDebug(){
  if (postSanityD == false) postSanityD = true;
  else postSanityD = false;
  vision->thresh->yellow->setSanity(postSanityD);
}


}
}
