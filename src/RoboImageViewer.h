#ifndef RENDERAREA_H
#define RENDERAREA_H

#include <QWidget>
#include <QBrush>
#include <QPen>
#include <QPixmap>
#include <QLabel>
#include "YUVImage.h"

class RoboImageViewer: public QWidget
{
    Q_OBJECT
public:
    enum Shape { Y, U, V, Blue, Red, Green, H, S, Z, EDGE, WHEEL, Pixmap };

    RoboImageViewer(YUVImage r1, QLabel *lab, QWidget *parent = 0);

    void  setYUVImage(YUVImage img) { r = img;}
    void mouseMoveEvent(QMouseEvent *event);
    int max(int a, int b) {if (a < b) return b; return a;}
    QSize minimumSizeHint() const;
    QSize sizeHint() const;

public slots:
    void setShape(Shape shape);
    void setPen(const QPen &pen);
    void setBrush(const QBrush &brush);
    void setAntialiased(bool antialiased);
    void setTransformed(bool transformed);

protected:
    void paintEvent(QPaintEvent *event);

private:
    Shape shape;
    QPen pen;
    QBrush brush;
    QLabel *info;
    bool antialiased;
    bool transformed;
    QPixmap pixmap;
    YUVImage r;

signals:


};

#endif // RENDERAREA_H
