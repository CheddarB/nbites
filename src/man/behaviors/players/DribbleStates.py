"""
Here is the FSA for dribbling a ball.
"""
import DribbleTransitions as transitions
import DribbleConstants as constants
from ..navigator import Navigator
from ..kickDecider import kicks
from objects import RelRobotLocation, Location

### BASIC IDEA 
# We dribble by setting ourselves up for a dribble kick. The sweet spot 
# is in front of the ball, so setting ourselves up for a kick actually
# results in us running through the ball. (There is no actual dribble sweet
# move.) If vision sees a crowded area in front of us, we rotate around the 
# ball and dribble again when we see a clear path. We only dribble if these 
# conditions are true: 1. We are between the two field crosses. 2. We are 
# facing our opponents' goal. 3. We can see the ball close to us.

### TODO
# test time-left based decision making
# dribble through if close enough to goal
# dribbleGoneBad should take in to account heading
# rotate towards goal when dribbling
# get rid of 'dribble' state?
# choose direction better, based on loc and heatmap
# frame counter rotatation?

### DONE
# cross to cross dribbling
# time-left based decision making
        # kick instead 
        # dribble through

def dribble(player):
    """
    Super State to determine what to do from various situations.
    """
    if (transitions.facingGoal(player) and transitions.betweenCrosses(player) 
        and transitions.crowded(player) and transitions.timeLeft(player)
        and not transitions.ballGotFarAway(player) and not
        transitions.ballLost(player)):
        return player.goNow('decideDribble')
    else:
        return player.goLater('chase')

def decideDribble(player):
    """
    Decide to dribble straight ahead or rotate to avoid other robots.
    """
    if player.firstFrame():
        player.brain.tracker.trackBall()
        if transitions.ballToOurLeft(player):
            player.kick = kicks.LEFT_DRIBBLE
        else:
            player.kick = kicks.RIGHT_DRIBBLE

    if transitions.centerLaneOpen(player) and transitions.crowded(player):
        return player.goNow('executeDribble')
    elif transitions.crowded(player):
        return player.goNow('rotateToOpenSpace')
    else:
        return player.goLater('dribble')

def executeDribble(player):
    """
    Move through the ball, so as to execute a dribble.
    """
    ball = player.brain.ball
    kick_pos = player.kick.getPosition()
    player.kickPose = RelRobotLocation(ball.rel_x - kick_pos[0],
                                       ball.rel_y - kick_pos[1],
                                       0)

    if player.firstFrame():
        # player.ballBeforeApproach = player.brain.ball
        # player.brain.tracker.lookStraightThenTrack()
        executeDribble.counter = 0
        player.brain.nav.goTo(player.kickPose,
                              Navigator.PRECISELY,
                              Navigator.MEDIUM_SPEED,
                              False,
                              False)
    else:
        player.brain.nav.updateDest(player.kickPose)

    if transitions.ballLost(player):
        return player.goNow('lookForBall')
    elif (transitions.ballGotFarAway(player) or not transitions.timeLeft(player):
        return player.goLater('dribble')
    # elif not transitions.crowded(player):
    #     executeDribble.counter += 1
    elif not transitions.centerLaneOpen(player):
        return player.goNow('rotateToOpenSpace')
    elif transitions.dribbleGoneBad(player):
        return player.goNow('positionForDribble')

    return player.stay()

def rotateToOpenSpace(player):
    """
    Rotate around ball, so as to find an open lane to dribble thru
    """
    if player.firstFrame():
        # rotateToOpenSpace.counter = 0
        if transitions.rotateLeft(player):
            player.setWalk(0, -.5, .15)
        else:
            player.setWalk(0, .5, -.15)

    if (transitions.ballLost(player) or transitions.ballGotFarAway(player) or
        not transitions.timeLeft(player) or transitions.centerLaneOpen(player)):
        player.stand()
        return player.goLater('dribble')
    # elif transitions.centerLaneOpen(player):
    #     rotateToOpenSpace.counter += 1

    return player.stay()

def lookForBall(player):
    """
    Backup and look for ball. If fails, leave the FSA.
    """
    if player.firstFrame():
        player.brain.tracker.repeatWidePan()
        backupLoc = RelRobotLocation(constants.BACKUP_WHEN_LOST,0,0)
        player.brain.nav.goTo(backupLoc,
                              Navigator.GENERAL_AREA,
                              Navigator.MEDIUM_SPEED,
                              False,
                              False)
        
    if transitions.seesBall(player):
        player.brain.tracker.trackBall()
        return player.goLater('positionForDribble')
    elif transitions.navDone(player):
        return player.goLater('chase')

    return player.stay()

def positionForDribble(player):
    """
    We should position ourselves behind the ball for easy dribbling.
    """
    ball = player.brain.ball
    # kick_pos = player.kick.getPosition()
    backed_off = constants.BACKED_OFF_WHEN_POSITIONING
    player.kickPose = RelRobotLocation(ball.rel_x + backed_off, # 20 cm behind ball
                                       ball.rel_y,
                                       0)

    if player.firstFrame():
        player.brain.nav.goTo(player.kickPose,
                              Navigator.GENERAL_AREA,
                              Navigator.MEDIUM_SPEED,
                              False,
                              False)
    else:
        player.brain.nav.updateDest(player.kickPose)

    if transitions.navDone(player):
        return player.goLater('dribble')

    return player.stay()
