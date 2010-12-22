// This file is part of Man, a robotic perception, locomotion, and
// team strategy application created by the Northern Bites RoboCup
// team of Bowdoin College in Brunswick, Maine, for the Aldebaran
// Nao robot.
//
// Man is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Man is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
// GNU Lesser Public License for more details.
//
// You should have received a copy of the GNU General Public License
// and the GNU Lesser Public License along with Man.  If not, see
// <http://www.gnu.org/licenses/>.


/*
 * Context - holds context information on field objects in order to
 * better recognize them and identify them.
 */

#include <iostream>
#include "Context.h"
#include "debug.h"
#include "FieldConstants.h"
#include "Utility.h"
#include <vector>
#include <list>
#include <boost/shared_ptr.hpp>

using namespace std;

Context::Context(Vision *vis, Threshold* thr, Field* fie)
    : vision(vis), thresh(thr), field(fie)
{
    // Initialize the array of VisualFieldObject which we use for distance
    // based identification of corners
    allFieldObjects[0] = vision->bgrp;
    allFieldObjects[1] = vision->bglp;
    allFieldObjects[2] = vision->ygrp;
    allFieldObjects[3] = vision->yglp;
#ifdef OFFLINE
    debugIdentifyCorners = false;
#endif

    init();
}

void Context::init() {
    rightYellowPost = false;
    leftYellowPost = false;
    unknownYellowPost = false;
    yellowPost = false;
    rightBluePost = false;
    leftBluePost = false;
    unknownBluePost = false;
    bluePost = false;
    tCorner = 0;
    lCorner = 0;
    iCorner = 0;
    oCorner = 0;
    cCorner = 0;
    cross = false;
    unknownCross = false;
    yellowCross = false;
    blueCross = false;
    ball = false;
    seeGoalBoxLines = false;
    seeCenterCircle = false;
    sameHalf = false;
    face = FACING_UNKNOWN;
    objectRightX = -1;
}

/**
 * Given a list of VisualCorners, attempts to assign ConcreteCorners (ideally
 * one, but sometimes multiple) that correspond with where the corner could
 * possibly be on the field.  For instance, if we have a T corner and it is
 * right next to the blue goal left post, then it is the blue goal right T.
 * Modifies the corners passed in by calling the setPossibleCorners method;
 * in certain cases the shape of a corner might be switched too (if an L
 * corner is determined to be a T instead, its shape is changed accordingly).
 */
void Context::identifyCorners(list <VisualCorner> &corners)
{
    if (corners.size() == 0) {
        // nothing to see here - move along
        return;
    }

    // Figure out what goal posts can provide us with contextual cues
    vector <const VisualFieldObject*> visibleObjects = getVisibleFieldObjects();

    setFacing();
    setFieldHalf();

    if (debugIdentifyCorners) {
        printContext();
    }

	int numCorners = corners.size();
    // collect up some information on corners that are connected
	if (numCorners > 1) {
        checkForConnectedCorners(corners);
	} else {
        list <VisualCorner>::iterator one = corners.begin();
        if (one->getShape() == INNER_L) {
            classifyInnerL(*one);
        }
    }

	// We might later use uncertain objects, but they cause problems. e.g. if you see
	// one post as 2 posts (both the left and right), you get really bad things or
	// if one post is badly estimated.
	if (visibleObjects.empty()) {
		visibleObjects = getAllVisibleFieldObjects();
    }

    checkForSidelineInformation(corners);

    // No explicit movement of iterator; will do it manually
    for (list <VisualCorner>::iterator i = corners.begin();i != corners.end();){
        if (debugIdentifyCorners) {
            cout << endl << "Before identification: Corner: "
                 << endl << "\t" << *i << endl;
			cout << "     Shape info: " << i->getSecondaryShape() << endl;
        }

        const list <const ConcreteCorner*> possibleClassifications =
            classifyCornerWithObjects(*i, visibleObjects);

        // Keep it completely abstract
		// NOTE: this is dumb - we need to be smarter.  E.g. at least
		// eliminate posts that are WAY off
		// Sometimes the distances fail because the object is occluded
		// so we need another method for this case
		// also, this may be a chance to detect a bad post
        if (possibleClassifications.empty()) {
            i->setPossibleCorners(ConcreteCorner::getPossibleCorners(
                                      i->getShape(), i->getSecondaryShape()));
            if (debugIdentifyCorners) {
                cout << "No matches were found for this corner; going to keep "
                     << "it completely abstract." << endl;
                printPossibilities(i->getPossibleCorners());
            }
            ++i;
        }

        // It is unique, append to the front of the list
        // For localization we want the positively identified corners to come
        // first so  that  they can inform the localization system and help
        // identify abstract corners that might be in the frame
        else if (possibleClassifications.size() == 1) {
            if (debugIdentifyCorners) {
                cout << "Only one possibility; appending to the front of the "
                     << "list " << endl;
                printPossibilities(possibleClassifications);
            }

            VisualCorner copy = *i;
            copy.setPossibleCorners(possibleClassifications);
            // This has the effect of incrementing our iterator and deleting the
            // corner from our list.
            i = corners.erase(i);
            corners.push_front(copy);
        }
        // More than 1 possibility for the corner
        else {
			// if we have more corners then those may help us ID the corner
			if (numCorners > 1) {
				if 	(i->getShape() == T) {
					if (tCorner > 1) {
						// for now we'll just toss these
						// @TODO: Theoretically we can classify these
						if (debugIdentifyCorners) {
							cout << "Two Ts found - for now we throw them both out" << endl;
						}
						corners.clear();
						return;
					}
				}
			}

            // If either of the lines forming the corner are cc lines, then
            // the corner must be a cc intersection
            if (i->getShape() == CIRCLE ||
                i->getLine1()->getCCLine() ||
                i->getLine2()->getCCLine()) {
                i->setPossibleCorners(ConcreteCorner::ccCorners());
                i->setShape(CIRCLE);
            } else {
                i->setPossibleCorners(possibleClassifications);
				i->identifyLinesInCorner();
            }
            if (debugIdentifyCorners) {
                printPossibilities(i->getPossibleCorners());
            }
            ++i;
        }
    }

    for (list <VisualCorner>::iterator i = corners.begin();
		 i != corners.end(); ++i){
		i->identifyFromLines();
		i->identifyLinesInCorner();
		if (debugIdentifyCorners) {
			printPossibilities(i->getPossibleCorners());
        }
	}

	// If our corners have no identity, set them to their shape possibilities
    for (list <VisualCorner>::iterator i = corners.begin();
		 i != corners.end(); ++i){
		if (i->getPossibleCorners().empty()) {
			i->setPossibleCorners(ConcreteCorner::getPossibleCorners(i->getShape(),
																	 i->getSecondaryShape()));
        }
	}
}

