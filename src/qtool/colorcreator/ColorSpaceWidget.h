/**
 * @class ColorSpaceWidget
 *
 * Defines a GUI to control color space parameters
 *
 * @author Octavian Neamtu
 */
#pragma once

#include <QWidget>
#include <QMap>
#include <QSlider>
#include "image/Color.h"
#include "ColorSpace.h"

namespace qtool {
namespace colorcreator {

class ColorSpaceWidget: public QWidget {

    Q_OBJECT;

public:
    static const float FLOAT_SLIDER_SCALE = 256.0f;

public:
    ColorSpaceWidget(ColorSpace* colorSpace = ColorSpace::NullInstance(),
                     QWidget* parent = NULL);
    ~ColorSpaceWidget() {}

public slots:
    void sliderValueChanged(int value);
    void setColorSpace(ColorSpace* _colorSpace);

private:
    QMap<QSlider*, ColorSpace::Channel> sliders;
    ColorSpace* colorSpace;
};

}
}

