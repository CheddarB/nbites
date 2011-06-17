
#include "QTool.h"
#include <iostream>

namespace qtool {

using data::DataManager;
using colorcreator::ColorCreator;


QTool::QTool() : QMainWindow(),
        toolTabs(new QTabWidget()),
        dataManager(new DataManager()),
        colorCreator(new ColorCreator(dataManager)){

    this->setWindowTitle(tr("HackTool"));

    this->setCentralWidget(toolTabs);

    toolTabs->addTab(colorCreator, tr("Color Creator"));

    dataManager->addSubscriber(colorCreator, data::NEW_IMAGE);
}

QTool::~QTool() {
    delete colorCreator;
    delete toolTabs;

    delete dataManager;
}

}