/* Loop through the corners looking for corners that may be a sideline.  If
   we see one that may help with context, plus it narrows the possibilities
   for that corner.
 */
void Context::checkForSidelineInformation(list<VisualCorner> &corners) {
    for (list <VisualCorner>::iterator i = corners.begin();i != corners.end();++i){
		// before we start, analyze the corner a bit more
		if (i->getSecondaryShape() == UNKNOWN && i->getShape() == T && face == FACING_UNKNOWN) {
			// really long TStems indicate that we have a center T
			// in this particular case sometimes we should be able to absolutely
			// identify the T - if the stem is pointing relatively left or right
			if (i->getTStem()->getLength() > 2 * GOALBOX_DEPTH) {
                if (debugIdentifyCorners) {
                    cout << "Setting to center with length " <<
                        i->getTStem()->getLength() << endl;
                }
				i->setSecondaryShape(SIDE_T);
                if (face == FACING_UNKNOWN) {
                    if (debugIdentifyCorners) {
                        cout << "Updating facing to sideline" << endl;
                    }
                    face == FACING_SIDELINE;
                }
			}
		}
    }
}

/* Iterate through all of the corners to see if any of them are directly
   connected.  If so, find out what sort of information we can glean from
   that connection (e.g. what sort of T we're looking at.
 */
void Context::checkForConnectedCorners(list<VisualCorner> &corners) {
    int cornerNumber = 0;
    list <VisualCorner>::iterator i = corners.begin();
    bool foundConnection = false;
    for ( ; i != corners.end(); i++){
        cornerNumber++;
        // find out if the current corner is connected to any other
        list <VisualCorner>::iterator j = corners.begin();
        for (int k = 1; j != corners.end(); j++, k++) {
            if (k > cornerNumber) {
                if (i->getLine1() == j->getLine1() ||
                    i->getLine1() == j->getLine2() ||
                    i->getLine2() == j->getLine1() ||
                    i->getLine2() == j->getLine2()) {
                    findCornerRelationship(*i, *j);
                    foundConnection = true;
                }
            }
        }
    }
    // if we see multiple corners, but none are connected, we can
    // still infer stuff
    if (!foundConnection) {
        i = corners.begin();
        cornerNumber = 0;
        for ( ; i != corners.end(); i++){
            cornerNumber++;
            // find out if the current corner is connected to any other
            list <VisualCorner>::iterator j = corners.begin();
            for (int k = 1; j != corners.end(); j++, k++) {
                if (k > cornerNumber) {
                    findUnconnectedCornerRelationship(*i, *j);
                }
            }
        }
    }
}

/** If we have a single INNER_L corner we can often glean a lot of information
    about what it might be - especially if it is connected to a field object.
    Things we can check:
          Can we see the goal box?
          Can we see a goal post?
          How long are the lines in the corner?
 */
