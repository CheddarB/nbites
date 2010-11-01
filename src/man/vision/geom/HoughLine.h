#ifndef _HoughLine_h_DEFINED
#define _HoughLine_h_DEFINED


/**
 * A line defined in polar coordinates, also storing information from the
 * Hough Space in which it was found.
 */
class HoughLine
{
public: 
   HoughLine(int _rIndex, int _tIndex,
              float _r, float _t, int _score);
    virtual ~HoughLine() { };

    float getRadius() { return r;      }
    float getAngle()  { return t;      }

    int getRIndex()   { return rIndex; }
    int getTIndex()   { return tIndex; }
    int getScore()    { return score;  }

private:
    float r, t;                 // Radius and angle of line in polar coords
    int rIndex,                 // Radius index in HoughSpace
        tIndex,                 // Theta index in HoughSpace
        score;                  // Hough accumulator count
};

#endif /* _HoughLine_h_DEFINED */
