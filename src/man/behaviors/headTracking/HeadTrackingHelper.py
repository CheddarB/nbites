from . import TrackingConstants as constants
from .. import MotionConstants
from ..util import MyMath as MyMath
from .. import StiffnessModes
from math import fabs, degrees
import HeadMoves

class HeadTrackingHelper(object):
    def __init__(self, tracker):
        self.tracker = tracker

    def executeHeadMove(self, headMove):
        """performs a sweetmove"""
        command = self.tracker.brain.interface.headMotionCommand
        command.type = command.CommandType.SCRIPTED_HEAD_COMMAND

        for position in headMove:
            if len(position) == 4:
                headJoints = command.scripted_command.add_commands()

                headJoints.time = position[1]
                if position[2] == 1:
                    headJoints.interpolation = headJoints.InterpolationType.LINEAR
                else:
                    headJoints.interpolation = headJoints.InterpolationType.SMOOTH

                # Only set the head angles, since this command never sets any other angles
                headJoints.angles.head_yaw = position[0][0]
                headJoints.angles.head_pitch = position[0][1]

                # Set all stiffnesses, since this command specifies them all
                headJoints.stiffness.head_yaw =         position[3][0]
                headJoints.stiffness.head_pitch =       position[3][1]

                headJoints.stiffness.l_shoulder_pitch = position[3][2]
                headJoints.stiffness.l_shoulder_roll =  position[3][3]
                headJoints.stiffness.l_elbow_yaw =      position[3][4]
                headJoints.stiffness.l_elbow_roll =     position[3][5]

                headJoints.stiffness.r_shoulder_pitch = position[3][18]
                headJoints.stiffness.r_shoulder_roll =  position[3][19]
                headJoints.stiffness.r_elbow_yaw =      position[3][20]
                headJoints.stiffness.r_elbow_roll =     position[3][21]

                headJoints.stiffness.l_hip_yaw_pitch =  position[3][6]
                headJoints.stiffness.l_hip_roll =       position[3][7]
                headJoints.stiffness.l_hip_pitch =      position[3][8]
                headJoints.stiffness.l_knee_pitch =     position[3][9]
                headJoints.stiffness.l_ankle_pitch =    position[3][10]
                headJoints.stiffness.l_ankle_roll =     position[3][11]

                headJoints.stiffness.r_hip_yaw_pitch =  position[3][12]
                headJoints.stiffness.r_hip_roll =       position[3][13]
                headJoints.stiffness.r_hip_pitch =      position[3][14]
                headJoints.stiffness.r_knee_pitch =     position[3][15]
                headJoints.stiffness.r_ankle_pitch =    position[3][16]
                headJoints.stiffness.r_ankle_roll =     position[3][17]

            else:
                self.tracker.printf("What kind of sweet ass-Move is this?")

        command.process_by_motion = False

    def startingPan(self, headMove):
        """Calculates the first part of a fixed pitch pan to get there quickly."""
        if len(headMove) < 2:
            # Not a normal pan: there's only 1 headMove.
            # Don't do a starting move.
            return

        headMoveYaw = headMove[1][0][0]
        headMovePitch = headMove[1][0][1]

        curYaw = degrees(self.tracker.brain.interface.jointAngles.head_yaw)
        degreesPerSecond = 80 #fast, but hopefully won't destabilize the walk much
        yawDiff = MyMath.fabs(curYaw - headMoveYaw)
        totalTime = yawDiff/degreesPerSecond

        newHeadMove = ( ((headMoveYaw,headMovePitch), totalTime, 1,
                         StiffnessModes.LOW_HEAD_STIFFNESSES), )

        self.executeHeadMove(newHeadMove)

    # Should be generalized.
    def convertKickPan(self, headMove, invert):
        """
        Converts the first step of the kick pan to have the same speed
        as the second step, regardless of starting yaw.
        ASSERT: 2 step headMove.
        """
        headMoveYaw = headMove[0][0][0]
        headMovePitch = headMove[0][0][1]

        curYaw = degrees(self.tracker.brain.interface.jointAngles.head_yaw)
        degreesPerSecond = (headMoveYaw*2)/headMove[0][1]
        yawDiff = MyMath.fabs(curYaw-headMoveYaw)
        totalTime = yawDiff/degreesPerSecond

        if invert is True:
            newHeadMove = ( ((-1*headMoveYaw,headMovePitch), totalTime, 1,
                             StiffnessModes.LOW_HEAD_STIFFNESSES),
                            ((-1*headMove[1][0][0],headMove[1][0][1]),
                             headMove[1][1], headMove[1][2], headMove[1][3]) )
        else:
            newHeadMove = ( ((headMoveYaw,headMovePitch), totalTime, 1,
                             StiffnessModes.LOW_HEAD_STIFFNESSES),
                            headMove[1] )

        return newHeadMove

    def trackObject(self):
        """
        Method to actually perform the tracking.
        Should only be called explicitly from state
        methods in TrackingStates.py
        """
        target = self.tracker.target

        changeX, changeY = 0.0 , 0.0

        # If we don't see it, let's try to use our model of it to find
        # it and track it

        # TODO: generalize this to objects which we cannot see.
        if not target or \
                (target.loc.relX == 0.0 and target.loc.relY == 0.0):
            return

        # If we haven't seen the object in the very recent past, look
        # towards where the model says it is. The framesOff > 3
        # provides a buffer to ensure that it's not just a flickering
        # image problem (prevents twitchy robot)
        if target.vis.framesOff > 3:
            self.lookToPoint(target)
            return

        # Find the target's angular distance from the center of the screen
        # if we have an object, track that
        changeX = target.vis.angle_x_deg
        changeY = target.vis.angle_y_deg #the pitch is pos = down

        curPitch = degrees(self.tracker.brain.interface.jointAngles.head_pitch)
        curYaw   = degrees(self.tracker.brain.interface.jointAngles.head_yaw)

        maxChange = 13.0

        # Warning- no gain is applied currently!
        safeChangeX = MyMath.clip(changeX, -maxChange, maxChange )
        safeChangeY = MyMath.clip(changeY, -maxChange, maxChange )

        newYaw = curYaw + safeChangeX
        newPitch = curPitch - safeChangeY

        newYaw = MyMath.clip(newYaw, -80., 80.)

        maxSpeed = 2.0

        # Set message fields
        command = self.tracker.brain.interface.headMotionCommand
        command.type = command.CommandType.SET_HEAD_COMMAND

        command.set_command.head_yaw = newYaw
        command.set_command.head_pitch = newPitch
        command.set_command.max_speed_yaw = maxSpeed
        command.set_command.max_speed_pitch = maxSpeed

        command.process_by_motion = False

    # Fixed Pitch
    def trackObjectFixedPitch(self):
        """
        Analogous to trackObject, but with a fixed value for pitch.
        """
        target = self.tracker.target
        changeX, changeY = 0.0, 0.0

        # If we cannot see the target, abort.
        if not target or \
                (target.rel_x == 0.0 and target.rel_y == 0.0) or \
                (target.vis.frames_off > 0 and target.vis.frames_off < 3):
            return

        # If we haven't seen the target, look towards loc model.
        if target.vis.frames_off > 3:
            self.lookToPointFixedPitch(target)
            return

        # Assert: target is visible.

        # Find the target's angular distance from yaw center.
        changeX = target.vis.angle_x_deg
        # ignore changeY: pitch is fixed

        curPitch = degrees(self.tracker.brain.interface.jointAngles.head_pitch)
        curYaw =   degrees(self.tracker.brain.interface.jointAngles.head_yaw)

        maxChange = 13.0

        # Warning- no gain is applied currently!
        safeChangeX = MyMath.clip(changeX, -maxChange, maxChange)
        # ignore safeChangeY: pitch is fixed

        newYaw = curYaw + safeChangeX
        # ignore newPitch: pitch is fixed

        maxSpeed = 2.0

        # Set message fields
        command = self.tracker.brain.interface.headMotionCommand
        command.type = command.CommandType.SET_HEAD_COMMAND

        command.set_command.head_yaw = newYaw
        command.set_command.head_pitch = 20.0 # MAKE A CONSTANT FOR THIS
        command.set_command.max_speed_yaw = maxSpeed
        command.set_command.max_speed_pitch = maxSpeed

        command.process_by_motion = False

    # Not called anywhere in the code.
    def lookToTargetAngles(self, target):
        """
        Uses setHeadCommands to bring given target to center of frame.
        """
        pitch = degrees(self.tracker.brain.interface.jointAngles.head_pitch)
        yaw =   degrees(self.tracker.brain.interface.jointAngles.head_yaw)

        # Find the target's angular distance from the center of vision
        if not target is None and target.vis.on:
            yaw += target.vis.angle_x_deg
            pitch -= target.vis.angle_y_deg
            #note: angleY is positive up from center of vision frame
            #note: angleX is positive left from center of vision frame
        else:
            # by default, do nothing
            return

        # Set message fields
        command = self.tracker.brain.interface.headMotionCommand
        command.type = command.CommandType.SET_HEAD_COMMAND

        command.set_command.head_yaw = yaw
        command.set_command.head_pitch = pitch
        # Leave max speeds to default
        command.process_by_motion = False

    def panTo(self, heads):
        """
        Pan heads at appropriate speed to given heads
        """
        headPitch = degrees(self.tracker.brain.interface.jointAngles.head_pitch)
        headYaw =   degrees(self.tracker.brain.interface.jointAngles.head_yaw)

        yawDiff = fabs(heads[0] - headYaw)
        pitchDiff = fabs(heads[1] - headPitch)

        maxDiff = max(pitchDiff, yawDiff)
        panTime = maxDiff/constants.MAX_PAN_SPEED
        self.executeHeadMove(((heads, panTime, 0,
                               StiffnessModes.LOW_HEAD_STIFFNESSES), ))

    def lookToPoint(self, target):
        if hasattr(target, "height"):
            height = target.height
        else:
            height = 0

        if hasattr(target, "loc"):
            target = target.loc

        print str(target.rel_x) + " " + str(target.rel_y)

        headMove = motion.CoordHeadCommand(relX = target.rel_x,
                                           relY = target.rel_y,
                                           relZ = height)

        self.tracker.brain.motion.coordHead(headMove)
        return headMove

    # Fixed Pitch
    def lookToPointFixedPitch(self, target):
        """
        If the relative y is positive, look left. Otherwise, look right.
        """
        if hasattr(target, "height"):
            height = target.height
        else:
            height = 0

        if hasattr(target, "loc"):
            target = target.loc

        if target.relY > 0:
            self.executeHeadMove(HeadMoves.FIXED_PITCH_LOOK_LEFT)
        else:
            self.executeHeadMove(HeadMoves.FIXED_PITCH_LOOK_RIGHT)

    # broken?
    # Not called anywhere in code.
    # Unsafe to call as of 4/5/2013
    def lookToAngles(self, yaw=0, pitch=0):
        headMove = motion.SetHeadCommand(MyMath.degrees(yaw),
                                         MyMath.degrees(pitch))
        self.tracker.brain.motion.setHead(headMove)

    # Fixed Pitch
    def lookToAngleFixedPitch(self, yaw):
        """
        Looks to the given yaw at the appropriate fixed pitch.
        NOTE: should call brain.motion.stopHeadMoves() first!
        """
        if yaw > 55 or yaw < -55:
            pitch = 11.0
        else:
            pitch = 17.0

        self.executeHeadMove((((yaw, pitch), 2.0, 1,
                               StiffnessModes.LOW_HEAD_STIFFNESSES),))

    # Consider updating this for new loc and vision systems (summer 2012)
    # Currently broken in Portals system.
    def calculateClosestLandmark(self):
        brain = self.tracker.brain
        posts = [brain.yglp, brain.ygrp, brain.bgrp, brain.bglp]

        currYaw = degrees(self.tracker.brain.interface.jointAngles.head_yaw)

        minDiff = 1000000000
        bestPost = None

        for p in posts:
            diff = MyMath.sub180Angle(currYaw - p.bearing)

            if diff < minDiff:
                bestPost = p
                minDiff = diff
        return bestPost

    # Debug method to print current head pitch and yaw.
    def printHeadAngles(self):
        headPitch = self.tracker.brain.interface.jointAngles.head_pitch
        yawPitch = self.tracker.brain.interface.jointAngles.head_yaw

        self.tracker.printf("Head pitch is:")
        self.tracker.printf(curPitch)
        self.tracker.printf("Head yaw is:")
        self.tracker.printf(curYaw)
