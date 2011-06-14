from .. import NogginConstants as NogCon

BALL_SAVE_LIMIT_TIME = 2.5
MOVE_TO_SAVE_DIST_THRESH = 200.

# We dont use STRAFE ONLY RIGHT NOW
STRAFE_ONLY = True
STRAFE_SPEED = 0.3

# Distance at which we use active localization
ACTIVE_LOC_THRESH = 150.

#Buffer for size of the goalie box so that the
#Goalie will clear a ball in his box
CENTER_SAVE_THRESH = 15
BOX_BUFFER = 10
SAVE_BUFFER = 20
CHASE_RIGHT_X_LIMIT = NogCon.MY_GOALBOX_RIGHT_X + 70
CHASE_UPPER_Y_LIMIT = NogCon.MY_GOALBOX_BOTTOM_Y - BOX_BUFFER
CHASE_LOWER_Y_LIMIT = NogCon.MY_GOALBOX_TOP_Y + BOX_BUFFER

CLOSE_BEHIND_GOALIE = -10

VEL_THRES = 11
