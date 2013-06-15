/**
 * Class responsible for creating, viewing, and editing a playbook.
 *
 * @author Wils Dawson
 * @date   May 2013
 */
#pragma once

#include <QtGui>
#include <QCheckBox>
#include <vector>

#include "PlaybookField.h"
#include "PlaybookModel.h"
#include "PlaybookConstants.h"

namespace tool{
namespace playbook{

static const int BOX_SIZE = 100;  // Size in cm for each grid box.

static const int GRID_WIDTH  = (int(FIELD_WHITE_WIDTH) / BOX_SIZE) + 2;
static const int GRID_HEIGHT = (int(FIELD_WHITE_HEIGHT)/ BOX_SIZE) + 2;

class PlaybookCreator : public QWidget
{
    Q_OBJECT;

public:
    PlaybookCreator(QWidget* parent = 0);

protected slots:
    void updatePositions();
    void updatePositionsCheck(bool check);
    void refreshTextDefender();
    void refreshTextOffender();
    void refreshTextMiddie();
    void refreshTextBall();
    void setDefenderXPosition();
    void setDefenderYPosition();
    void setMiddieXPosition();
    void setMiddieYPosition();
    void setOffenderXPosition();
    void setOffenderYPosition();
    void setBallX();
    void setBallY();
    void setTwoFieldPlayers(bool checked);
    void setThreeFieldPlayers(bool checked);
    void setFourFieldPlayers(bool checked);

protected:
    PlaybookField* fieldPainter;
    PlaybookModel* model;

    QHBoxLayout* mainLayout;
    QHBoxLayout* field;
    QVBoxLayout* settings;

    QPushButton* undoBtn;
    QPushButton* loadBtn;
    QPushButton* saveBtn;
    QCheckBox* lockDefender;
    QCheckBox* lockMiddie;
    QCheckBox* lockOffender;
    QCheckBox* goalie;
    QLineEdit* editDefenderX;
    QLineEdit* editDefenderY;
    QLineEdit* editMiddieX;
    QLineEdit* editMiddieY;
    QLineEdit* editOffenderX;
    QLineEdit* editOffenderY;
    QRadioButton* twoFieldPlayers;
    QRadioButton* threeFieldPlayers;
    QRadioButton* fourFieldPlayers;
    QLineEdit* editBallX;
    QLineEdit* editBallY;

private:
    bool havePlaybook;
    void updateRobotPositions();
    void updateLockedPositions();
    void refreshTextAll();
};

}
}
