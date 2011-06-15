import ChaseBallTransitions as transitions

"""
Here we house all of the state methods used for kicking the ball
"""

def kickBallExecute(player):
    """
    Kick the ball
    """
    if player.firstFrame():
        player.brain.tracker.trackBall()
        player.executeMove(player.brain.kickDecider.getSweetMove())

        #if player.penaltyKicking:
        if not player.penaltyMadeFirstKick:
            player.penaltyMadeFirstKick = True
        elif not player.penaltyMadeSecondKick:
            player.penaltyMadeSecondKick = True

    if player.counter > 1 and player.brain.nav.isStopped():
        player.brain.nav.justKicked = True
        return player.goLater('afterKick')

    return player.stay()

def afterKick(player):
    """
    State to follow up after a kick.
    Currently exits after one frame.
    """
    # trick the robot into standing up instead of leaning to the side
    if player.firstFrame():
        player.hasAlignedOnce = False
        player.standup()
        player.brain.tracker.trackBall()

        if player.penaltyKicking:
            return player.goLater('penaltyKickRelocalize')

        return player.stay()

    if player.brain.ball.on and player.brain.nav.isStopped():
        player.inKickingState = False
        player.brain.nav.justKicked = False
        return player.goLater('chase')

    if transitions.shouldScanFindBall(player) and player.brain.nav.isStopped():
        player.inKickingState = False
        player.brain.nav.justKicked = False
        return player.goLater('scanFindBall')

    return player.stay()