void Context::classifyInnerL(VisualCorner & first) {
    // punt (for now) when we can be sure about what we see
    if (!seeGoalBoxLines && face == FACING_UNKNOWN) {
        return;
    }
    float l1 = first.getLine1()->getLength();
    float l2 = first.getLine2()->getLength();
    int l1x1 = first.getLine1()->getLeftEndpoint().x;
    int l2x1 = first.getLine2()->getLeftEndpoint().x;
    int l1y1, l2y1;
    // there are two ways the two lines can be connected
    bool l1IsLeft = l1x1 < l2x1;
    // l1 and l2 hold information on points away from the corner
    if (l1IsLeft) {
        l2x1 = first.getLine2()->getRightEndpoint().x;
        l2y1 = first.getLine2()->getRightEndpoint().y;
        l1y1 = first.getLine1()->getLeftEndpoint().y;
    } else {
        l1x1 = first.getLine1()->getRightEndpoint().x;
        l1y1 = first.getLine1()->getRightEndpoint().y;
        l2y1 = first.getLine2()->getLeftEndpoint().y;
    }
    // determine which endpoint is higher up on screen
    bool l1IsHigher = l1y1 < l2y1;
    bool l1IsLonger = l1 > l2;
    if (debugIdentifyCorners) {
        if (l1IsLeft) {
            cout << "The left line has length " << l1 <<
                " the right " << l2 << endl;
            if (l1IsHigher) {
                cout << "The left line is higher up" << endl;
            } else {
                cout << "The right line is higher up" << endl;
            }
        } else {
            cout << "The left line has length " << l2 <<
                " the right " << l1 << endl;
            if (l1IsHigher) {
                cout << "The right line is higher up" << endl;
            } else {
                cout << "The left line is higher up" << endl;
            }
        }
    }
    // determine general facing
    if (face == FACING_UNKNOWN) {
        // we have one corner and no field objects
        first.setSecondaryShape(GOAL_L);
        // now see if we can figure out exactly which L
        if (l1IsLonger) {
            if (l1 > 2.5 * GOALBOX_DEPTH) {
                // we have enough information - is big line to left or right?
                if (l1IsLeft) {
                    first.setSecondaryShape(RIGHT_GOAL_L);
                } else {
                    first.setSecondaryShape(LEFT_GOAL_L);
                }
            }
        } else if (l2 > 2 * GOALBOX_DEPTH) {
            if (!l1IsLeft) {
                first.setSecondaryShape(RIGHT_GOAL_L);
            } else {
                first.setSecondaryShape(LEFT_GOAL_L);
            }
        }
    } else {
        shape leftColor = LEFT_GOAL_YELLOW_L;
        shape rightColor = RIGHT_GOAL_YELLOW_L;
        if (face == FACING_BLUE_GOAL) {
            leftColor = LEFT_GOAL_BLUE_L;
            rightColor = RIGHT_GOAL_BLUE_L;
        }
        // we see an inner l and a goal post
        // now see if we can figure out exactly which L
        // sometime to be super-safe we should check where the line intersects
        // the goal post
        // best done by using the post
        if (objectRightX > -1 && objectRightX < IMAGE_WIDTH / 2) {
            // post is on the left, normally this indicates a left goal inner L
            // the left line should basically disappear into the middle of the post
            if (l1IsLeft) {
                if (l1y1 < objectRightY) {
                    first.setSecondaryShape(leftColor);
                } else {
                    // we must have missed the other L corner for some reason
                    first.setSecondaryShape(RIGHT_GOAL_CORNER);
                }
            } else {
                if (l2y1 < objectRightY) {
                    first.setSecondaryShape(leftColor);
                } else {
                    first.setSecondaryShape(RIGHT_GOAL_CORNER);
                }
            }
        } else {
            // the post is on the right - normally a right corner
            if (l1IsLeft) {
                if (l2y1 < objectRightY) {
                    first.setSecondaryShape(rightColor);
                } else {
                    first.setSecondaryShape(LEFT_GOAL_CORNER);
                }
            } else {
                if (l1y1 < objectRightY) {
                    first.setSecondaryShape(rightColor);
                } else {
                    first.setSecondaryShape(LEFT_GOAL_CORNER);
                }
            }
        }
    }
}

/** We see two inner Ls.  This is quite easy to handle, figure out which
    one is inside, then look at whether it is right or left of the other
    one.
 */
void Context::unconnectedInnerLs(VisualCorner & inner, VisualCorner & outer) {
    // inner is a field corner, outer a goal corner
    // determine which ones
    if (inner.getX() < outer.getX()) {
        if (face == FACING_BLUE_GOAL) {
            inner.setSecondaryShape(RIGHT_GOAL_BLUE_L);
            outer.setSecondaryShape(BLUE_GOAL_BOTTOM);
        } else if (face == FACING_YELLOW_GOAL) {
            inner.setSecondaryShape(RIGHT_GOAL_YELLOW_L);
            outer.setSecondaryShape(YELLOW_GOAL_TOP);
        } else {
            inner.setSecondaryShape(RIGHT_GOAL_L);
            outer.setSecondaryShape(RIGHT_GOAL_CORNER);
        }
    } else {
        if (face == FACING_BLUE_GOAL) {
            inner.setSecondaryShape(LEFT_GOAL_BLUE_L);
            outer.setSecondaryShape(BLUE_GOAL_TOP);
        } else if (face == FACING_YELLOW_GOAL) {
            inner.setSecondaryShape(LEFT_GOAL_YELLOW_L);
            outer.setSecondaryShape(YELLOW_GOAL_BOTTOM);
        } else {
            inner.setSecondaryShape(LEFT_GOAL_L);
            outer.setSecondaryShape(LEFT_GOAL_CORNER);
        }
    }
}

/** Given two corners that we know are not connected, explore to find out the
    possible relationships.  This should help us reduce the number of
    possible corners each can be and ultimately help the identification
    process.
 */
