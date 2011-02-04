import man.motion.SweetMoves as SweetMoves
import man.motion.HeadMoves as HeadMoves
import man.motion.StiffnessModes as StiffnessModes
from ..navigator import BrunswickSpeeds as speeds

####Change these for picture taking####
FRAME_SAVE_RATE = 1
NUM_FRAMES_TO_SAVE = 150

def gameReady(player):
    if player.firstFrame():
        player.gainsOn()
        player.executeMove(SweetMoves.INITIAL_POS)
    return player.stay()

def gameSet(player):
    if player.firstFrame:
        player.numFramesSaved = 0
    return player.stay()

def gamePlaying(player):
    if player.firstFrame():
        player.setWalk(0,0, speeds.RIGHT_SPIN_MAX_SPEED)
        player.brain.tracker.startScan(HeadMoves.SPIN_RIGHT_SCAN_BALL)

    #if player.brain.ball.on:
    player.brain.sensors.saveFrame()
    player.numFramesSaved += 1

    if player.numFramesSaved > 300:
        return player.goLater('doneState')

    return player.stay()

def gamePenalized(player):
    if player.firstFrame():
        player.executeMove(SweetMoves.SIT_POS)
        player.brain.tracker.stopHeadMoves()
    return player.stay()

def doneState(player):
    if player.firstFrame():
        player.executeMove(SweetMoves.SIT_POS)
        player.brain.tracker.stopHeadMoves()
        player.brain.sensors.resetSaveFrame()

#     if player.stateTime > 8.0:
#         shutoff = motion.StiffnessCommand(0.0)
#         player.brain.motion.sendStiffness(shutoff)

    return player.stay()

def gameInitial(player):
    return player.stay()

