#include <QtGui/QApplication>
#include "mainwindow.h"
#include <QSplashScreen>
#include <QTextCodec>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

	//set codec
	//QTextCodec *codec = QTextCodec::codecForName("GB2312");
	QTextCodec *codec = QTextCodec::codecForName("GBK");
    QTextCodec::setCodecForLocale(codec);  
    QTextCodec::setCodecForCStrings(codec);  
    QTextCodec::setCodecForTr(codec);  

	// start to show splash
	QSplashScreen *splash = new QSplashScreen;
	splash->setPixmap(QPixmap("../opencv-qt-example/Resources/Splash.png"));
	splash->show();

    MainWindow w;
    w.show();

	splash->finish(&w);//when software begin£¬splash end
	delete splash;

    return a.exec();
}
