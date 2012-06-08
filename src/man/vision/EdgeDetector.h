#ifndef EdgeDetector_h_DEFINED
#define EdgeDetector_h_DEFINED

#include "visionconfig.h"
#include "VisionDef.h"
#include "Gradient.h"
#include "Profiler.h"
#include <stdint.h>

#include "boost/shared_ptr.hpp"

/**
 * Used to find the step edges in a given channel of an image according to a
 * preset, fixed threshold.
 */
class EdgeDetector
{
public:
    EdgeDetector(uint8_t thresh = default_edge_value);
    virtual ~EdgeDetector(){ };

/**
 * Public interface
 */
public:
    void detectEdges(int upperBound,
                     int* field_edge,
                     const uint16_t* channel,
                     Gradient& gradient);

    void    setThreshold(uint8_t thresh) { threshold = thresh; }
    uint8_t getThreshold() { return threshold; }

private:
    void sobelOperator(int upperBound,
                       const uint16_t* channel,
                       Gradient& gradient);

    void findPeaks(int upperBound,
                   int* field_edge,
                   Gradient& gradient);

private:
    enum {
        default_edge_value = 75
    };

private:
    uint8_t threshold;
};

#endif /* EdgeDetector_h_DEFINED */
