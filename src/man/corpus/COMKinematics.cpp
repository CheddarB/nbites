#include "COMKinematics.h"

using namespace NBMath;
using namespace std;
using namespace boost::numeric;

#define DEBUG_COM
//#define DEBUG_COM_VERBOSE

const ufvector4
Kinematics::getCOMc(const vector<float> bodyAngles){

  //copy the body angles to an array
  float angles[NUM_JOINTS];
  for(unsigned int i =0; i< NUM_JOINTS; i++){
    angles[i] = bodyAngles[i];
  }

  ufvector4 partialComPos = calculateChestCOM();

  for(unsigned int i = 0; i < NUM_CHAINS; i++){
    partialComPos+= slowCalculateChainCom((ChainID)i, &angles[chain_first_joint[i]]);
  }

#ifdef DEBUG_COM
  cout << "Body Com " << partialComPos <<endl;
#endif

  return partialComPos/TOTAL_MASS;
}

const ufvector4 Kinematics::calculateChestCOM() {
	ufvector4 chestCom = CoordFrame4D::vector4D(CHEST_MASS_X,
												CHEST_MASS_Y,
												CHEST_MASS_Z)
		*(CHEST_MASS_g);
#ifdef DEBUG_COM_VERBOSE
	cout << "Chest COM " << chestCom << endl;
#endif

	return chestCom;
}

const ufvector4
Kinematics::slowCalculateChainCom(const ChainID id,
		      const float angles[]) {
  ufmatrix4 fullTransform = ublas::identity_matrix <float> (4);
  ufvector4 comPos = CoordFrame4D::vector4D(0,0,0,0);

  // See: NaoPose.cpp::calculateForwardTransform
  // we use the same method here, but we need to apply joint weights
  // as we traverse down each joint chain

  const ufvector4 origin = CoordFrame4D::vector4D(0,0,0);

  // Do base transforms
  const int numBaseTransforms = NUM_BASE_TRANSFORMS[id];
  for (int i = 0; i < numBaseTransforms; i++) {
    fullTransform = prod(fullTransform, BASE_TRANSFORMS[id][i]);
  }

  // Do mDH transforms
  const int numTransforms = NUM_JOINTS_CHAIN[id];
  for (int i = 0; i < numTransforms; i++) {
    // Right before we do a transformation, we are in the correct coordianate
    // frame and we need to store it, so we know where all the links of a
    // chain are. We only need to do this if the transformation gives us a new
    // link
    const float *currentmDHParameters = MDH_PARAMS[id];

    //length L - movement along the X(i-1) axis
    if (currentmDHParameters[i*4 + L] != 0) {
      const ufmatrix4 transX =
	CoordFrame4D::translation4D(currentmDHParameters[i*4 + L],0.0f,0.0f);
      fullTransform = prod(fullTransform, transX);
    }

    //twist ALPHA: - rotate about the X(i-1) axis
    if (currentmDHParameters[i*4 + ALPHA] != 0) {
      const ufmatrix4 rotX =
	CoordFrame4D::rotation4D(CoordFrame4D::X_AXIS,
			       currentmDHParameters[i*4 + ALPHA]);
      fullTransform = prod(fullTransform, rotX);
    }

    //theta - rotate about the Z(i) axis
	//may need to run transform even if THETA=0
	if (currentmDHParameters[i*4 + THETA] + angles[i] != 0) {
		const ufmatrix4 rotZ =
			CoordFrame4D::rotation4D(CoordFrame4D::Z_AXIS,
									 currentmDHParameters[i*4 + THETA] +
									 angles[i]);
		fullTransform = prod(fullTransform, rotZ);
	}

    //offset D movement along the Z(i) axis
    if (currentmDHParameters[i*4 + D] != 0) {
      const ufmatrix4 transZ =
	CoordFrame4D::translation4D(0.0f,0.0f,currentmDHParameters[i*4 + D]);
      fullTransform = prod(fullTransform, transZ);
    }

	// add the local mass to the calculated CoM based on where
	// we are on our way down the joint chain
    const float *curInertialPos = INERTIAL_POS[id];

    const ufmatrix4 massEndTrans =
        CoordFrame4D::translation4D(curInertialPos[i*4 + 0],
                                    curInertialPos[i*4 + 1],
                                    curInertialPos[i*4 + 2]);
    const ufmatrix4 curMassTrans = prod(fullTransform,massEndTrans);
    const float jointMass = curInertialPos[i*4 + MASS_INDEX];
    const ufvector4 thisSegmentWeightedPos =
        prod(curMassTrans,origin)*jointMass;

#ifdef DEBUG_COM_VERBOSE
	cout << "added joint: " << i << "--"<< thisSegmentWeightedPos << endl;

	const ufvector4 thisSegmentPosition =
		prod(curMassTrans,origin);
	cout << "position " << thisSegmentPosition << endl;
#endif
    comPos += thisSegmentWeightedPos;
  }
#ifdef DEBUG_COM_VERBOSE
  cout << "chain id: " << id <<endl;
  cout << "weight: " << comPos <<endl;
#endif
  return comPos;
}