void Context::findUnconnectedCornerRelationship(VisualCorner & first,
                                                VisualCorner & second) {
    VisualCorner* inner;
    VisualCorner* outer;
    cout << "Testing non connected" << endl;
    if (first.getShape() == INNER_L) {
        if (second.getShape() == OUTER_L) {
            inner = &first;
            outer = &second;
        } else if (second.getShape() == INNER_L) {
            // Should be an easy case - find the "inner" innerl
            if (first.getY() < second.getY()) {
                unconnectedInnerLs(second, first);
            } else {
                unconnectedInnerLs(first, second);
            }
            return;
        }
    } else if (second.getShape() == INNER_L) {
        if (first.getShape() == OUTER_L) {
            inner = &second;
            outer = &first;
        } else {
            return;
        }
    } else {
        return;
    }
    // if both of the endpoints of the outerl are on the same side of the corner
    // then this is easy
    int left1 = outer->getLine1()->getLeftEndpoint().x;
    int left2 = outer->getLine2()->getLeftEndpoint().x;
    int right1 = outer->getLine1()->getRightEndpoint().x;
    int right2 = outer->getLine2()->getRightEndpoint().x;
    int cx = outer->getLocation().x;
    if (left1 < cx && left2 < cx) {
        inner->setSecondaryShape(RIGHT_GOAL_CORNER);
        outer->setSecondaryShape(RIGHT_GOAL_L);
        return;
    } else if (right1 > cx && right2 > cx) {
        inner->setSecondaryShape(LEFT_GOAL_CORNER);
        outer->setSecondaryShape(LEFT_GOAL_L);
        return;
    }
    // when we get orientation information we'll be able to do better than this
    inner->setSecondaryShape(CORNER_L);
    outer->setSecondaryShape(GOAL_L);
}

/** Given two corners that we know are connected, explore to find out the
    possible relationships.  This should help us reduce the number of
    possible corners each can be and ultimately help the identification
    process.
 */
void Context::findCornerRelationship(VisualCorner & first, VisualCorner & second) {
    boost::shared_ptr<VisualLine> common;
    if (first.getLine1() == second.getLine1()) {
        common = first.getLine1();
    } else if (first.getLine1() == second.getLine2()) {
        common = first.getLine1();
    } else {
        common = first.getLine2();
    }
    VisualCorner* t;
    VisualCorner* l1;
    if (first.getShape() == T) {
        t = &first;
        if (second.getShape() == OUTER_L ||
            second.getShape() == INNER_L) {
            l1 = &second;
        } else {
            // T to CIRCLE or T to T
            if (debugIdentifyCorners) {
                cout << "T to something weird" << endl;
            }
            if (second.getShape() == T) {
                // T to T, one is almost certainly the center circle
                if (common == first.getTStem()) {
                    if (first.getTStem()->getLength() > GOALBOX_DEPTH * 2) {
                        first.setSecondaryShape(SIDE_T);
                        second.setShape(CIRCLE);
                        tCorner--;
                    }
                } else {
                    if (second.getTStem()->getLength() > GOALBOX_DEPTH * 2) {
                        second.setSecondaryShape(SIDE_T);
                        first.setShape(CIRCLE);
                        tCorner--;
                    }
                }
            }
			return;
        }
    } else if (second.getShape() == T) {
        t = &second;
        if (first.getShape() == OUTER_L ||
            first.getShape() == INNER_L) {
            l1 = &first;
        } else {
            if (debugIdentifyCorners) {
                cout << "T to something weird" << endl;
            }
            return;
        }
    } else {
        if (debugIdentifyCorners) {
            cout << "Two non T corners with common length " << common->getLength() << endl;
        }
        // it is likely that one of the corners is actually a T
        return;
    }
    // at this point we have a T and an L
    if (t->getTStem() == common) {
		t->setSecondaryShape(GOAL_T);
		l1->setSecondaryShape(GOAL_L);
        // looks very good - ultimately we should check line length too
        if (debugIdentifyCorners) {
            cout << "T connect to an L, should be goal box " <<
                t->getTStem()->getLength() << endl;
        }
        // can we determine which side?
        if (l1->getShape() == OUTER_L) {
            // look at the non-common line, figure out which direction it goes
            // if it goes in the same as left T, then it is a RIGHT L
            point<int> left;
            point<int> right;
            point<int> tpoint = t->getTStemEndpoint();
            if (l1->getLine1() == common) {
                left = l1->getLine2()->getLeftEndpoint();
                right = l1->getLine2()->getRightEndpoint();
            } else {
                left = l1->getLine1()->getLeftEndpoint();
                right = l1->getLine1()->getRightEndpoint();
            }
            // unfortunately the tendpoint doesn't always equal the corner
            int close = abs(left.x - tpoint.x) + abs(left.y - tpoint.y);
            int close2 = abs(right.x - tpoint.x) + abs(right.y - tpoint.y);
            if (close > close2) {
                // Line goes to right of T
                l1->setSecondaryShape(LEFT_GOAL_L);
                t->setSecondaryShape(LEFT_GOAL_T);
            } else {
                l1->setSecondaryShape(RIGHT_GOAL_L);
                t->setSecondaryShape(RIGHT_GOAL_T);
            }
        }
    } else {
        if (debugIdentifyCorners) {
            cout << "T connect to an L should be goal line to corner " <<
                t->getTBar()->getLength() << endl;
        }
		l1->setSecondaryShape(CORNER_L);
        if (lCorner > 1) {
            // we should be able to narrow the choices down to 2
            // compare the location of the L corner to the T corner
            if (l1->getX() < t->getX()) {
                l1->setSecondaryShape(RIGHT_GOAL_CORNER);
                t->setSecondaryShape(RIGHT_GOAL_T);
            } else {
                l1->setSecondaryShape(LEFT_GOAL_CORNER);
                t->setSecondaryShape(LEFT_GOAL_T);
            }
        } else {
            // check length  -- it its REALLY long to corner then we know
            // its a sideline T
            float dist = getEstimatedDistance(l1->getDistance(), l1->getBearing(),
                                              t->getDistance(), t->getBearing());
            cout << "Distance " << dist << endl;
            // figure out if lcorner is left or right of T
            bool left = false;
            if (t->getLocation().x < l1->getLocation().x) {
                left = true;
            }
            if (dist * 2 < FIELD_WHITE_WIDTH) {
                // seems to be a goalline - evaluate further
                if (left) {
                    t->setSecondaryShape(LEFT_GOAL_T);
                    l1->setSecondaryShape(LEFT_GOAL_CORNER);
                } else {
                    t->setSecondaryShape(RIGHT_GOAL_T);
                    l1->setSecondaryShape(RIGHT_GOAL_CORNER);
                }
            } else {
                face = FACING_SIDELINE;
                t->setSecondaryShape(SIDE_T);
                if (left) {
                    l1->setSecondaryShape(RIGHT_GOAL_CORNER);
                } else {
                    l1->setSecondaryShape(LEFT_GOAL_CORNER);
                }
            }
        }
    }
}

