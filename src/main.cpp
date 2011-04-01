#include <QtGui>
#include "mainwindow.h"
#include "MessageModel.h"
#include "/home/oneamtu/nbites/build/man/straight/memory/protos/Sensors.pb.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
//    MainWindow w;
//    w.show();

    memory::proto::PMotionSensors* ms =
            new memory::proto::PMotionSensors();
    ms->set_timestamp(42);
    ProtoView::MessageModel messageModel(ms);


    QTreeView view;
    view.setModel(&messageModel);
    view.setWindowTitle(QObject::tr("Proto Viewer"));
    view.show();
    return app.exec();
}
