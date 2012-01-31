#ifndef COLORSPACE_H
#define COLORSPACE_H

#include <cmath>
#include "Common.h"

namespace qtool {
namespace image {

class Color
{
    // holds an rgb value in the form #XXRRGGBB
    typedef unsigned int RGB;

public:
    Color();

    static RGB makeRGB(byte r, byte g, byte b, byte a = 0xFF) {
        RGB result;
        ((byte*) &result)[3] = a;
        ((byte*) &result)[2] = r;
        ((byte*) &result)[1] = g;
        ((byte*) &result)[0] = b;
        return result;
    }

    static RGB makeRGBFromSingleByte(byte single_byte) {
        return makeRGB(single_byte, single_byte, single_byte);
    }

    float getRed() {return red;}
    float getGreen() {return grn;}
    float getBlue() {return blue;}
    float getY() {return 0.299f * red + 0.587f * grn + 0.114f * blue;}
    float getU() {return -0.14713 * red - 0.28886f * grn + 0.436f * blue;}
    float getV() {return 0.615f * red - 0.51499f * grn - 0.10001f * blue;}
    float getH();
    float getS() { return getC() / getZ();}
    float getZ() { return max(max(red, grn), blue); }
    float getC() { return getZ() - min(min(red, grn), blue);}
    bool valid() { return 0 <= red && red < 1 && 0 <= grn && grn < 1 && 0 <= blue && blue < 1;}

    void setRgb(float r, float g, float b) { red = r; grn = g; blue = b;}
    void setRgb(byte r, byte g, byte b) {
        setRgb((float)r / 256.0f, (float)g / 256.0f, (float)b / 256.0f);}

    void setYuv(float y, float u, float v) {
        red = y + 1.13983f * v;
        grn = y - 0.39465f * u - 0.58060f * v;
        blue = y + 2.03211f * u;
    }
    void setYuv(byte y, byte u, byte v) {
        setYuv((float)y / 256.0f, (float)u / 256.0f - 0.5f, (float)v / 256.0f - 0.5f);
    }

    void setHsz(float h, float s, float z);

    // guards against negative values and values that might overflow in a byte
    static byte safeFloatToByte(float x) {
        return (byte) min(max(x, 0.0f), 255.0f);
    }

    byte cvb(float x) {return (byte)(min(max(256 * x, 0.0f), 255.0f));}
    byte getRb() { return cvb(red);}
    byte getGb() { return cvb(grn);}
    byte getBb() { return cvb(blue);}
    byte getYb() { return cvb(getY());}
    byte getUb() { return cvb(getU() + 0.5f);}
    byte getVb() { return cvb(getV() + 0.5f);}
    byte getHb() { return cvb(getH());}
    byte getSb() { return cvb(getS());}
    byte getZb() { return cvb(getZ());}
    RGB getRGB() { return makeRGB(getRb(), getGb(), getBb());}

    static float min(float a, float b) {if (a < b) return a; return b;}
    static float max(float a, float b) {if (a > b) return a; return b;}

private:

    float red, grn, blue;
};

}
}

#endif // COLORSPACE_H