/**
 * Compare the given VisualCorner with other visible objects
 *  to see if the distances fit any sort of ConcreteCorners.
 *  Sets the possiblilities of the VisualCorner.
 *
 *  @param corner Corner to compare with VisualFieldObjects
 *  @param visibleObjects Visible objects to compare corner dists to.
 *  @return List of all ConcreteCorners whose distances fit the given objects.
 */
const list<const ConcreteCorner*> Context::classifyCornerWithObjects(
    const VisualCorner &corner,
    const vector <const VisualFieldObject*> &visibleObjects) const
{

	// Get all the possible corners given the shape of the corner
	vector <const ConcreteCorner*> possibleCorners =
		ConcreteCorner::getPossibleCorners(corner.getShape(), corner.getSecondaryShape());

    if (debugIdentifyCorners) {
        cout << endl
             << "Beginning to get possible classifications for corner given "
             << visibleObjects.size() << " visible objects and "
             << possibleCorners.size() << " corner possibilities" << endl
             << endl;
    }

    // sometimes we have already classified the corner, so just return it
    if (possibleCorners.size() == 1) {
        list<const ConcreteCorner*> classification;
        vector<const ConcreteCorner*>::const_iterator j = possibleCorners.begin();
        classification.push_back(*j);
        return classification;
    }

	// Get all the possible corners given the shape of the corner
	list<const ConcreteCorner*> possibleClassifications =
        compareObjsCorners(corner, possibleCorners, visibleObjects);

	// If we found nothing that time, try again with all the corners to
	// see if possibly the corner was misidentified (e.g. saw an L, but
	// actually a T)
	if (possibleClassifications.empty()){

		possibleCorners = ConcreteCorner::concreteCorners();
		possibleClassifications =
			compareObjsCorners(corner, possibleCorners, visibleObjects);
	}

    return possibleClassifications;
}

