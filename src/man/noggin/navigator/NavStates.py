""" States for finding our way on the field """

from ..util import MyMath
from . import NavConstants as constants
from . import NavHelper as helper
from . import WalkHelper as walker
from . import NavTransitions as navTrans
from ..objects import RobotLocation

from math import fabs
import copy

DEBUG = False

def doingSweetMove(nav):
    '''State that we stay in while doing sweet moves'''
    if nav.firstFrame():
        nav.doingSweetMove = True
        return nav.stay()

    if not nav.brain.motion.isBodyActive():
        nav.doingSweetMove = False
        return nav.goNow('stopped')

    return nav.stay()

def goToPosition(nav):
    """
    Go to a position set in the navigator. General go to state.  Goes
    towards an x,y position on the field without regard to the
    destination heading. Switches over to omni to finish the heading changes.
    """
    if nav.firstFrame():
        nav.omniWalkToCount = 0
        nav.atPositionCount = 0

    my = nav.brain.my
    dest = nav.getDestination()

    if (navTrans.atDestinationCloser(my, dest) and
        navTrans.atHeading(my, dest.h)):
            nav.atPositionCount += 1
            if nav.atPositionCount > \
            constants.FRAMES_THRESHOLD_TO_AT_POSITION:
                return nav.goNow('atPosition')
    else:
        if not nav.atPositionCount == 0:
            nav.atPositionCount -= 1

    # We don't want to alter the actual destination, we just want a
    # temporary destination for getting the params to walk straight at
    intermediateH = my.headingTo(dest)
    tempDest = RobotLocation(dest.x, dest.y, intermediateH)

    walkX, walkY, walkTheta = walker.getWalkSpinParam(my, tempDest)
    helper.setSpeed(nav, walkX, walkY, walkTheta)

    if navTrans.useFinalHeading(nav.brain, dest):
        nav.omniWalkToCount += 1
        if nav.omniWalkToCount > constants.FRAMES_THRESHOLD_TO_POSITION_OMNI:
            return nav.goLater('omniGoTo')
    else:
        nav.omniWalkToCount = 0

    if navTrans.shouldAvoidObstacle(nav):
        return nav.goLater('avoidObstacle')

    return nav.stay()

def omniGoTo(nav):
    if nav.firstFrame():
        nav.stopOmniCount = 0
        nav.atPositionCount = 0

    my = nav.brain.my
    dest = nav.getDestination()

    if (navTrans.atDestinationCloser(my, dest) and
        navTrans.atHeading(my, dest.h)):
        nav.atPositionCount += 1
        if (nav.atPositionCount >
            constants.FRAMES_THRESHOLD_TO_AT_POSITION):
            return nav.goNow('atPosition')
    else:
        nav.atPositionCount = 0

    walkX, walkY, walkTheta = walker.getOmniWalkParam(my, dest)
    helper.setSpeed(nav, walkX, walkY, walkTheta)

    if not navTrans.useFinalHeading(nav.brain, dest):
        nav.stopOmniCount += 1
        if nav.stopOmniCount > constants.FRAMES_THRESHOLD_TO_GOTO_POSITION:
            return nav.goLater('goToPosition')
    else:
        nav.stopOmniCount = 0

    if navTrans.shouldAvoidObstacle(nav):
        return nav.goLater('avoidObstacle')

    return nav.stay()

# WARNING: avoidObstacle could possibly go into our own box
def avoidObstacle(nav):
    """
    If we detect something in front of us, dodge it
    """

    avoidLeft = navTrans.shouldAvoidObstacleLeft(nav)
    avoidRight = navTrans.shouldAvoidObstacleRight(nav)

    # store previous state here, b/c lastDiffState gets
    # replaced when we perform 'goNow'
    nav.preAvoidState = nav.lastDiffState

    if (avoidLeft and avoidRight):
        return nav.goNow('avoidFrontObstacle')
    elif avoidLeft:
        return nav.goNow('avoidLeftObstacle')
    elif avoidRight:
        return nav.goNow('avoidRightObstacle')
    else:
        return nav.goLater(nav.lastDiffState)


