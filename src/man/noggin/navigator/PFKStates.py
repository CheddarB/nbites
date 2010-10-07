from man.noggin.util import MyMath
from . import WalkHelper as walker
from . import NavHelper as helper
from . import NavConstants as constants
from . import BrunswickSpeeds as speeds
from math import fabs

# Values for controlling the speeds
PFK_LEFT_SPIN_SPEED = speeds.LEFT_SPIN_WHILE_X_MAX_SPEED
PFK_RIGHT_SPIN_SPEED = speeds.RIGHT_SPIN_WHILE_X_MAX_SPEED
PFK_LEFT_SPEED = speeds.OMNI_LEFT_MAX_SPEED
PFK_RIGHT_SPEED = speeds.OMNI_RIGHT_MAX_SPEED
PFK_FWD_SPEED = speeds.OMNI_FWD_MAX_SPEED
PFK_REV_SPEED = speeds.OMNI_REV_MAX_SPEED
PFK_MIN_Y_MAGNITUDE = speeds.MIN_OMNI_Y_MAGNITUDE
PFK_MIN_X_MAGNITUDE = speeds.MIN_OMNI_X_MAGNITUDE
PFK_MIN_SPIN_MAGNITUDE = speeds.MIN_SPIN_WHILE_X_MAGNITUDE

# Buffering values, insure that we eventually kick the ball
PFK_CLOSE_ENOUGH_XY = 2.0
PFK_CLOSE_ENOUGH_THETA = 11

"""
Control State for PFK. Breaks up the problem into
Theta, Y, and X. We want to make sure that we kick
and don't try to get perfect position, so we get
each direction good enough and then don't look at
it again.
"""
def pfk_all(nav):

    sX = 0.0            # speed in the x direction
    sY = 0.0            # speed in the y direction
    sTheta = 0.0        # speed in the theta direction

    if nav.firstFrame():
        nav.stopTheta = False
        nav.stopY = False
        nav.stopX = False
        print "entered from: ", nav.lastDiffState

    # get our ideal relative positionings from the kick
    (x_offset, y_offset, heading) = nav.kick.getPosition()

    ball = nav.brain.ball

    """
    # determine the theta speed if our position isn't good enough
    if not nav.stopTheta:
        sTheta = pfk_theta(nav, ball, heading)
    """

    # determine the y speed if our position isn't good enough
    if not nav.stopY:
        sY = pfk_y(nav, ball, y_offset)
        #if sY interferes with what sTheta says, dangerous ball.
            #Move X and Y only?

    # determine the x speed if our position isn't good enough
    if not nav.stopX:
        sX = pfk_x(nav, ball, x_offset)
        if (sX < 0):
            # possible dangerous ball move back slowly
            helper.setSpeed(nav, sX, 0, 0)
            return nav.stay()

    if (nav.stopX and nav.stopY and nav.stopTheta):
        # in good position for kick
        return nav.goNow('stop')

    helper.setSpeed(nav, sX, sY, sTheta)

    return nav.stay()

"""
Determines the speed in the theta direction to position
accurately on the ball and returns that value
"""
#def pfk_theta(nav, ball, targetTheta)
#
# NOT IMPLEMENTED!!!
#
# Must use a global heading target based on a global robot heading.
# Currently loc is unreliable so this cannot be done well.
# Ideally, the kick would be determined at the time we decide to chase
# the ball, maybe changed dynamically if we start recognizing other
# robots, or if we have to avoid along the way. This would mean
# positioning along a good theta direction. When motion has a good
# OmniWalk, or walking along a path functionality, this would be more
# achievable, maybe included in the motion system.

"""
Determines the speed in the y direction to position
accurately on the ball and returns that value
"""
def pfk_y(nav, ball, targetY):

    targetDist = ball.relY - targetY

    if (fabs(targetDist) <= PFK_CLOSE_ENOUGH_XY):
        nav.stopY = True
        return 0

    # Edge of acceptable area for kicking
    distToSpeedModifier = targetY + PFK_CLOSE_ENOUGH_XY

    if (targetDist > 0):
        # Move left to match ball with target
        # Change the distance to a speed and clip within vector limits
        return MyMath.clip(targetDist/distToSpeedModifier - 1,
                           PFK_MIN_Y_MAGNITUDE,
                           PFK_LEFT_SPEED)

    else:
        # Move right to match ball with target
        # Change the distance to a speed and clip within vector limits
        return MyMath.clip(targetDist/distToSpeedModifier + 1,
                           PFK_RIGHT_SPEED,
                           -PFK_MIN_Y_MAGNITUDE)

"""
Determines the speed in the x direction to position
accurately on the ball and returns that value
"""
def pfk_x(nav, ball, targetX):

    targetDist = ball.relX - targetX

    if (fabs(targetDist) <= PFK_CLOSE_ENOUGH_XY):
        nav.stopX = True
        return 0

    # Edge of acceptable area for kicking
    distToSpeedModifier = targetX + PFK_CLOSE_ENOUGH_XY

    if (targetDist > 0):
        # Move foward to match ball with target
        # Change the distance to a speed and clip within vector limits
        return MyMath.clip(targetDist/distToSpeedModifier - 1,
                           PFK_MIN_X_MAGNITUDE,
                           PFK_FWD_SPEED)

    else:
        # Move Backwards
        print "dangerous ball detected during PFK"
        # Move back slowly
        return -PFK_MIN_X_MAGNITUDE
