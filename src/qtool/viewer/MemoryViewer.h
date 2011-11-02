/**
 * @class Viewer
 *
 * A class that maintains a UI that views the data
 * that the data manager has
 *
 * @author Octavian Neamtu <oneamtu89@gmail.com>
 */

#pragma once

#include <QWidget>
#include <QtGui>
#include <vector>

#include "man/memory/Memory.h"
#include "RoboImageViewer.h"
#include "MObjectViewer.h"

namespace qtool {
namespace viewer {

class MemoryViewer : public QMainWindow{
    Q_OBJECT

public:
    MemoryViewer(man::memory::Memory::const_ptr memory);

public slots:
    void nextImage();
    void prevImage();

private:
    std::vector<QDockWidget*> dockWidget;
    man::memory::Memory::const_ptr memory;
    RoboImageViewer* roboImageViewer;
    QPushButton* prevButton;
    QPushButton* nextButton;
    QToolBar* toolbar;
};

}
}
