
#include "HoughSpaceTest.h"
#include <list>
#include <string.h>

#include "Profiler.h"


using namespace std;
using boost::shared_ptr;

 // approx. 3 degrees in radians
const float HoughSpaceTest::ACCEPT_ANGLE = 0.05f;

HoughSpaceTest::HoughSpaceTest() :
    hs(shared_ptr<Profiler>(new Profiler(&micro_time)))
{

}

void HoughSpaceTest::test_hs()
{
    // Create gradient map such that it has a known line
    shared_ptr<Gradient> g = shared_ptr<Gradient>(new Gradient());
    createLineAtPoint(*g, 0, 80);

    // Run the gradient through the Hough Space
    hs.markEdges(g);

    for (int t=0; t < HoughSpace::t_span; ++t){
        for (int r=0; r < HoughSpace::r_span; ++r){
            GTE(hs.getHoughBin(r,t) , 0);
        }
    }
    PASSED(MORE_THAN_ZERO);

    // Check to make sure there is a point at r = 80 = img_width/4 , theta = 0
    // which is where the above gradient has a line, roughly.
    GT(hs.getHoughBin(IMAGE_WIDTH * 1/4 + HoughSpace::r_span/2,0) , 0);
    PASSED(EDGE_AT_BOUND);

    // Notice that it is t_span +1. This is the same as in the
    // Hough Space.
    uint16_t pre[HoughSpace::t_span+10][HoughSpace::r_span];

    for (int t=0; t < HoughSpace::t_span+10; ++t){
        for(int r=0; r < HoughSpace::r_span; ++r){
            pre[t][r] = hs.hs[t][r];
        }
    }

    hs.smooth();

    // Test if smoothing worked
    for (int t=HoughSpace::first_smoothing_row;
         t < HoughSpace::t_span + HoughSpace::first_smoothing_row; ++t){
        for (int r=0; r < HoughSpace::r_span-1; ++r){

            int preSum = (pre[t][r]   + pre[t+1][r] +
                          pre[t][r+1] + pre[t+1][r+1]) -
                hs.getAcceptThreshold()*4; // Smoothing grows mag by 4x

            preSum = max(preSum, 0); // Bound it at zero

            int smoothed = hs.getHoughBin(r,t);
            EQ_INT( smoothed, preSum);
        }
    }
    PASSED(SMOOTH_CORRECT);
}

/**
 * Test for known lines in an image
 */
void HoughSpaceTest::test_lines()
{
    for(int t=0; t < 255; t += 5){
        for (float r=0; r < 120; r += 10){
            test_for_line(static_cast<uint8_t>(t), r);
        }
    }
    PASSED(FOUND_GOOD_LINES);
}

void HoughSpaceTest::test_for_line(uint8_t angle, float radius)
{
    float radAngle = static_cast<float>(angle) * M_PI_FLOAT / 128.f;

    shared_ptr<Gradient> g = shared_ptr<Gradient>(new Gradient());
    g->reset();
    createLineAtPoint(*g, angle, radius);

    list<HoughLine> lines = hs.findLines(g);

    // We only want one line to be found in this fake image since we
    // only created one
    EQ_INT(lines.size(), 1);

    float maxRadius = sqrtf(IMAGE_WIDTH * IMAGE_WIDTH +
                           IMAGE_HEIGHT * IMAGE_HEIGHT);

    bool foundFixedLine = false;
    list<HoughLine>::iterator l = lines.begin();
    while (l != lines.end()){
        LTE(l->getRadius() , maxRadius); // Line must be in image
        GTE(l->getRadius(), -maxRadius); // in either direction
        GTE(l->getAngle() , 0);          // 0 <= Angle <= 2 * pi
        LTE(l->getAngle() , 2 * M_PI_FLOAT + ACCEPT_ANGLE);
        GTE(l->getScore() , 0);

        // Make sure the system found the one line in the gradient
        if (isDesiredLine(radius, radAngle, *l)){
            foundFixedLine = true;
        }

        l++;
    }
    // We better have found that line
    TRUE(foundFixedLine);
}

void HoughSpaceTest::test_suppress()
{
    list<HoughLine> lines;

    // test three identical lines to make sure that only one of the
    // duplicate lines survives suppress()
    HoughLine a1 = HoughSpace::createLine(100, 180, 50);
    HoughLine a2 = HoughSpace::createLine(101, 182, 2);
    HoughLine a3 = HoughSpace::createLine(99, 182, 4);

    lines.push_back(a1);
    lines.push_back(a2);
    lines.push_back(a3);

    int x0 = IMAGE_HEIGHT /2;
    int y0 = IMAGE_WIDTH /2;
    hs.suppress(x0, y0, lines);

    EQ_INT(lines.size() , 1);   // Ensure that only one duplicate line remains
    TRUE(lines.front() == a1);

    PASSED(NO_DUPE_LINES);

    // Make sure that suppress doesn't delete lines needlessly
    HoughLine a = HoughSpace::createLine(100, 180, 50);
    HoughLine b = HoughSpace::createLine(10,200,400);
    HoughLine c = HoughSpace::createLine(100,10,4);
    HoughLine b2 = HoughSpace::createLine(10,203,4);

    lines.clear();
    lines.push_back(a);
    lines.push_back(b);
    lines.push_back(c);
    lines.push_back(b2);

    hs.suppress(x0, y0, lines);
    EQ_INT( lines.size() , 3 );
    bool at = false, bt = false, ct = false;

    list<HoughLine>::iterator i = lines.begin();
    while (i != lines.end()){
        if (*i == a)
            at = true;
        if (*i == b)
            bt = true;
        if (*i == c)
            ct = true;
        i++;
    }

    TRUE(at && bt && ct);
    PASSED(DONT_DELETE_GOOD_LINES);

}

bool HoughSpaceTest::isDesiredLine(float goalR, float goalT,
                                   const HoughLine& line)
{
    float lineR = line.getRadius();
    float lineT = line.getAngle();

    LTE(lineT, 2*M_PI_FLOAT);
    GTE(lineT, 0);

    float goalLowerT = goalT - ACCEPT_ANGLE;
    float goalUpperT = goalT + ACCEPT_ANGLE;

    float tDiff = fabs(lineT - goalT);
    float rDiff = fabs(lineR - goalR);

    return (
        // Correct radius
        (rDiff < ACCEPT_RADIUS) &&

        // Correct angle
        // Greater than lower bound
        (tDiff < ACCEPT_ANGLE ||
         fabs(2*M_PI_FLOAT - tDiff) < ACCEPT_ANGLE));

}

void HoughSpaceTest::createLineAtPoint(Gradient& g, uint8_t angle, float radius)
{
    float radAngle = static_cast<float>(angle) * M_PI_FLOAT/128.f;


    double sn = sin(radAngle);
    double cs = cos(radAngle);

    double x0 = radius * cs;
    double y0 = radius * sn;

    for (double u = -200.; u <= 200.; u+=1.){
        int x = (int)round(x0 + u * sn) + IMAGE_WIDTH  / 2;
        int y = (int)round(y0 - u * cs) + IMAGE_HEIGHT / 2;

        if (0 <= x && x < IMAGE_WIDTH &&
            0 <= y && y < IMAGE_HEIGHT){
            g.addAngle(angle,
                       static_cast<uint16_t>(x),
                       static_cast<uint16_t>(y));
        }
    }
}

int HoughSpaceTest::runTests()
{
    test_hs();
    test_lines();
    test_suppress();
    return 0;
}


int main(int argc, char * argv[])
{
    HoughSpaceTest * tests = new HoughSpaceTest();
    return tests->runTests();
}
