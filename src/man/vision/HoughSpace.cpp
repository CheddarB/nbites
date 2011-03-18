#include "HoughSpace.h"
#include <stdio.h>


using namespace std;
using boost::shared_ptr;

const int HoughSpace::drTab[peak_points] = {  1,  1,  0, -1 };
const int HoughSpace::dtTab[peak_points] = {  0,  1,  1,  1 };

extern "C" void _mark_edges(int numPeaks, int angleSpread,
                            int16_t *peaks, uint16_t *houghSpace);
extern "C" void _smooth_hough(uint16_t *hs);

HoughSpace::HoughSpace(shared_ptr<Profiler> p) :
    profiler(p),
    acceptThreshold(default_accept_thresh),
    angleSpread(default_angle_spread),
    numPeaks(0)
{

}

/**
 * The main public interface for the HoughSpace class.
 * Finds all the lines in the image using the Hough Transform.
 */
list<HoughLine> HoughSpace::findLines(shared_ptr<Gradient> g)
{
    PROF_ENTER(profiler, P_HOUGH);
    reset();

    markEdges(g);
    smooth();
    peaks();

    list<HoughLine> lines;
    createLinesFromPeaks(lines);

    int x0 = static_cast<int>(Gradient::cols/2);
    int y0 = static_cast<int>(Gradient::rows/2);

    suppress(x0, y0, lines);

    PROF_EXIT(profiler, P_HOUGH);
    return lines;
}

/**
 * Pass through the given Gradient and mark all potential edges
 * in the accumulator.
 */
void HoughSpace::markEdges(shared_ptr<Gradient> g)
{
    PROF_ENTER(profiler, P_MARK_EDGES);
#ifdef USE_MMX
    if (g->numPeaks > 0){
        _mark_edges(g->numPeaks, angleSpread, g->angles, &hs[0]);
    }
#else
    const int height = Gradient::rows;
    const int width  = Gradient::cols;
    const int x0     = width/2;
    const int y0     = height/2;

    // See comment in FindPeaks re: why this is shrunk in by 2
    // rows/columns on each side
    for (int y = 2; y < height-2; ++y){
        for (int x = 2; x < width-2; ++x){
            if (g->peaks[y][x]){
                int t = Gradient::dir(g->getY(y,x), g->getX(y,x));
                edge(x - x0, y - y0,
                     t - angleSpread,
                     t + angleSpread);
            }
        }
    }
#endif /* USE_MMX */
    PROF_EXIT(profiler, P_MARK_EDGES);
}

/**
 * Marks an edge point in the source gradient as an edge in the Hough
 * accumulator
 */
void HoughSpace::edge(int x, int y, int t0, int t1)
{
#ifndef USE_MMX
    int r0 = getR(x, y, t0);
    for (int t=t0; t <= t1; ++t){
        int t8 = t & 0xff;
        int r1 = getR(x, y, t8 + 1);

        for (int r = min(r0, r1); r <= max(r0, r1); ++r){
            int ri = r + r_span / 2;
            if (0 <=ri && ri <r_span){
                ++hs[ri][t8];
            }
        }
        r0 = r1;
    }
#endif
}

/**
 * Returns the radius of a line at the given location with the given
 * angle.
 */
int HoughSpace::getR(int x, int y, int t)
{
    float a = static_cast<float>(t & 0xff) * M_PI_FLOAT / 128.0f;
    return static_cast<int>(floor(static_cast<float>(x) * cos(a) +
                                  static_cast<float>(y) * sin(a)));
}

/**
 * Smooth out irregularities in the Hough accumulator to reduce noisy
 * peaks by using a 2x2 boxcar kernel.
 *
 * Boxcar kernel: (each element becomes the sum of four surrounding pixels)
 *       |1 1|
 *       |1 1|
 */
void HoughSpace::smooth()
{
    PROF_ENTER(profiler, P_SMOOTH);

#ifdef USE_MMX
    _smooth_hough(hs);
#else

    // Make a copy of the row t=0 at t=t_span. t=0 and t=t_span-1 are
    // neighbors in the cylindrical Hough Space, but t=0 gets written
    // over before T=t_span - 1 is smoothed so we copy it now.
    for (int r=0; r < r_span; ++r) {
        hs[r][t_span] = hs[r][0];
    }

    // In-place 2x2 boxcar smoothing
    for (int r=0; r < r_span-1; ++r) {
        for (int t=0; t < t_span; ++t) {
            hs[r][t] = hs[r][t] + hs[r + 1][t] +
                hs[r][t + 1] + hs[r + 1][t + 1];
        }
    }
#endif

    PROF_EXIT(profiler, P_SMOOTH);
}

