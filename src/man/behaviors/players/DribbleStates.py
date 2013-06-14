"""
Here is the FSA for dribbling a ball.
"""
import DribbleTransitions as transitions
import DribbleConstants as constants
from ..navigator import Navigator
# from ..kickDecider import HackKickInformation as hackKick
# from ..kickDecider import kicks
# from objects import RelRobotLocation, Location
# from math import fabs
# import noggin_constants as nogginConstants

def dribble(player):
    """
    Super State to determine what to do from various situations.
    """
    if not transitions.seesBall(player):
        return player.goNow('backupForBall')
    elif (transitions.facingGoal(player) and transitions.middleThird(player) 
        and transitions.crowded(player)):
        return player.goNow('decideDribble')
    else:
        return player.goLater('chase')

def decideDribble(player):
    """
    Decide to dribble straight ahead or rotate to avoid other robots.
    """
    if player.firstFrame():
        player.brain.tracker.trackBall()

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
    if player.firstFrame():
        player.brain.nav.goTo(player.brain.ball,
                              Navigator.CLOSE_ENOUGH,
                              Navigator.MEDIUM_SPEED,
                              False,
                              False)
    else:
        player.brain.nav.updateDest(player.brain.ball)

    if (transitions.ballLost(player) or transitions.ballGotFarAway(player)
        not transitions.facingGoal(player) or 
        not transitions.middleThird(player) 
        or not transitions.crowded(player)):
        return player.goLater('dribble')
    elif not transitions.centerLaneOpen(player):
        return player.goLater('rotateToOpenSpace')

    return player.stay()

def rotateToOpenSpace(player):
    """
    Rotate around ball, so as to find an open lane to dribble thru
    """
    if player.firstFrame():
        if player.brain.interface.left_dist > player.brain.interface.right_dist:
            player.setWalk(0, -.5, .15)
        else:
            player.setWalk(0, .5, -.15)

    if (transitions.ballLost(player) or transitions.ballGotFarAway(player)
        not transitions.facingGoal(player) or 
        not transitions.middleThird(player) 
        or not transitions.crowded(player)):
        player.stand()
        return player.goLater('dribble')
    elif transitions.centerLaneOpen(player):
        player.stand()
        return player.goNow('executeDribble')

    return player.stay()

def backupForBall(player):
    """
    Backup and look for ball. If fails, leave the FSA.
    """
    return player.goLater('chase')