// Given a list of concrete corners that the visual corner could possibly be,
// weeds out the bad ones based on distances to visible objects and returns
// those that are still in the running.
list <const ConcreteCorner*> Context::compareObjsCorners(
	const VisualCorner& corner,
	const vector<const ConcreteCorner*>& possibleCorners,
	const vector<const VisualFieldObject*>& visibleObjects) const
{
	list<const ConcreteCorner*> possibleClassifications;

    // For each field object that we see, calculate its real distance to
    // each possible concrete corner and compare with the visual estimated
    // distance. If it fits, add it to the list of possibilities.
    vector<const ConcreteCorner*>::const_iterator j =
        possibleCorners.begin();

    // Note: changed the order of the loops 12/3/2010 so we can check
    // every object against the corner instead of just finding one good one
    for (; j != possibleCorners.end(); ++j) {
		bool isOk = true;
        for (vector <const VisualFieldObject*>::const_iterator k =
                 visibleObjects.begin(); k != visibleObjects.end() && isOk; ++k) {

            // outerls are never going to be corners of the field
            if (corner.getShape() == OUTER_L) {
                if ((*j)->getID() == BLUE_CORNER_TOP_L ||
                    (*j)->getID() == BLUE_CORNER_BOTTOM_L ||
                    (*j)->getID() == YELLOW_CORNER_TOP_L ||
                    (*j)->getID() == YELLOW_CORNER_BOTTOM_L) {
					isOk = false;
                    continue;
                }
            } else if (corner.getShape() == T) {
                const float estimatedDistance = getEstimatedDistance(&corner, *k);
                // if we have a T corner and a goal post, then we can determine which
                // one it is definitely - look at whether the Stem is going up or down
                // if it is down (normal case) just look at whether T is left or right
                // if it is up, then reverse the results because you are over the endline
                // Is stem pointing up or down?
                if ((*j)->getID() == CENTER_TOP_T ||
                    (*j)->getID() == CENTER_BOTTOM_T) {
                    // needs to be related to some field constant
                    if (estimatedDistance > 250.0) {
                        // we can tell which one by looking at the direction of the T
                        if (corner.doesTPointRight()) {
                            if ((*k)->getID() == BLUE_GOAL_LEFT_POST ||
                                (*k)->getID() == BLUE_GOAL_RIGHT_POST) {
                                if ((*j)->getID() == CENTER_BOTTOM_T) {
                                    continue;
                                } else {
                                    isOk = false;
                                    continue;
                                }
                            } else {
                                if ((*j)->getID() == CENTER_TOP_T) {
                                    continue;
                                } else {
                                    isOk = false;
                                    continue;
                                }
                            }
                        } else {
                            if ((*k)->getID() == BLUE_GOAL_LEFT_POST ||
                                (*k)->getID() == BLUE_GOAL_RIGHT_POST) {
                                if ((*j)->getID() == CENTER_TOP_T) {
                                    continue;
                                } else {
                                    isOk = false;
                                    continue;
                                }
                            } else {
                                if ((*j)->getID() == CENTER_BOTTOM_T) {
                                    continue;
                                } else {
                                    isOk = false;
                                    continue;
                                }
                            }
                        }
                    }
                }
                bool down = corner.doesTPointDown();
                bool right;
                if (corner.getX() > (*k)->getX()) {
                    right = true;
                    cout << "T is right " << estimatedDistance << endl;
                } else {
                    right = false;
                    cout << "T is left " << estimatedDistance << endl;
                }
                if (down) {
                    if (right) {
                        if ((*j)->getID() == BLUE_GOAL_RIGHT_T ||
                            (*j)->getID() == YELLOW_GOAL_RIGHT_T) {
							isOk = false;
                            continue;
                        }
                    } else {
                        if ((*j)->getID() == BLUE_GOAL_LEFT_T ||
                            (*j)->getID() == YELLOW_GOAL_LEFT_T) {
							isOk = false;
                            continue;
                        }
                    }
                } else {
                    if (!right) {
                        if ((*j)->getID() == BLUE_GOAL_RIGHT_T ||
                            (*j)->getID() == YELLOW_GOAL_RIGHT_T) {
							isOk = false;
                            continue;
                        }
                    } else {
                        if ((*j)->getID() == BLUE_GOAL_LEFT_T ||
                            (*j)->getID() == YELLOW_GOAL_LEFT_T) {
							isOk = false;
                            continue;
                        }
                    }
                }
                // need a constant related to field constants here
                if (estimatedDistance < 100.0f && getRealDistance(*j, *k, 0) < 100.0f) {
                    continue;
                }
            }

            const float estimatedDistance = getEstimatedDistance(&corner, *k);
            const float distanceToCorner = corner.getDistance();
			// The visual object might be abstract, so we should check
			// all of its possible objects to see if we're close enough to one
			// and add all the possibilities up.
			list<const ConcreteFieldObject*>::const_iterator i =
				(*k)->getPossibleFieldObjects()->begin();

			bool close = false;
			for (int p = 0; i != (*k)->getPossibleFieldObjects()->end() && !close; ++i, ++p) {

				if (arePointsCloseEnough(estimatedDistance, *j, *k,
										 distanceToCorner, p)) {
					close = true;
                }
            }
			// if the corner wasn't close enough to any possible object
			if (!close) {
				isOk = false;
			}
		}
		// if we made it through all possible field objects
		if (isOk) {
			possibleClassifications.push_back(*j);
			if (debugIdentifyCorners) {
				cout << "Corner is possibly a " << (*j)->toString() << endl;
			}
		}
	}
	return possibleClassifications;
}

/**
 * Compares the given estimated distance against the actual distance between
 * the given concrete corner and the field object's true field position. Returns
 * true if the estimated distance is acceptable.
 *
 * @param estimatedDistance Estimated distance between VisualCorner and VisualFieldObject
 * @param j Corner whose distance from the FieldObject is being measured
 * @param k FieldObject used for measuremnts
 * @param distToCorner Distance from robot to corner.
 */