/**
 * Find the peaks of the accumulator and create the list of lines in the space.
 */
void HoughSpace::peaks()
{
    PROF_ENTER(profiler, P_HOUGH_PEAKS);
    int thresh = 4 * acceptThreshold; // smoothing has a gain of 4

    numPeaks = 0;

    for (int r=0; r < r_span-1; ++r) {
        for (int t=0; t < t_span; ++t) {

            const int z = getHoughBin(r,t);
            if (z >= thresh){

                bool shouldCreate = true;

                for (int i=0; shouldCreate && i < peak_points; ++i) {

                    if ( ! ( z >  getHoughBin(r + drTab[i],
                                              (t + dtTab[i]) & 0xff) &&
                             z >= getHoughBin(r - drTab[i],
                                              (t - dtTab[i]) & 0xff))) {
                        shouldCreate = false;
                    }
                }

                if (shouldCreate){
                    addPeak(numPeaks, r, t, z);
                }
            }
        }
    }

    PROF_EXIT(profiler, P_HOUGH_PEAKS);
}

/**
 * Using the list of peaks found in the Hough Transform, create line objects
 *
 * @param lines List to be filled with lines
 */
void HoughSpace::createLinesFromPeaks(list<HoughLine>& lines)
{
    for (int i=0; i <= numPeaks; ++i){
        HoughLine line = createLine(getPeakR(i),
                                    getPeakT(i),
                                    getPeakZ(i) );
        lines.push_back(line);
    }
}
/**
 * Combine/remove duplicate lines and lines which are not right.
 */
void HoughSpace::suppress(int x0, int y0, list<HoughLine>& lines)
{
    PROF_ENTER(profiler, P_SUPPRESS);
    bool toDelete[lines.size()];
    for (unsigned int i = 0; i < lines.size(); ++i) {
        toDelete[i] = false;
    }
    list<HoughLine>::iterator line = lines.begin();
    list<HoughLine>::iterator line2;
    int index = 0;
    while (line != lines.end()){

        // Set the second point to point at the next line
        line2 = line;
        int index2 = index;
        line2++;
        index2++;

        while (line2 != lines.end()){

            const int tDiff = abs(((line->getTIndex() -
                                    line2->getTIndex()) & 0xff)
                                  << 24 >> 24);
            const int rDiff = abs(line->getRIndex() - line2->getRIndex());

            if ( 0 < tDiff && tDiff <= angleSpread &&
                 (rDiff <= 4 || HoughLine::intersect(x0, y0, *line, *line2))) {

                if (line->getScore() < line2->getScore()){
                    toDelete[index] = true;
                } else {
                    toDelete[index2] = true;
                }
            }
            index2++;
            line2++;
        }
        index++;
        line++;
    }

    line = lines.begin();
    int i = 0;
    while (line != lines.end()){
        if (toDelete[i]){
            line = lines.erase(line);
        } else {
            line++;
        }
        i++;
    }
    PROF_EXIT(profiler, P_SUPPRESS);
}

/**
 * Reset the accumulator and peak arrays to their initial values.
 */
void HoughSpace::reset()
{
#ifdef USE_MMX
    // @TODO: Rewrite with MMX intrinsics or inline ASM
    for (int i=0; i < r_span * (t_span+1); ++i) {
        hs[i] = 0;
    }
#else
    for (int r=0; r < r_span; ++r) {
        for (int t=0; t < t_span; ++t) {
            hs[r][t] = 0;
        }
    }
#endif
}

HoughLine HoughSpace::createLine(int r, int t, int z)
{
    return HoughLine(r, t,
                     static_cast<float>(r) -
                     r_span / 2.0f + 0.5f,
                     (static_cast<float>(t) + 1.0f) * // @TODO: WHY IS THIS +1.0f
                     M_PI_FLOAT / 128.0f, z >> 2);
}

// Check to see if the HoughSpace value at the given radius and theta
// is a peak.
bool HoughSpace::isPeak(int r, int t)
{
#ifdef USE_MMX
    return peak[t * r_span + r];
#else
    return peak[r][t];
#endif
}

int HoughSpace::getHoughBin(int r, int t)
{
#ifdef USE_MMX
    return static_cast<int>(hs[t * r_span + r]);
#else
    return hs[r][t];
#endif
}
