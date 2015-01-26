#ifndef FILESREADER_H
#define FILESREADER_H

#include <QtGui/QWidget>
#include <QPixmap>
#include <QDesktopWidget>
#include <QPainter>
#include <QPointF>

class FilesReader : public QWidget
{
    Q_OBJECT

public:
    FilesReader(QWidget *parent = 0);
    ~FilesReader();

    bool bFit;      // whether the image is adapt for window size or not
    qreal scale;    // the value of image scale

    void setPixmap(QString fileName);// set image
    void setPixmap(QPixmap pix);// set image
    QPixmap getPixmap();
    void setAngle(qreal rotateAngle);// set rotate angle

protected:
    void paintEvent(QPaintEvent *);// override this method

private:
    QPixmap pixmap;
    qreal angle;

};

#endif // IMAGEWIDGET_H
