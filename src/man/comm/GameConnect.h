/**
 * This class provides functionality for communicating with the
 * GameController. Uses UDP for transmission and reception and
 * keeps track of the data with the GameData class.
 * @author Wils Dawson 5/29/2012
 */

#pragma once

#include "CommTimer.h"
#include "NetworkMonitor.h"
#include "UDPSocket.h"
#include "GameData.h"
#include "RoboCupGameControlData.h"

namespace man {

namespace comm {

class GameConnect
{
public:
    /**
     * Constructor
     */
    GameConnect(CommTimer* t, NetworkMonitor* m, int team, int player);

    /**
     * Destructor
     */
    ~GameConnect();

    /**
     * Signal to receive any information from the socket
     * and send any appropriate response back to the GC.
     * @param player: The player number to respond as.
     *                0 to respond as all.
     */
    void handle(int player);

    /**
     * @return: The pointer to the GameData.
     */
    GameData* getGameData() {return _data;}

    /**
     * Checks if the specified player is penalized.
     * @param player: The player number to check.
     * @return:       Number of seconds left in penalty.
     *                -1 if not penalized.
     */
    int checkPenalty(int player);

    /*************************************************
     *               Button Interaction              *
     *************************************************/

    /**
     * Called when we have a remote GC and we advanced
     * the game state to penalized.
     */
    void manualPenalize();

    void setMyTeamNumber(int tn, int pn);
    int  myTeamNumber() {return _myTeamNumber;}

private:
    /**
     * Sets up the socket to be used.
     */
    void setUpSocket();

    /**
     * Builds and sends a response to the Game Controller.
     * @param player: The player to respond as.
     * @param msg:    The message to respond with.
     *                Defined in RoboCupGameControlData.h
     */
    void respond(int player, unsigned int msg = GAMECONTROLLER_RETURN_MSG_ALIVE);

    /**
     * Verifies the packet we received from the socket.
     * @param packet: Pointer to the packet.
     */
    bool verify(char* packet);

    CommTimer*      _timer;
    NetworkMonitor* _monitor;
    GameData*       _data;
    UDPSocket*      _socket;
    int             _myTeamNumber;

    bool            _haveRemoteGC;
};

}

}