const bool Context::arePointsCloseEnough(const float estimatedDistance,
											const ConcreteCorner* j,
											const VisualFieldObject* k,
											const float distToCorner, int n) const
{
	const float realDistance = getRealDistance(j, k, n);
	const float absoluteError = fabs(realDistance - estimatedDistance);

	const float relativeErrorReal = absoluteError / realDistance * 100.0f;

	// If we have already one good distance between this corner and a
	// field object, we only require that the next objects have a small
	// relative error.
	const float MAX_RELATIVE_ERROR = 70.0f;
	const float USE_RELATIVE_DISTANCE = 250.0f;
    const float MAX_ABSOLUTE_ERROR = 200.f;

    if ( relativeErrorReal < MAX_RELATIVE_ERROR &&
         k->getDistance() > USE_RELATIVE_DISTANCE &&
         distToCorner > USE_RELATIVE_DISTANCE &&
         absoluteError < MAX_ABSOLUTE_ERROR) {
		if (debugIdentifyCorners) {
			cout << "\tDistance between " << j->toString() << " and "
				 << k->toString() << " was fine! Relative error of "
                 << relativeErrorReal
				 << " and absolute error of "
				 << absoluteError
				 << " corner pos: (" << j->getFieldX() << ","
				 << j->getFieldY()
				 << " goal pos: (" << k->getFieldX() << ","
				 << k->getFieldY() << endl;
		}
		return true;
    } else if (absoluteError > getAllowedDistanceError(k) ||
        absoluteError > MAX_ABSOLUTE_ERROR) {
		if (debugIdentifyCorners) {
			cout << "\tDistance between " << j->toString() << " and "
				 << k->toString() << " too large." << endl
				 << "\tReal: " << realDistance
				 << "\tEstimated: " << estimatedDistance << endl
				 << "\tAbsolute error: " << absoluteError
				 << "\tRelative error: " << relativeErrorReal << "% , "
				 << relativeErrorReal << "%"
				 << endl;
		}
		return false;
    } else {
		if (debugIdentifyCorners) {
			cout << "\tDistance between " << j->toString() << " and "
				 << k->toString() << " was fine! Absolute error of "
				 << absoluteError
				 << " corner pos: (" << j->getFieldX() << ","
				 << j->getFieldY() << ")"
				 << " goal pos: (" << k->getFieldX() << ","
				 << k->getFieldY() << ")" << endl;
		}
		return true;
	}
}

// @TODO A real distance error calculation. For now, just uses 2 times
// the FieldObject's Distance SD
float Context::getAllowedDistanceError(const VisualFieldObject * obj) const
{
    return obj->getDistanceSD() * 2;
}

// Given two points on the screen, estimates the straight line distance
// between them, on the field
float Context::getEstimatedDistance(const point<int> &point1,
                                       const point<int> &point2) const
{

    const estimate point1Est = vision->pose->pixEstimate(point1.x, point1.y, 0);
    const estimate point2Est = vision->pose->pixEstimate(point2.x, point2.y, 0);

    const float point1Dist = point1Est.dist;
    const float point2Dist = point2Est.dist;
    // We cannot accurately calculate the distance because the pix estimate
    // is too bad
    if (point1Dist <= 0 || point2Dist <= 0) { return BAD_DISTANCE; }

    return Utility::getLength(point1Est.x, point1Est.y,
                              point2Est.x, point2Est.y);
}

// Uses vector subtraction in order to determine distance between corner and
// field object.
// If the field object is very close (<100 cm away) we can use pose to estimate
// bearing and distance to the field object rather than the distance and bearing
// provided by the VisualFieldObject obj itself.
float Context::getEstimatedDistance(const VisualCorner *c,
                                       const VisualFieldObject *obj) const
{
	const float objDist = obj->getDistance();
	const float objBearing = obj->getBearing();

    const float cornerDist = c->getDistance();
    // Convert degrees to radians for the sin/cos formulas
    const float cornerBearing = c->getBearing();

    const float dist = getEstimatedDistance(objDist, objBearing,
                                            cornerDist, cornerBearing);
    return dist;
}

float Context::getEstimatedDistance(const float dist1, const float bearing1,
                                 const float dist2, const float bearing2) const
{
    const float x1 = dist1 * sin(bearing1);
    const float y1 = dist1 * cos(bearing1);

    const float x2 = dist2 * sin(bearing2);
    const float y2 = dist2 * cos(bearing2);

    return Utility::getLength(x1, y1,
                              x2, y2);
}

float Context::getEstimatedDistance(const VisualCorner& corner,
                                       const point<int>& p) const
{
    const estimate pixEst = vision->pose->pixEstimate(p.x, p.y, 0);

    return getEstimatedDistance(pixEst.dist, pixEst.bearing,
                                corner.getDistance(), corner.getBearing());
}

// This method should only be called with a VisualFieldObject object that has a
// concrete location on the field (YGLP, YGRP, BGLP, BGRP, BY, and YB).
// Calculates the length of the straight line between the two objects on the
// field
float Context::getRealDistance(const ConcreteCorner *c,
                                  const VisualFieldObject *obj, int which) const
{
	if (which == 0) {
		return Utility::getLength(c->getFieldX(), c->getFieldY(),
								  obj->getFieldX(), obj->getFieldY());
	}
	return Utility::getLength(c->getFieldX(), c->getFieldY(),
								  obj->getFieldX2(), obj->getFieldY2());
}

void Context::printPossibilities(const list <const ConcreteCorner*> &_list) const
{
    cout << "Possibilities: " << endl;
    for (list<const ConcreteCorner*>::const_iterator i = _list.begin();
         i != _list.end(); ++i) {
        cout << (*i)->toString() << endl;
    }
}

