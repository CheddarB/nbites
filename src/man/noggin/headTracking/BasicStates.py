from man.motion import HeadMoves

# ** # old method - kept unchanged
def stopped(tracker):
    '''default state where the tracker does nothing'''
    tracker.activeLocOn = False
    return tracker.stay()

# ** # old method - kept unchanged
def stop(tracker):
    ''' stop all head moves '''
    if tracker.firstFrame():
        tracker.activeLocOn = False
        tracker.brain.motion.stopHeadMoves()

    if not tracker.brain.motion.isHeadActive():
        return tracker.goNow('stopped')

    return tracker.stay()

# ** # old method
def neutralHead(tracker):
    '''move head to neutral position'''
    if tracker.firstFrame():
        tracker.activeLocOn = False
        tracker.brain.motion.stopHeadMoves()
        tracker.helper.executeHeadMove(HeadMoves.NEUT_HEADS)

    if not tracker.brain.motion.isHeadActive():
        return tracker.goLater('stopped')

    return tracker.stay()

# ** # old method - kept unchanged
def doHeadMove(tracker):
    '''executes the currently set headMove'''
    if tracker.firstFrame():
        tracker.activeLocOn = False
        tracker.brain.motion.stopHeadMoves()
        tracker.helper.executeHeadMove(tracker.headMove)

    if not tracker.brain.motion.isHeadActive():
        return tracker.goLater('stopped')

    return tracker.stay()
