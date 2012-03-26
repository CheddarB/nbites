/**
 * Implements an interface between localization and the vision
 * system to apply visual landmark measurements to the 
 * particles. 
 *
 * @author Ellis Ratner <eratner@bowdoin.edu>
 */
#ifndef VISION_SYSTEM_H
#define VISION_SYSTEM_H

#include <string>
#include "ParticleFilter.h"
#include "ConcreteFieldObject.h"

/**
 * Holds a known landmark.
 */
struct Landmark
{
    Landmark(float X = 0.0f, float Y = 0.0f, std::string w = "UNKNOWN")
    : x(X), y(Y), what(w)
    { }

    /**
     * Constructs a Landmark from a ConcreteFieldObject.
     */
    Landmark(const ConcreteLandmark& fieldLandmark)
    {
        x    = fieldLandmark.getFieldX();
	y    = fieldLandmark.getFieldY();
        what = fieldLandmark.toString();
    }
    
    float x;
    float y;
    std::string what;

    friend std::ostream& operator<<(std::ostream& out, Landmark l)
    {
	out << "Landmark \"" << l.what << "\" at (" << l.x << ", "
	    << l.y << ") \n";
	return out;
    }
};

/**
 * Helper function to construct a list of Landmark possibilities from
 * a VisualFieldObject. 
 * @param fieldObjects
 * @return a vector of landmarks.
 */
template <typename VisualT, typename ConcreteT>
static std::vector<Landmark> constructLandmarks(VisualT fieldObject)
{
    std::vector<Landmark> landmarks;

    const std::list<const ConcreteT * > * possibilities = fieldObject.getPossibilities();

    typename std::list<const ConcreteT * >::const_iterator i;
    for(i = possibilities->begin(); i != possibilities->end(); ++i) 
    {
        // Construct landmarks from possibilities.
        Landmark l(**i);
	landmarks.push_back(l);
    }

    return landmarks;
}

/**
 * Holds a single observation.
 */
namespace PF
{
    struct Observation
    {
         Observation(std::vector<Landmark> p, float dist = 0.0f, float theta = 0.0f)
	 : possibilities(p), distance(dist), angle(theta)
	{ }
	
	bool isAmbiguous() const { return possibilities.size() > 1 ? true : false; }

	std::vector<Landmark> possibilities;
	float distance;
	float angle;
    };
}

typedef std::vector<Landmark> LandmarkMap;

/**
 * @class VisionSystem
 */
class VisionSystem : public PF::SensorModel
{
 public:
    VisionSystem(LandmarkMap m);
    ~VisionSystem() { }

    PF::ParticleSet update(PF::ParticleSet particles);

    bool hasNewObservations() const { return hasNewObs; }
    
    void feedObservations(std::vector<PF::Observation> newObs);

 private:
    LandmarkMap map;
    bool hasNewObs;
    std::vector<PF::Observation> currentObservations;
};

#endif // VISION_SYSTEM_H