# def approachBall(player):
#     if player.firstFrame():
#         player.brain.tracker.trackBall()
#         if player.shouldKickOff:
#             player.brain.nav.chaseBall(Navigator.QUICK_SPEED, fast = True)
#         else:
#             player.brain.nav.chaseBall(fast = True)
# 
#     if (transitions.shouldFindBall(player)):
#         print "DEBUG_SUIT: In 'approachBall', shouldFindBall is True. Switching to 'chase'."
#         return player.goLater('chase')
# 
#     if (transitions.shouldPrepareForKick(player) or
#         player.brain.nav.isAtPosition()):
#         player.inKickingState = True
# 
#         if player.shouldKickOff:
#             if player.brain.ball.rel_y > 0:
#                 player.kick = kicks.LEFT_STRAIGHT_KICK
#             else:
#                 player.kick = kicks.RIGHT_STRAIGHT_KICK
#             player.shouldKickOff = False
#             return player.goNow('positionForKick')
#         else:
#             print "DEBUG_SUITE: In 'approachBall', either shouldPrepareForKick or nav.isAtPosition is True. Not a kickoff: switching to 'prepareForKick'."
#             return player.goNow('prepareForKick')
# 
#     else:
#         return player.stay()
# 
# def prepareForKick(player):
#     if player.firstFrame():
#         prepareForKick.hackKick = hackKick.KickInformation(player.brain)
#         player.orbitDistance = player.brain.ball.distance
#         player.brain.tracker.performKickPan(prepareForKick.hackKick.shouldKickPanRight())
#         player.brain.nav.stand()
#         return player.stay()
# 
#     prepareForKick.hackKick.collectData()
# 
#     if player.brain.ball.distance > 40:
#         print "DEBUG_SUITE: In 'prepareForKick', ball.distance is >40. Switching to 'chase'."
#         # Ball has moved away. Go get it!
#         player.inKickingState = False
#         return player.goLater('chase')
# 
#     # If loc is good, stop pan ASAP and do the kick
#     # Loc is currently never accurate enough @summer 2012
#     #  Might have to do it anyway if comm is always down.
# 
#     # If hackKickInfo has enough information already, prematurely end pan and kick.
#     if player.brain.tracker.isStopped() or \
#             prepareForKick.hackKick.hasEnoughInformation():
#         print "DEBUG_SUITE: In 'prepareForKick', either tracker.isStopped or hackKick.hasEnoughInformation. Switching to 'orbitBall'."
#         prepareForKick.hackKick.calculateDataAverages()
#         if hackKick.DEBUG_KICK_DECISION:
#             print str(prepareForKick.hackKick)
#         player.kick = prepareForKick.hackKick.shoot()
#         if hackKick.DEBUG_KICK_DECISION:
#             print str(player.kick)
#         return player.goNow('orbitBall')
# 
#     return player.stay()
# 
# def orbitBall(player):
#     """
#     State to orbit the ball
#     """
#     if player.firstFrame():
#         orbitBall.counter = 0
#         if hackKick.DEBUG_KICK_DECISION:
#             print "Orbiting at angle: ",player.kick.h
# 
#         if player.kick.h == 0:
#             print "DEBUG_SUITE: In 'orbitBall', orbit is zero. Switching to 'positionForKick'."
#             return player.goNow('positionForKick')
# 
#         print "DEBUG_SUITE: In 'orbitBall', orbiting at non-zero angle."
#         # Reset from pre-kick pan to straight, then track the ball.
#         player.brain.tracker.lookStraightThenTrack()
# 
#         if player.kick.h > 0:
#             #set y vel at 50% speed
#             print "Turn to right"
#             player.brain.nav.walk(0, .5, -.15)
#         
#         if player.kick.h < 0:
#             #set y vel at 50% speed in opposite direction
#             print "Turn to left"
#             player.brain.nav.walk(0, -.5, .15)
# 
#     elif player.brain.nav.isStopped():
#         player.shouldOrbit = False
#         player.kick.h = 0
#         if player.kick == kicks.ORBIT_KICK_POSITION:
#             print "DEBUG_SUITE: In 'orbitBall', finished with orbit kick. Switching to 'prepareForKick'."
#             return player.goNow('prepareForKick')
#         else:
#             print "DEBUG_SUITE: In 'orbitBall', finished with orbit. Switching to 'positionForKick'."
#             player.kick = kicks.chooseAlignedKickFromKick(player, player.kick)
#             return player.goNow('positionForKick')
#     
#     #Used to update kick.h so we can *ideally* determine how long we've been orbiting
#     prepareForKick.hackKick.shoot()
# 
#     #debugging
#     if orbitBall.counter%25 == 0:
#         print "h is: ", player.kick.h
#         print "stateTime is: ", player.stateTime
# 
#     #hackKick.shoot() is of the opinion that we're pointed in the right direction
#     if player.kick.h > -5 and player.kick.h < 5:
#         print "I'm not orbiting anymore"
#         player.shouldOrbit = False
#         player.kick.h = 0
#         player.kick = kicks.chooseAlignedKickFromKick(player, player.kick)
#         return player.goNow('positionForKick')
# 
#     if player.stateTime > 5:
#         print "In state orbitBall for too long, switching to chase"
#         player.shouldOrbit = False
#         return player.goLater('chase')
# 
#     #These next three if statements might need some fine tuning
#     #ATM that doesn't appear to be the case
#     if player.orbitDistance < player.brain.ball.distance - 1:
#         #We're too far away
#         player.brain.nav.setXSpeed(.15)
#         
#     if player.orbitDistance > player.brain.ball.distance + 1:
#         #We're too close
#         player.brain.nav.setXSpeed(-.15)
# 
#     if player.orbitDistance > player.brain.ball.distance -1 and player.orbitDistance < player.brain.ball.distance +1:
#         #print "We're at a good distance"
#         player.brain.nav.setXSpeed(0)
# 
#     if (transitions.shouldFindBallKick(player) or
#         transitions.shouldCancelOrbit(player)):
#         print "DEBUG_SUITE: In 'orbitBall', either shouldFindBall or shouldCancelOrbit. Switching to 'chase'."
#         player.inKickingState = False
#         return player.goLater('chase')
# 
#     #Keeps track of the number of frames in orbitBall
#     orbitBall.counter = orbitBall.counter + 1
#     return player.stay()
# 
# 
# def positionForKick(player):
#     """
#     Get the ball in the sweet spot
#     """
#     if (transitions.shouldApproachBallAgain(player) or
#         transitions.shouldRedecideKick(player)):
#         print "DEBUG_SUITE: In 'positionForKick', either shouldApproachBallAgain or shouldRedecideKick. Switching to 'chase'."
#         player.inKickingState = False
#         return player.goLater('chase')
# 
#     ball = player.brain.ball
#     kick_pos = player.kick.getPosition()
#     positionForKick.kickPose = RelRobotLocation(ball.rel_x - kick_pos[0],
#                                                 ball.rel_y - kick_pos[1],
#                                                 0)
# 
#     #only enque the new goTo destination once
#     if player.firstFrame():
#         player.ballBeforeApproach = player.brain.ball
#         # Safer when coming from orbit in 1 frame. Still works otherwise, too.
#         player.brain.tracker.lookStraightThenTrack()
#         #TODO: try getting rid of ADAPTIVE here, if ball estimates are good,
#         #we don't need to lower the speed/shuffle to the ball
#         player.brain.nav.goTo(positionForKick.kickPose,
#                               Navigator.PRECISELY,
#                               Navigator.GRADUAL_SPEED,
#                               False,
#                               Navigator.ADAPTIVE)
#     else:
#         player.brain.nav.updateDest(positionForKick.kickPose)
# 
#     if transitions.shouldFindBallKick(player) and player.counter > 15:
#         print "DEBUG_SUITE: In 'positionForKick', both shouldFindBallKick and counter >15. Switching to 'chase'."
#         player.inKickingState = False
#         return player.goLater('chase')
# 
#     if (transitions.ballInPosition(player, positionForKick.kickPose) or
#         player.brain.nav.isAtPosition()):
#         print "DEBUG_SUITE: In 'positionForKick', either ballInPosition or nav.isAtPosition. Switching to 'kickBallExecute'."
#         player.brain.nav.stand()
#         return player.goNow('kickBallExecute')
# 
#     return player.stay()
# 
# # Currently not used as of 6/7/13.
# # TODO: implement this again?
# def lookAround(player):
#     """
#     Nav is stopped. We want to look around to get better loc.
#     """
#     if player.firstFrame():
#         player.stopWalking()
#         player.brain.tracker.stopHeadMoves() # HACK so that tracker goes back to stopped.
#         player.brain.tracker.repeatBasicPan()
# 
#     # Make sure we leave this state...
#     if player.brain.ball.vis.frames_off > 200:
#         return player.goLater('chase')
# 
#     if player.brain.tracker.isStopped() and player.counter > 2:
#             player.brain.tracker.trackBall()
#             player.brain.kickDecider.decideKick()
#             if transitions.shouldOrbit(player) and not player.penaltyKicking:
#                 print "Don't have a kick, orbitting"
#                 return player.goNow('orbitBall')
#             else:
#                 return player.goLater('chase')
# 
#     return player.stay()
