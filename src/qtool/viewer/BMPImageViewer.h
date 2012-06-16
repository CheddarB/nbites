/**
 * Renders a BMPImage by transforming it to a QPixmap and attaching
 * it to a QLabel
 *
 * @author Octavian Neamtu
 */

#pragma once

#include <QWidget>
#include <QLabel>
#include <QLayout>
#include <QResizeEvent>

#include "data/Typedefs.h"
#include "image/FastYUVToBMPImage.h"

namespace qtool {
namespace viewer {

class BMPImageViewer: public QWidget {
    Q_OBJECT
public:

    BMPImageViewer(image::BMPImage* image,
				   QWidget *parent = NULL);

    virtual ~BMPImageViewer();

	virtual unsigned getWidth() { return scaledPixmap.width();}
	virtual unsigned getHeight() { return scaledPixmap.height();}
	virtual QSize size(){return QSize(scaledPixmap.width(), scaledPixmap.height());}
	virtual void resize(QSize newSize){
		scaledPixmap = scaledPixmap.scaled(newSize);
	}
	float scale;

	QVBoxLayout* getLayout();

public slots:
    void updateView(bool resize = 1);

protected:
    void showEvent(QShowEvent* event);
    void paintEvent(QPaintEvent* event);
    void setupUI();
    void addPixmap();

protected:
    image::BMPImage* image;
    QLabel imagePlaceholder;
	QVBoxLayout* BMPlayout;
	QSize imageSize;
	QPixmap scaledPixmap;
	int initialW;
	int initialH;

private:
	bool shouldRedraw;

};

}
}