def avoidFrontObstacle(nav):
    # Backup
    # strafe away from the closer one?
    # strafe towards dest?

    # ever a good time to backup?
    # we'll probably want to go forward again and most obstacle
    # are moving, so pausing might make more sense

    if nav.firstFrame():
        nav.doneAvoidingCounter = 0
        nav.printf(nav.brain.sonar)
        nav.printf("Avoid by backup");
        helper.setSpeed(nav, constants.DODGE_BACK_SPEED, 0, 0)

    avoidLeft = navTrans.shouldAvoidObstacleLeft(nav)
    avoidRight = navTrans.shouldAvoidObstacleRight(nav)

    if (avoidLeft and avoidRight):
        nav.doneAvoidingCounter -= 1
        nav.doneAvoidingCounter = max(0, nav.doneAvoidingCounter)
        return nav.stay()

    elif avoidRight:
        return nav.goLater('avoidRightObstacle')

    elif avoidLeft:
        return nav.goLater('avoidLeftObstacle')

    else:
        nav.doneAvoidingCounter += 1

    if nav.doneAvoidingCounter > constants.DONE_AVOIDING_FRAMES_THRESH:
        nav.shouldAvoidObstacleRight = 0
        nav.shouldAvoidObstacleLeft = 0
        return nav.goLater(nav.preAvoidState)

    return nav.stay()

def avoidLeftObstacle(nav):
    """
    dodges right if we only detect something to the left of us
    """

    if nav.firstFrame():
        nav.doneAvoidingCounter = 0
        nav.printf(nav.brain.sonar)
        nav.printf("Avoid by right dodge");
        helper.setSpeed(nav, 0, constants.DODGE_RIGHT_SPEED, 0)

    avoidLeft = navTrans.shouldAvoidObstacleLeft(nav)
    avoidRight = navTrans.shouldAvoidObstacleRight(nav)

    if (avoidLeft and avoidRight):
        return nav.goLater('avoidFrontObstacle')
    elif avoidRight:
        return nav.goLater('avoidRightObstacle')
    elif avoidLeft:
        nav.doneAvoidingCounter -= 1
        nav.doneAvoidingCounter = max(0, nav.doneAvoidingCounter)
        return nav.stay()
    else:
        nav.doneAvoidingCounter += 1

    if nav.doneAvoidingCounter > constants.DONE_AVOIDING_FRAMES_THRESH:
        nav.shouldAvoidObstacleRight = 0
        nav.shouldAvoidObstacleLeft = 0
        return nav.goLater(nav.preAvoidState)

    return nav.stay()

def avoidRightObstacle(nav):
    """
    dodges left if we only detect something to the left of us
    """

    if nav.firstFrame():
        nav.doneAvoidingCounter = 0
        nav.printf(nav.brain.sonar)
        nav.printf("Avoid by left dodge");
        helper.setSpeed(nav, 0, constants.DODGE_LEFT_SPEED, 0)

    avoidLeft = navTrans.shouldAvoidObstacleLeft(nav)
    avoidRight = navTrans.shouldAvoidObstacleRight(nav)

    if (avoidLeft and avoidRight):
        return nav.goLater('avoidFrontObstacle')
    elif avoidLeft:
        return nav.goLater('avoidLeftObstacle')
    elif avoidRight:
        nav.doneAvoidingCounter -= 1
        nav.doneAvoidingCounter = max(0, nav.doneAvoidingCounter)
        return nav.stay()
    else:
        nav.doneAvoidingCounter += 1

    if nav.doneAvoidingCounter > constants.DONE_AVOIDING_FRAMES_THRESH:
        nav.shouldAvoidObstacleRight = 0
        nav.shouldAvoidObstacleLeft = 0
        return nav.goLater(nav.preAvoidState)

    return nav.stay()


# State to be used with standard setSpeed movement
def walking(nav):
    """
    State to be used when setSpeed is called
    """
    helper.setSpeed(nav, nav.walkX, nav.walkY, nav.walkTheta)

    return nav.stay()

