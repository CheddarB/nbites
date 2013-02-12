#pragma once

#include <QMainWindow>
#include "LogModule.h"

class VSSPMain : public QWidget
{
    Q_OBJECT;

};

class VSSPTool : public QMainWindow
{
    Q_OBJECT;

public:
    VSSPTool() : QMainWindow()
    {
        this->setCentralWidget(new VSSPMain());
        LogModule thing("/home/ecat/nbites/data/logs/failed_walktogoals/returnfrompenalty/MVision.log");
        thing.run();
    };
    ~VSSPTool() {};
};
