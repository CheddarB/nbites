from man.motion import MotionConstants
from . import TrackingConstants as constants

DEBUG = False

# ** # old method
def ballTracking(tracker):
    '''Super state which handles following/refinding the ball'''
    if tracker.target.framesOff <= constants.TRACKER_FRAMES_OFF_REFIND_THRESH:
        return tracker.goNow('tracking')
    else:
        return tracker.goNow('scanBall')

# ** # old method
def tracking(tracker):
    """
    state askes it's parent (the tracker) for an object or angles to track
    while the object is on screen, or angles are passed, we track it.
    Otherwise, we continually write the current values into motion via setHeads.

    If a sweet move is begun while we are tracking, the current setup is to let
    the sweet move conclude and then resume tracking afterward.
    """

    if tracker.firstFrame():
        tracker.activeLocOn = False

    tracker.helper.trackObject()
    if not tracker.target.on:
        if DEBUG : tracker.printf("Missing object this frame",'cyan')
        if tracker.target.framesOff > constants.TRACKER_FRAMES_OFF_REFIND_THRESH:
            return tracker.goLater(tracker.lastDiffState)
        return tracker.stay()

    return tracker.stay()

# ** # old method
def ballSpinTracking(tracker):
    '''Super state which handles following/refinding the ball'''
    if tracker.target.framesOff <= constants.TRACKER_FRAMES_OFF_REFIND_THRESH:
        return tracker.goNow('tracking')
    else:
        return tracker.goNow('spinScanBall')


# ** # old method
def activeTracking(tracker):
    """
    Method to perform tracking
    """
    if tracker.firstFrame():
        tracker.activeLocOn = True

    if tracker.target.locDist < constants.MAX_ACTIVE_TRACKING_DIST and \
            tracker.target.framesOn > 2:
        return tracker.goLater('tracking')

    if tracker.target.framesOff > constants.TRACKER_FRAMES_OFF_REFIND_THRESH \
            and not tracker.brain.motion.isHeadActive() \
            and not (tracker.activePanOut):
        return tracker.goLater('activeLocScan')

    elif not tracker.activePanOut and \
            tracker.counter <= constants.ACTIVE_LOC_STARE_THRESH:
        tracker.helper.trackObject()
        tracker.activePanOut = False
        return tracker.stay()

    elif tracker.activePanOut:
        tracker.activePanOut = False
        return tracker.goLater('returnHeadsPan')
    else :
        tracker.activePanOut = True
        motionAngles = tracker.brain.sensors.motionAngles
        tracker.preActivePanHeads = (
            motionAngles[MotionConstants.HeadYaw],
            motionAngles[MotionConstants.HeadPitch])

        return tracker.goLater('trianglePan')

    return tracker.stay()

# ** # new method
def trackLoc(tracker):
    """
    Look at the current LocObject, then go to lastDiffState
    """
    # make sure head is inactive first
    if tracker.firstFrame():
        tracker.brain.motion.stopHeadMoves()

    # safety check that currentLocObject was set
    if tracker.currentLocObject is None:
        print "currentLocObject is None"
        return tracker.goLater(tracker.lastDiffState())

    tracker.lookToLocObject()

    if not tracker.brain.motion.isHeadActive():
        print "head is not active"
        return tracker.goLater(tracker.lastDiffState)

    return tracker.stay()