def destWalking(nav):
    """
    State to be used when we are walking to a destination
    """
    if nav.firstFrame() or nav.newDestination:
        if (nav.destGain < 0):
            nav.destGain = 1;

        nav.nearDestination = False
        nav.newDestination = False

        if (nav.destX == 0 and nav.destY == 0 and nav.destTheta == 0) \
               or nav.destGain == 0:
            return nav.goNow('stop')

        helper.setDestination(nav, nav.destX, nav.destY, nav.destTheta, nav.destGain)

    framesLeft = nav.currentCommand.framesRemaining()

    # the frames remaining counter is sometimes set to -1 initially
    if framesLeft != -1 and framesLeft < 40:
        nav.nearDestination = True

    if nav.currentCommand.isDone():
        nav.nearDestination = True
        return nav.goNow('stop')

    return nav.stay()

### Stopping States ###
def stop(nav):
    """
    Wait until the walk is finished.
    """
    if nav.firstFrame():
        # stop walk vectors
        helper.setSpeed(nav, 0, 0, 0)

        # stop destination walking
        nav.destX = nav.destY = nav.destTheta = 0
        helper.setDestination(nav, 0, 0, 0, 1)

    if not nav.brain.motion.isWalkActive():
        return nav.goNow('stopped')

    return nav.stay()

def stopped(nav):
    return nav.stay()

def orbitPointThruAngle(nav):
    """
    Circles around a point in front of robot, for a certain angle
    """
    if fabs(nav.angleToOrbit) < constants.MIN_ORBIT_ANGLE:
        return nav.goNow('stop')

    if nav.angleToOrbit < 0:
        orbitDir = constants.ORBIT_LEFT
    else:
        orbitDir = constants.ORBIT_RIGHT

    #determine speeds for orbit
    ball = nav.brain.ball

    #want x to keep a radius of 17 from the ball, increase and
    #decrease x velocity as we move farther away from that dist
    walkX = (ball.relX - 18) * .045

    #keep constant y velocity, let x and theta changea
    walkY = orbitDir * .8

    #Vary theta based on ball bearing.  increase theta velocity as
    #we get farther away from facing the ball
    walkTheta = orbitDir * ball.bearing * .035

    #set speed for orbit
    helper.setSpeed(nav, walkX, walkY, walkTheta )

    #Funny enough, we orbit about 1 degree a frame,
    #So the angle can be used as a thresh

    if nav.counter >= nav.angleToOrbit:
        return nav.goLater('stop')
    return nav.stay()

def positionForKick(nav):
    """
    This state is called by player through Navigator::kickPosition(kick)
    It will position the robot at the ball using self.kick to determine the x,y
    offset and the final heading.

    This state will aggresively omni-walk, so it's probably best if we don't call
    it until we're near the ball.
    """
    ball = nav.brain.ball

    # we've either just started, or are close to our last destination
    # tell the robot where to go!
    if nav.firstFrame():
        nav.destX = ball.relX - nav.kick.x_offset -2 # HACK!!!
        nav.destY = ball.relY - nav.kick.y_offset

        nav.destTheta = ball.bearing

        # TODO later?
        #nav.destTheta = nav.kick.heading - nav.brain.my.h

        # slow down as we get near the ball (max 80% speed)
        if ball.dist < 30:
            nav.destGain = (0.4 + (ball.dist / 30)) * .8
        else:
            nav.destGain = .8

        nav.newDestination = True

        print 'Ball rel X: {0} Y: {1} ball bearing: {2}' \
              .format(ball.relX, ball.relY, ball.bearing)
        print 'Set new PFK destination of ({0}, {1}, {2}, gain={3})' \
              .format(nav.destX, nav.destY, nav.destTheta, nav.destGain)

        nav.brain.speech.say("New destination")

        return nav.goNow('destWalking')


def atPosition(nav):
    if nav.firstFrame():
        nav.brain.speech.say("At Position")
        helper.setSpeed(nav, 0, 0, 0)
        nav.startOmniCount = 0

    my = nav.brain.my
    dest = nav.getDestination()

    if navTrans.atDestinationCloser(my, dest) and \
            navTrans.atHeading(my, dest.h):
        nav.startOmniCount = 0
        return nav.stay()

    else:
        nav.startOmniCount += 1
        if nav.startOmniCount > constants.FRAMES_THRESHOLD_TO_POSITION_OMNI:
            return nav.goLater('omniGoTo')
        else:
            return nav.stay()
