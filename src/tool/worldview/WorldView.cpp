#include "WorldView.h"
#include <iostream>
#include <string>

namespace tool {
namespace worldview {

WorldView::WorldView(QWidget* parent)
    : portals::Module(),
      QWidget(parent),
      commThread("comm", COMM_FRAME_LENGTH_uS),
      wviewComm(16,0)

{
    commThread.addModule(*this);
    commThread.addModule(wviewComm);

    fieldPainter = new WorldViewPainter(this);
    QHBoxLayout *mainLayout = new QHBoxLayout(this);

    QHBoxLayout *field = new QHBoxLayout();
    field->addWidget(fieldPainter);

    QVBoxLayout *rightBar = new QVBoxLayout();

    QVBoxLayout *options = new QVBoxLayout();
    options->setAlignment(Qt::AlignTop);
    startButton = new QPushButton(QString("Start World Viewer"));
    options->addWidget(startButton);

    QVBoxLayout *stateLayout = new QVBoxLayout();
    stateLayout->setAlignment(Qt::AlignBottom);

    QGroupBox *stateBox = new QGroupBox(tr("Robot States"));
    QVBoxLayout *boxLayout = new QVBoxLayout();
    for (int i = 0; i < NUM_PLAYERS_PER_TEAM; ++i)
    {
        roleLabels[i] = new QLabel(tr("Inactive"));
        subroleLabels[i] = new QLabel(tr("Inactive"));
    }
    QHBoxLayout *p1Layout = new QHBoxLayout();
    QLabel *p1Label = new QLabel(tr("Player 1: "));
    QHBoxLayout *p2Layout = new QHBoxLayout();
    QLabel *p2Label = new QLabel(tr("Player 2: "));
    QHBoxLayout *p3Layout = new QHBoxLayout();
    QLabel *p3Label = new QLabel(tr("Player 3: "));
    QHBoxLayout *p4Layout = new QHBoxLayout();
    QLabel *p4Label = new QLabel(tr("Player 4: "));
    QHBoxLayout *p5Layout = new QHBoxLayout();
    QLabel *p5Label = new QLabel(tr("Player 5: "));

    p1Layout->addWidget(p1Label);
    p1Layout->addWidget(roleLabels[0]);
    p1Layout->addWidget(subroleLabels[0]);

    p2Layout->addWidget(p2Label);
    p2Layout->addWidget(roleLabels[0]);
    p2Layout->addWidget(subroleLabels[0]);

    p3Layout->addWidget(p3Label);
    p3Layout->addWidget(roleLabels[0]);
    p3Layout->addWidget(subroleLabels[0]);

    p4Layout->addWidget(p4Label);
    p4Layout->addWidget(roleLabels[0]);
    p4Layout->addWidget(subroleLabels[0]);

    p5Layout->addWidget(p5Label);
    p5Layout->addWidget(roleLabels[0]);
    p5Layout->addWidget(subroleLabels[0]);

    boxLayout->addLayout(p1Layout);
    boxLayout->addLayout(p2Layout);
    boxLayout->addLayout(p3Layout);
    boxLayout->addLayout(p4Layout);
    boxLayout->addLayout(p5Layout);
    boxLayout->setSpacing(20);

    stateBox->setFlat(false);

    stateBox->setLayout(boxLayout);
    stateLayout->addWidget(stateBox);

    rightBar->addLayout(options);
    rightBar->addLayout(stateLayout);

    mainLayout->addLayout(field);
    mainLayout->addLayout(rightBar);

    this->setLayout(mainLayout);

    connect(startButton, SIGNAL(clicked()), this, SLOT(startButtonClicked()));

    for (int i = 0; i < NUM_PLAYERS_PER_TEAM; ++i)
    {
        commIn[i].wireTo(wviewComm._worldModels[i]);
    }
}


void WorldView::run_()
{
    for (int i = 0; i < NUM_PLAYERS_PER_TEAM; ++i)
    {
        commIn[i].latch();
        if(!(commIn[i].message().my_x()==0 && commIn[i].message().my_y()==0))
            fieldPainter->updateWithLocationMessage(commIn[i].message());
    }
}

void WorldView::startButtonClicked(){
    commThread.start();
    startButton->setText(QString("Stop World Viewer"));
    disconnect(startButton, SIGNAL(clicked()), this, SLOT(startButtonClicked()));
    connect(startButton, SIGNAL(clicked()), this, SLOT(stopButtonClicked()));
}

void WorldView::stopButtonClicked(){
    commThread.stop();
    startButton->setText(QString("Start World Viewer"));
    disconnect(startButton, SIGNAL(clicked()), this, SLOT(stopButtonClicked()));
    connect(startButton, SIGNAL(clicked()), this, SLOT(startButtonClicked()));
}

}
}
