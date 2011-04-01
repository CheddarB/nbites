#ifndef _HoughLine_h_DEFINED
#define _HoughLine_h_DEFINED

#include <ostream>
#include <cmath>


/**
 * A line defined in polar coordinates, also storing information from the
 * Hough Space in which it was found.
 */
class HoughLine
{
public:
    HoughLine(int _rIndex, int _tIndex, int _score);

    HoughLine();
    virtual ~HoughLine() { };

    inline float getRadius() const { return r;      }
    inline float getAngle()  const { return t;      }

    inline int getRIndex()   const { return rIndex; }
    inline int getTIndex()   const { return tIndex; }
    inline int getScore()    const { return score;  }

    inline float getSinT()  const {
        if (!didSin){
            didSin = true;
            sinT = sinf(t);
        }
        return sinT;
    }

    inline float getCosT()   const {
        if (!didCos){
            didCos = true;
            cosT = cosf(t);
        }
        return cosT;
    }

    static bool intersect(int x0, int y0,
                          const HoughLine& a, const HoughLine& b);
    friend std::ostream& operator<< (std::ostream &o,
                                     const HoughLine &l){
        return o << "Line: rIndex: " << l.rIndex << " tIndex: " << l.tIndex <<
            "\n\tr: " << l.r << " t:" << l.t << " score: " << l.score;
    }

    bool operator==(const HoughLine &other);
    bool operator!=(const HoughLine &other);

private:
    int rIndex, tIndex;    // Radius, angle indices in HoughSpace table
    float r, t;            // Radius and angle of line in polar coords
    int score;             // Hough accumulator count

    mutable float sinT, cosT;   // These get computed on the fly, if needed
    mutable bool didSin, didCos;
};

#endif /* _HoughLine_h_DEFINED */
