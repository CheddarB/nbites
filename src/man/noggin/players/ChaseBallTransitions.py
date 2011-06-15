import man.motion.SweetMoves as SweetMoves
import man.motion.HeadMoves as HeadMoves
import ChaseBallConstants as constants
from .. import NogginConstants
from math import fabs

####### CHASING STUFF ##############

def shouldChaseBall(player):
    """
    We see the ball. So go get it.
    """
    ball = player.brain.ball
    return (ball.framesOn > constants.BALL_ON_THRESH)

def shouldChaseFromPositionForKick(player):
    """
    Exit PFK if the ball is too far away.
    """
    ball = player.brain.ball
    return shouldChaseBall(player) and \
        not shouldPositionForKick(player) and \
        ball.dist > constants.BALL_PFK_DIST+10

def shouldPositionForKick(player):
    """
    Should begin aligning on the ball for a kick when close
    """
    ball = player.brain.ball
    return (ball.on and ball.dist < constants.BALL_PFK_DIST and
            (constants.BALL_PFK_LEFT_Y > ball.relY >
             constants.BALL_PFK_RIGHT_Y))

def shouldStopBeforeKick(player):
    """
    Ball is right in front of us but we aren't stopped
    Used before we have a kick decided
    """
    ball = player.brain.ball
    return (shouldPositionForKick(player) and
            ball.dist < constants.SHOULD_STOP_BEFORE_KICK_DIST)

def shouldSpinToBall(player):
    """
    Ball is close and we should spin before we decide our kick
    """
    ball = player.brain.ball
    return ((ball.on and not shouldPositionForKick(player) and
             ball.dist < constants.SHOULD_STOP_BEFORE_KICK_DIST) or
            (ball.relX < constants.SHOULD_SPIN_TO_KICK_X))

def shouldKick(player):
    """
    Ball is in correct position to kick
    """
    return player.brain.nav.isStopped() and player.counter > 1

def shouldDribble(player):
    """
    Ball is in between us and the opp goal, let's dribble for a while
    """
    if constants.USE_DRIBBLE:
        my = player.brain.my
        # helpers is no longer used. Find a different way.
        dribbleAimPoint = helpers.getShotCloseAimPoint(player)
        goalBearing = my.getRelativeBearing(dribbleAimPoint)
        return  (not player.penaltyKicking and
                 0 < player.brain.ball.relX < constants.SHOULD_DRIBBLE_X and
                 0 < fabs(player.brain.ball.relY) < constants.SHOULD_DRIBBLE_Y and
                 fabs(goalBearing) < constants.SHOULD_DRIBBLE_BEARING and
                 not player.brain.my.inOppGoalbox() and
                 player.brain.my.x > (
                     NogginConstants.FIELD_WHITE_WIDTH / 3.0 +
                     NogginConstants.GREEN_PAD_X) )

def shouldStopDribbling(player):
    """
    While dribbling we should stop
    """
    my = player.brain.my
    # helpers is no longer used. Find a different way.
    dribbleAimPoint = helpers.getShotCloseAimPoint(player)
    goalBearing = my.getRelativeBearing(dribbleAimPoint)
    return (player.penaltyKicking or
            player.brain.my.inOppGoalbox() or
            player.brain.ball.relX > constants.STOP_DRIBBLE_X or
            fabs(player.brain.ball.relY) > constants.STOP_DRIBBLE_Y or
            fabs(goalBearing) > constants.STOP_DRIBBLE_BEARING or
            player.brain.my.x < ( NogginConstants.FIELD_WHITE_WIDTH / 3.0 +
                                  NogginConstants.GREEN_PAD_X))

def shouldKickOff(player):
    """
    Determines whether we should do our KickOff play as chaser
    """
    return (not player.hasKickedOffKick)

####### PENALTY KICK STUFF ###########

def shouldStopPenaltyKickDribbling(player):
    """
    While dribbling we should stop
    """
    my = player.brain.my
    # helpers is no longer used. Find a different way.
    dribbleAimPoint = helpers.getShotCloseAimPoint(player)
    goalBearing = my.getRelativeBearing(dribbleAimPoint)
    return (inPenaltyKickStrikezone(player) or
            player.brain.ball.relX > constants.STOP_DRIBBLE_X or
            fabs(player.brain.ball.relY) > constants.STOP_DRIBBLE_Y or
            fabs(goalBearing) > constants.STOP_DRIBBLE_BEARING or
            player.counter > constants.STOP_PENALTY_DRIBBLE_COUNT)

def inPenaltyKickStrikezone(player):
    """
    If we are in a good place to kick
    """
    return (NogginConstants.OPP_GOALBOX_LEFT_X + 75. < player.brain.my.x)


####### FIND BALL STUFF ##############

def shouldScanFindBall(player):
    """
    We lost the ball, scan to find it
    """
    return (player.brain.ball.framesOff > constants.BALL_OFF_THRESH)

def shouldScanFindBallKick(player):
    """
    We lost the ball while in a kicking state, be more generous before looking
    """
    return (player.brain.ball.framesOff > constants.BALL_OFF_KICK_THRESH)

def shouldSpinFindBall(player):
    """
    Should spin if we already tried scanning
    """
    return (player.stateTime >=
            SweetMoves.getMoveTime(HeadMoves.HIGH_SCAN_BALL))

def shouldSpinFindBallAgain(player):
    """
    If we have been walkFindBall-ing too long we should spin.
    """
    return player.stateTime > constants.WALK_FIND_BALL_FRAMES_THRESH

def shouldWalkFindBall(player):
    """
    If we've been spinFindBall-ing too long we should walk
    """
    return player.counter > constants.WALK_FIND_BALL_FRAMES_THRESH
