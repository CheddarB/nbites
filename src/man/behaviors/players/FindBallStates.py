import ChaseBallConstants as constants
import ChaseBallTransitions as transitions
from ..util import *
from objects import Location
from ..navigator import Navigator

@superState('gameControllerResponder')
def findBall(player):
    """
    State to spin to find the ball.
    """
    if player.firstFrame():
        player.brain.tracker.stopHeadMoves()
        player.stopWalking()
        player.inKickingState = False

    if player.brain.nav.isStopped() and player.brain.tracker.isStopped():
        my = player.brain.loc
        ball = Location(player.brain.ball.x, player.brain.ball.y)
        spinDir = my.spinDirToPoint(ball)
        player.setWalk(0, 0, spinDir*constants.FIND_BALL_SPIN_SPEED)

        player.brain.tracker.lookToSpinDirection(spinDir)

    if transitions.shouldChaseBall(player):
        player.stopWalking()
        player.brain.tracker.trackBall()
        return player.goLater('spinToBall')

    if transitions.spunOnce(player):
        return player.goLater('playOffBall')

    return player.stay()