// Creates a vector of all those field objects that are visible in the frame
// and have SURE certainty
vector <const VisualFieldObject*> Context::getVisibleFieldObjects()
{
    vector <const VisualFieldObject*> visibleObjects;
    for (int i = 0; i < NUM_FIELD_OBJECTS_WITH_DIST_INFO; ++i) {
        if (allFieldObjects[i]->getDistance() > 0 &&
            // We don't want to identify corners based on posts that aren't sure,
            // for instance
            allFieldObjects[i]->getIDCertainty() == _SURE) {
            // set field half information
            if (allFieldObjects[i]->getDistance() < MIDFIELD_X) {
                sameHalf = true;
            }
            objectRightX = allFieldObjects[i]->getRightBottomX();
            objectRightY = allFieldObjects[i]->getRightBottomY();
            // With the Nao we need to make sure that the goal posts are near the
            // green of the field in order to use them for distance
            if (ConcreteFieldObject::isGoal(allFieldObjects[i]->getID()) &&
                !(goalSuitableForPixEstimate(allFieldObjects[i]))) {
                // We can't use it.
            }
            else {
                visibleObjects.push_back(allFieldObjects[i]);
            }
        } else if (allFieldObjects[i]->getDistance() == 0 &&
                   allFieldObjects[i]->getIDCertainty() == _SURE) {
            // we see a post, but it is probably too close to get
            // a good distance (occluded on two sides)
            // we may not want to use the object too much, but it can help
            sameHalf = true;
            objectRightX = allFieldObjects[i]->getRightBottomX();
            objectRightY = allFieldObjects[i]->getRightBottomY();
        }
    }
    return visibleObjects;
}

vector<const VisualFieldObject*> Context::getAllVisibleFieldObjects() const
{
    vector <const VisualFieldObject*> visibleObjects;
    for (int i = 0; i < NUM_FIELD_OBJECTS_WITH_DIST_INFO; ++i) {
        if (allFieldObjects[i]->getDistance() > 0){
                visibleObjects.push_back(allFieldObjects[i]);
        }
    }
    return visibleObjects;
}

// In some Nao frames, robots obscure part of the goal and the bottom is not
// visible.  We can only use pix estimates of goals whose bottoms are visible
const bool Context::goalSuitableForPixEstimate(const VisualFieldObject * goal) const
{
    // When we're the goalie, we don't expect there to be any robots blocking
    // the goal.  Furthermore, we're seeing it from a much different angle where
    // the green will not be at the bottom of the post
    const int MAX_PIXEL_DIFF = 10;
    int greenHorizon = vision->thresh->getVisionHorizon();
    int midBottomY = (goal->getLeftBottomY() + goal->getRightBottomY())/2;
    // higher values are lower in the image
    return greenHorizon - midBottomY < MAX_PIXEL_DIFF;
}

/* Set facing information.  Initially this is based on whether we see a goal
   post or not.  If we do then we are facing one.  Eventually we may realize
   that we are facing a sideline (e.g. if we see a side T and no goal).
   Down the road we may want to make it more fine grained (e.g. facing a
   particular field corner).
 */
void Context::setFacing() {
    if (bluePost) {
        face = FACING_BLUE_GOAL;
    } else if (yellowPost) {
        face = FACING_YELLOW_GOAL;
    } else {
        face = FACING_UNKNOWN;
    }
}

/** Set the field half if possible by looking at the distance to any field
    objects.
 */
void Context::setFieldHalf() {
    if (face == FACING_UNKNOWN) {
        fieldHalf = HALF_UNKNOWN;
        return;
    }
    if ((face == FACING_BLUE_GOAL && sameHalf) ||
        (face == FACING_YELLOW_GOAL && !sameHalf)) {
        fieldHalf = HALF_BLUE;
    } else {
        fieldHalf = HALF_YELLOW;
    }
}

/**
   Print lots of useful debugging information.
*/
void Context::printContext() {
    cout << "Begining identify Corners." << endl;
    cout << "   Facing: ";
    switch (face) {
    case FACING_BLUE_GOAL:
        cout << "Blue goals " << endl;
        if (rightBluePost) {
            cout << "     We see a right blue post" << endl;
        }
        if (leftBluePost) {
            cout << "     We see a left blue post" << endl;
        }
        if (unknownBluePost) {
            cout << "     We see an unidentified blue post" << endl;
        }
        break;
    case FACING_YELLOW_GOAL:
        cout << "Yellow goals " << endl;
        if (rightYellowPost) {
            cout << "     We see a right yellow post" << endl;
        }
        if (leftYellowPost) {
            cout << "     We see a left yellow post" << endl;
        }
        if (unknownYellowPost) {
            cout << "     We see an unidentified yellow post" << endl;
        }
        break;
    case FACING_SIDELINE:
        cout << "Sideline " << endl;
        break;
    case FACING_UNKNOWN:
        cout << "Unknown " << endl;
        break;
    }
    cout << "   Field half is: ";
    switch (fieldHalf) {
    case HALF_UNKNOWN:
        cout << "unknown" << endl;
        break;
    case HALF_BLUE:
        cout << "blue side" << endl;
        break;
    case HALF_YELLOW:
        cout << "yellow side" << endl;
        break;
    default:
        break;
    }
    cout << "   We can see:" << endl;
    cout << "                " << tCorner << " T corners " << endl;
    cout << "                " << lCorner << " L corners " << endl;
    cout << "                " << cCorner << " CC corners" << endl;
    if (cCorner < 1 && seeCenterCircle) {
        cout << "    We see the center circle" << endl;
    }
    if (seeGoalBoxLines) {
        cout << "    We see goal box lines" << endl;
    }
}
