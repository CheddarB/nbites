#pragma once

#include <QtGui>

#include "RoboGrams.h"
#include "WorldModel.pb.h"

#include "WorldViewPainter.h"

#include "comm/CommModule.h"


namespace tool {
namespace worldview {

class WorldView : public QWidget, public portals::Module{

	Q_OBJECT;

public:
    WorldView(QWidget* parent = 0);

    portals::InPortal<messages::WorldModel> commIn[NUM_PLAYERS_PER_TEAM];

protected:
    virtual void run_();

protected:
	WorldViewPainter* fieldPainter;

    QHBoxLayout* mainLayout;
    QHBoxLayout* field;
    QVBoxLayout* options;

	man::comm::CommModule wview_comm;
};

}
}
