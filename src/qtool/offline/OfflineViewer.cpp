
#include "OfflineViewer.h"
#include "image/ThresholdedImage.h"

namespace qtool {
namespace offline {

using namespace man::corpus;
using namespace man::memory;
using namespace viewer;
using namespace image;
using namespace data;


OfflineViewer::OfflineViewer(Memory::const_ptr memory, QWidget* parent) :
        QWidget(parent),
        offlineControl(new OfflineManController(memory)),
        manPreloader(offlineControl),
        manMemoryManager(new
                RobotMemoryManager(offlineControl->getManMemory())),
        manMemoryViewer(manMemoryManager) {

    memory->addSubscriber(this, MIMAGE_ID);

    QVBoxLayout *vertLayout = new QVBoxLayout;
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    vertLayout->addLayout(buttonLayout);

    QPushButton* reloadManButton = new QPushButton(tr("&Reload Man"));
    connect(reloadManButton, SIGNAL(clicked()), this, SLOT(reloadMan()));
//    buttonLayout->addWidget(reloadManButton);

    QPushButton* loadTableButton = new QPushButton(tr("&Load Table"));
    connect(loadTableButton, SIGNAL(clicked()), this, SLOT(loadColorTable()));
    buttonLayout->addWidget(loadTableButton);

    vertLayout->addWidget(&manMemoryViewer);
    //add the thresholded image to the memory viewer
    ThresholdedImage::ptr threshImage(new ThresholdedImage(
            offlineControl->getManMemory()->getMImage()->getThresholded()));
    manMemoryManager->connectSlotToMObject(threshImage.get(),
                        SLOT(updateBitmap()), MIMAGE_ID);

    QDockWidget* dockWidget =
            new QDockWidget(tr("Thresholded"), &manMemoryViewer);
    RoboImageViewer* threshView = new RoboImageViewer(threshImage, dockWidget);
    dockWidget->setWidget(threshView);

    dockWidget->setMinimumSize(350, 300);
    manMemoryViewer.addDockWidget(Qt::BottomDockWidgetArea, dockWidget);

    this->setLayout(vertLayout);
}

void OfflineViewer::update() {
    offlineControl->signalNextImageFrame();
}

void OfflineViewer::loadColorTable() {
    QString colorTablePath = QFileDialog::getOpenFileName(this, tr("Open Color Table"),
                                                "../../data/tables",
                                                tr("Table Files (*.mtb)"));
    offlineControl->loadTable(colorTablePath.toStdString());
}

void OfflineViewer::reloadMan() {
    manPreloader.reloadMan();
}

}
}
