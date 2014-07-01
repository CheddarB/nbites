from ..navigator import BrunswickSpeeds as speeds

DEBUG_ORBIT = False

# Transitions' Constants
# Ball on and off frame thresholds
BALL_ON_THRESH = 1
BALL_OFF_THRESH = 10
BALL_OFF_KICK_THRESH = 4 # 4 seconds, time threshold!

# Should position for kick
BALL_PFK_LEFT_Y = 16.0
PREPARE_FOR_KICK_DIST = 50.0
SETUP_DISTANCE_X = 5
SLOW_DOWN_TO_BALL_DIST = 20.
WAIT_COUNT = 0

# Should approach ball again
APPROACH_BALL_AGAIN_DIST = PREPARE_FOR_KICK_DIST + 25.0
BALL_MOVED_THR = 3

# Orbit
ORBIT_BALL_DISTANCE = 25
ORBIT_DISTANCE_FAR = 7
ORBIT_DISTANCE_CLOSE = 1
ORBIT_DISTANCE_GOOD = 2
ORBIT_GOOD_BEARING = 10
SHOULD_CANCEL_ORBIT_BALL_DIST = APPROACH_BALL_AGAIN_DIST
ORBIT_TOO_LONG_THR = 8

# Should kick again
SHOULD_KICK_AGAIN_CLOSE_X = 9.5
SHOULD_KICK_AGAIN_FAR_X = 25
SHOULD_KICK_AGAIN_Y = 15

# Ball in position
## NOTE: important that these values slightly overestimate the values in
##        navigator's PFK. Otherwise there can be bad looping.
BALL_X_OFFSET = 1.5
BALL_Y_OFFSET = 1.5
GOOD_ENOUGH_H = 6

# States' constants

# Spin To Ball
FIND_BALL_SPIN_SPEED = .5
FACING_BALL_ACCEPTABLE_BEARING = 1.5
SHOULD_SPIN_TO_BALL_BEARING = 25.0

# find ball
SPUN_ONCE_TIME_THRESH = 8
WALK_FIND_BALL_TIME_THRESH = SPUN_ONCE_TIME_THRESH * 3 # Roughly 3 spins
