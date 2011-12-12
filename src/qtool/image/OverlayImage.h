/**
 * @class OverlayImage
 * 
 * Draws an overlay of the location of the ball and corners over the image in Qtool.
 *
 */

#pragma once

#include "BMPImage.h"
#include <QImage>
#include "man/memory/MVision.h"

namespace qtool {
namespace image {


class OverlayImage : public BMPImage
{

 public:
    OverlayImage(man::memory::MVision::const_ptr visionData);
    virtual ~OverlayImage() {};

    void updateBitmap();

    unsigned getWidth() {return 640;}
    unsigned getHeight() {return 480;}

 protected:
    man::memory::MVision::const_ptr visionData;
};

}
}
