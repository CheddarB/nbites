/**
 * Class responsible for holding information about a playbook.
 *
 * @author Wils Dawson
 * @date   May 2013
 */

#pragma once

#include <QObject>
#include <QStack>
#include "FieldConstants.h"

namespace tool {
namespace playbook {

struct PlaybookPosition
{
    short x;
    short y;
    short h;
    short role;
    PlaybookPosition(int x_, int y_, int h_, char r) :
        x(x_), y(y_), h(h_), role(r)
    {}
};

static const short defaultRoleList [] = {0,2,1,0,2,0};

class PlaybookModel : public QObject
{
    Q_OBJECT;

public:
    PlaybookModel(int b_s, int g_w, int g_h, QObject* parent = 0);

    // Playbook is a big table of positions organized by:
    // 1) Goalie presence       2 possibilities
    // 2) Active Field Players  3+2+1 possibilities
    // 3) Ball X-Position       FIELD_WIDTH / BOX_SIZE + 2
    // 4) Ball Y-Position       FIELD_HEIGHT/ BOX_SIZE + 2
    // Note: #2 (above) is a priority ordered list of roles for a
    // varying number of active field players. For example, the first
    // three entries would be for when we have 4 active field players;
    // the chaser is not in the list, the first position is the
    // defender, next is the offender, and last is the middie; the
    // next two entries are for when there are 3 active field players
    // (defender and offender); and the last position is just the two
    // active field player case. So the 2nd dimension of the array is:
    //
    // 4-Defender | 4-Offender | 4-Middie | 3-Defender | 3-Offender | 2-Defender
    //
    // Where the number is the active field players present. The 3rd
    // and 4th dimensions of the playbook, then, handle where that role should
    // go given where the ball is.

    PlaybookPosition***** playbook; // [2][3+2+1][GRID_WIDTH][GRID_HEIGHT];

    PlaybookPosition** getRobotPositions();

protected slots:
    void toggleGoalie(bool on);
    void toggleDefender(bool on);
    void toggleOffender(bool on);
    void toggleMiddie(bool on);

protected:
    QStack<int> changedIndex;  // Holds the indecies in the playbook
                               // that were changed.
    QStack<PlaybookPosition> changedPositions; // The old positions.

    int goalieOn;
    bool defenderLocked;
    bool offenderLocked;
    bool middieLocked;

    int BOX_SIZE;
    int GRID_WIDTH;
    int GRID_HEIGHT;
};

}
}
