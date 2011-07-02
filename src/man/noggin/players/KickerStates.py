#
# This file defines the states necessary for kick testing. Each method
# defines a state.
#

import man.motion as motion
import man.motion.HeadMoves as HeadMoves
import man.motion.SweetMoves as SweetMoves

def gameInitial(player):
    if player.firstFrame():
        player.gainsOn()
        player.brain.fallController.enableFallProtection(False)
    return player.stay()

def gameReady(player):
    if player.firstFrame():
        player.brain.fallController.enableFallProtection(False)
    return player.goLater('standup')

def gameSet(player):
    if player.firstFrame():
        player.brain.fallController.enableFallProtection(False)
    return player.goLater('standup')

def gamePlaying(player):
    if player.firstFrame():
        player.brain.fallController.enableFallProtection(False)
    return player.goLater('standup')

def gamePenalized(player):
    if player.firstFrame():
        player.brain.fallController.enableFallProtection(False)
    return player.goLater('standup')

def standup(player):
    if player.firstFrame():
        player.gainsOn()
        player.walkPose()

    if player.counter == 1:
        return player.goLater('kickStraight')
    return player.stay()

def kickStraight(player):
    printLoc = False
    if player.firstFrame():
        player.brain.tracker.trackBall()
        printLoc = True
    if printLoc:
        print player.brain.ball.relX
        print player.brain.ball.relY

    if player.counter == 30:
        printLoc = False
        player.executeMove(SweetMoves.GOOGZ_LEFT_SIDE_KICK)

    if player.counter == 80:
        return player.goLater('done')
    return player.stay()

def done(player):
    if player.firstFrame():
        #player.walkPose()
        return player.stay()
    return player.stay()
