#include "HoughSpace.h"
#include <stdio.h>
#include <climits>

using namespace std;
using boost::shared_ptr;

const int HoughSpace::drTab[peak_points] = {  1,  1,  0, -1 };
const int HoughSpace::dtTab[peak_points] = {  0,  1,  1,  1 };

extern "C" void _mark_edges(int numPeaks, int angleSpread,
                            Gradient::AnglePeak *peaks, uint16_t *houghSpace);
extern "C" void _smooth_hough(uint16_t *hs, uint32_t threshold);

extern "C" void _houghMain(uint16_t* hs,
                           Gradient::AnglePeak* edges, int numEdges);

HoughSpace::HoughSpace(shared_ptr<Profiler> p) :
    profiler(p),
    acceptThreshold(default_accept_thresh),
    angleSpread(default_angle_spread),
    numPeaks(0), activeLines(active_line_buffer)
{

}

/**
 * The main public interface for the HoughSpace class.
 * Finds all the lines in the image using the Hough Transform.
 */
list<HoughLine> HoughSpace::findLines(Gradient& g)
{
    PROF_ENTER(profiler, P_HOUGH);
    reset();
    findHoughLines(g);
    list<HoughLine> lines = narrowHoughLines();

    PROF_EXIT(profiler, P_HOUGH);
    return lines;
}

/**
 * Locate the lines in the image using a Hough Transform
 */
void HoughSpace::findHoughLines(Gradient& g)
{
    markEdges(g);
    smooth();
    peaks();
    createLinesFromPeaks(activeLines);
}

/**
 * Process hough lines to eliminate duplicate lines
 * and pair up the lines.
 */
list<HoughLine> HoughSpace::narrowHoughLines()
{
    int x0 = static_cast<int>(Gradient::cols/2);
    int y0 = static_cast<int>(Gradient::rows/2);

    suppress(x0, y0, activeLines);
    list<pair<int, int> > pairs = pairLines(activeLines);

    list<HoughLine> lines;
    list<pair<int, int> >::iterator i;
    for (i = pairs.begin(); i != pairs.end(); ++i){
        lines.push_back(activeLines[(*i).first]);
        lines.push_back(activeLines[(*i).second]);
    }
    return lines;
}

/**
 * Pass through the given Gradient and mark all potential edges
 * in the accumulator.
 */
