#ifndef COLORCREATOR_H
#define COLORCREATOR_H

#include <QWidget>
#include "roboimage.h"
#include "renderarea.h"
#include "window.h"
#include "coloredit.h"
#include "colortable.h"
#define  NEWFRAMES
#ifdef   NEWFRAMES
#define  COLORS 8
#define  WIDTH 640
#define  HEIGHT 480
#define  EXTENSION ".frm"
#define  EXTENSIONLENGTH = 4
#else
#define  COLORS 8
#define  WIDTH 320
#define  HEIGHT 240
#define  EXTENSION ".NBFRM"
#define  EXTENSIONLENGTH = 6
#endif


namespace Ui {
    class ColorCreator;
}

class ColorCreator : public QWidget
{
    Q_OBJECT

public:
    enum Colors {Orange, Blue, Yellow, Green, White, Pink, Navy, Black};
    enum Choices {Single, Multiple, Table};
    explicit ColorCreator(QWidget *parent = 0);
    ~ColorCreator();
    void updateDisplays();
    void updateColors();
    void updateThresh();
    void initStats();
    void collectStats(int x, int y);
    void outputStats();
    float min(float a, float b) {if (a < b) return a; return b;}
    float max(float a, float b) {if (a > b) return a; return b;}
    int min(int a, int b) {if (a < b) return a; return b;}
    int max(int a, int b) {if (a > b) return a; return b;}
    void writeNewFormat(QString filename);
    void writeOldFormat(QString filename);

private slots:
    void on_pushButton_clicked();

    void on_previousButton_clicked();

    void on_nextButton_clicked();

    void on_hMin_valueChanged(int value);

    void on_hMax_valueChanged(int value);

    void on_sMin_valueChanged(int value);

    void on_sMax_valueChanged(int value);

    void on_yMin_valueChanged(int value);

    void on_yMax_valueChanged(int value);

    void on_zSlice_valueChanged(int value);

    void on_colorSelect_currentIndexChanged(int index);

    void on_viewChoice_currentIndexChanged(int index);

    void on_zMin_valueChanged(int value);

    void on_zMax_valueChanged(int value);

    void on_getColorTable_clicked();

    void on_writeNew_clicked();

private:
    Ui::ColorCreator *ui;
    RoboImage roboimage;
    ColorTable *table;
    RenderArea *renderArea;
    Window imageWindow;
    QString baseDirectory;
    QString currentDirectory;
    QString baseColorTable;
    QString currentColorDirectory;
    QString nextFrame;
    QString previousFrame;
    int currentFrameNumber;
    int mode;
    QColor *cols;
    float *hMin;
    float *hMax;
    float *sMin;
    float *sMax;
    float *zMin;
    float *zMax;
    float zSlice;
    int *yMin;
    int *yMax;
    float statsHMin, statsHMax, statsSMin, statsSMax, statsZMin, statsZMax;
    int statsYMin, statsYMax, statsUMin, statsUMax, statsVMin, statsVMax;
    int currentColor;
    ColorEdit *green;
    unsigned *bitColor;
    bool haveFile;
};

#endif // COLORCREATOR_H
