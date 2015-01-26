#include "filesReader.h"

FilesReader::FilesReader(QWidget *parent)
    : QWidget(parent)
{
    QDesktopWidget desktop;
    pixmap = QPixmap(desktop.width(),desktop.height());
    scale = 1;
    angle = 0;
    bFit = true;
}

FilesReader::~FilesReader()
{

}

void FilesReader::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    if(angle)
    {
        QPointF center(width()/2.0,height()/2.0);
        painter.translate(center);
        painter.rotate(angle);
        painter.translate(-center);
    }

    if(bFit)
    {
        QPixmap fitPixmap = pixmap.scaled(width(),height(),Qt::IgnoreAspectRatio);
        painter.drawPixmap(0,0,fitPixmap);
    }
    else
        painter.drawPixmap(0,0,pixmap);
}

void FilesReader::setPixmap(QString fileName)
{
    pixmap.load(fileName);
    update();
}

void FilesReader::setPixmap(QPixmap pix)
{
	if(!pix.isNull())
	{
		pixmap=pix;
		update();
	}
}

QPixmap FilesReader::getPixmap()
{
    return pixmap;
}

void FilesReader::setAngle(qreal rotateAngle)
{
    angle += rotateAngle;
    update();
}