void HoughSpace::markEdges(Gradient& g)
{
    PROF_ENTER(profiler, P_MARK_EDGES);
#ifdef USE_MMX
    if (g.numPeaks > 0){
        // _mark_edges(g.numPeaks, angleSpread, g.angles, &hs[0][0]);

        _houghMain(&hs[0][0], g.angles, g.numPeaks);
    }
#else
    // See comment in FindPeaks re: why this is shrunk in by 2
    // rows/columns on each side
    for (int i = 0; g.isPeak(i); ++i) {
        edge(g.getAnglesXCoord(i),
             g.getAnglesYCoord(i),
             g.getAngle(i) - angleSpread,
             g.getAngle(i) + angleSpread);
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
                ++hs[t8][ri];
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
    _smooth_hough(&hs[0][0], acceptThreshold);
#else

    // In-place 2x2 boxcar smoothing
    for (int t=first_smoothing_row; t < t_span+first_smoothing_row; ++t) {
        for (int r=0; r < r_span-1; ++r) {

            hs[t][r] = static_cast<int16_t>(
                max((hs[t][r] + hs[t][r+1] + hs[t+1][r] + hs[t+1][r+1] -
                     getAcceptThreshold() * 4),
                    0)
                );
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

    for (uint16_t t=first_peak_row; t < t_span+first_peak_row; ++t) {

        // First and last columns are not accurate, so they shouldn't
        // be queried, so we skip to third row as first possible peak
        for (uint16_t r=2; r < r_span-2; ++r) {

            const uint16_t z = getHoughBin(r,t);
            if (z){
                for (int i=0; i < peak_points; ++i) {

                    if ( ! ( z >  getHoughBin(r + drTab[i],
                                              (t + dtTab[i])) &&
                             z >= getHoughBin(r - drTab[i],
                                              (t - dtTab[i])))) {
                        goto notALine;
                    }
                }
#ifdef USE_MMX
                addPeak(r, static_cast<uint16_t>(t - first_peak_row), z);
#else
                addPeak(r, t, z);
#endif
            }
        notALine:
            continue;
        }
    }

    PROF_EXIT(profiler, P_HOUGH_PEAKS);
}

/**
 * Using the list of peaks found in the Hough Transform, create line objects
 *
 * @param lines List to be filled with lines
 */
void HoughSpace::createLinesFromPeaks(ActiveArray<HoughLine>& lines)
{
    for (int i=0; i < numPeaks; ++i){
        HoughLine line = createLine(getPeakR(i),
                                    getPeakT(i),
                                    getPeakZ(i) );
        lines.add(line);
    }
}
/**
 * Combine/remove duplicate lines and lines which are not right.
 */
void HoughSpace::suppress(int x0, int y0, ActiveArray<HoughLine>& lines)
{
    PROF_ENTER(profiler, P_SUPPRESS);
    bool toDelete[lines.size()];

    for (int i = 0; i < lines.size(); ++i) {
        toDelete[i] = false;
    }

    int index = 0;
    while (index < lines.size()){

        int index2 = index+1;

        while (index2 < lines.size()){

            const int tDiff = abs(((lines[index].getTIndex() -
                                    lines[index2].getTIndex()) & 0xff)
                                  << 24 >> 24);

            const int rDiff = abs(lines[index].getRIndex() -
                                  lines[index2].getRIndex());

            // Since the lines are ordered by T value, if the tDiff is
            // too great, we should stop going any further with the
            // current line. This keeps us from looking at every pair
            // of lines, every time.
            if (tDiff > angleSpread){
                goto inner_suppress_loop;
            }

            if ( (rDiff <= suppress_r_bound ||
                  HoughLine::intersect(x0, y0, lines[index], lines[index2]))) {

                if (lines[index].getScore() < lines[index2].getScore()){
                    toDelete[index] = true;
                } else {
                    toDelete[index2] = true;
                }
            }
        inner_suppress_loop:
            index2++;
        }
        index++;
    }

    index = 0;
    while (index < lines.size()){
        if (toDelete[index]){
            lines.deactivate(index);
        }
        index++;
    }
    PROF_EXIT(profiler, P_SUPPRESS);
}

list<pair<int, int> > HoughSpace::pairLines(ActiveArray<HoughLine>& lines)
{
    list<pair<int, int> > pairs;
    const int size = lines.size();

    int pair_array[size];
    int min_pair_r[size];

    // Init arrays
    for(int i=0; i < size; ++i){
        pair_array[i] = -1;
        min_pair_r[i] = INT_MAX;
    }

    for(int i=0; i < size; ++i){
        if(!lines.active(i)){
            continue;
        }

        for(int j=i+1; j < size; ++j){

            // These inner loops look awkward, yes. I'm avoiding
            // computation of tDiff and rSum by exiting early.
            if (!lines.active(j)){
                continue;
            }

            const int tDiff = abs(abs(lines[i].getTIndex() -
                                      lines[j].getTIndex())-t_span/2);

            if (tDiff >= opp_line_thresh){
                continue;
            }

            const int rSum = abs(lines[i].getRIndex() +
                                 lines[j].getRIndex() - r_span);
            if (rSum < min_pair_r[i] &&
                rSum < min_pair_r[j]){

                pair_array[i] = j;
                pair_array[j] = i;

                min_pair_r[i] = min_pair_r[j] = rSum;
            }
        }
    }

    for(int i=0; i < size; ++i){
        // If this line hasn't been paired up with a line after it,
        // this keeps us from duplicating line pairs.
        if (pair_array[i] < i){
            continue;
        }

        pairs.push_back(pair<int,int>(i, pair_array[i]));
    }
    return pairs;
}

/**
 * Reset the accumulator and peak arrays to their initial values.
 */
void HoughSpace::reset()
{
    numPeaks = 0;
    activeLines.clear();

#ifdef USE_MMX
    // Array resetting done in edge marking
#else
    for (int t=0; t < t_span; ++t) {
        for (int r=0; r < r_span; ++r) {
            hs[t][r] = 0;
        }
    }
#endif
}

HoughLine HoughSpace::createLine(int r, int t, int z)
{
    return HoughLine(r, t,
                     static_cast<float>(r) -
                     r_span / 2.0f + 0.5f,
                     (static_cast<float>(t+0.5f)) *
                     M_PI_FLOAT / 128.0f, z >> 2);
}

uint16_t HoughSpace::getHoughBin(int r, int t)
{
    return hs[t][r];
}

void HoughSpace::addPeak(uint16_t r, uint16_t t, uint16_t z)
{
    HoughPeak& p = peak[numPeaks];
    numPeaks++;

    p.r = r;
    p.t = t;
    p.z = z;
}
