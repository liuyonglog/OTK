#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QScrollArea>
#include <QLabel>
#include <QKeyEvent>
#include <QSystemTrayIcon>
#include <QDir>

#include "imgWidget.h"

#include <opencv/cv.h>
#include <opencv/highgui.h>

//OpenCV headers
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

class QTimer;
using namespace cv;
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
	QImage MatToQImage(const Mat& mat);
	void OpenImage(QString fileName);
	void OpenImage(QPixmap pix);
    Mat image;
    QString filename;

    
private:
    Ui::MainWindow *ui;
    void setWebcam();
    QTimer *timer;
    IplImage* captureimage;
    //CvCapture* capture;

	void createActions();                   //��������
    void createMenus();                     //�����˵�
    void createToolBars();                  //����������
    void createStatusBar();                 //����״̬��
	void createTrayIcon();					//����������ͼ��
	IplImage colorBalance(IplImage* imageBalanced);			//ɫ��ƽ��
	IplImage skinColorLikelihood(IplImage* imageLikelihood);//��ɫ��Ȼ
	CvPoint gravity(IplImage* imageLikelihood);				//������
	
	QScrollArea *scrollArea;                //��������
    ImageWidget *imageWidget;               //ͼƬ��ʾ����

    QMenu *fileMenu;                        //�ļ������˵�
    QMenu *editMenu;                        //ͼƬ�༭�˵�
	QMenu *helpMenu;						//�����˵�
	QMenu *trayIconMenu;					//�������˵�

    QToolBar *fileToolBar;                  //�ļ�����������
    QToolBar *editToolBar;                  //ͼƬ�༭������

    QAction *dirAct;                        //Ŀ¼����
    QAction *nextAct;
    QAction *previousAct;
    QAction *leftAct;
    QAction *rightAct;
    QAction *zoomInAct;
    QAction *zoomOutAct;
    QAction *actualSizeAct;
    QAction *fitSizeAct;
    QAction *fullScreenAct;
    QAction *closeAct;
	QAction *aboutAct;

	QAction *openCameraAct;					//OPENCV:������ͷ
	QAction *imageControlByHandAct;			//�ֿ�ͼƬ�л�
	QAction *imageControlByHandAct2;			//�ֿ�ͼƬ��ת

	QLabel *statusLabel;					//״̬����ǩ
	QSystemTrayIcon *trayIcon;

    QStringList imageList;                  //����ͼƬ·��
    int index;                              //ͼƬ����
    QDir imageDir;                          //ͼƬ·��
private slots:
    void captureFrame();
	void on_OpenImageFile();
    void on_ConvertImage();
    void on_SaveImageFile();

	void selectDir();       //ѡ��Ŀ¼
    void next();            //��һ��
    void previous();        //��һ��
    void rotateLeft();      //������ת
    void rotateRight();     //������ת
    void zoomIn();          //�Ŵ�
    void zoomOut();         //��С
    void actualSize();      //ʵ�ʴ�С
    void fitSize();         //���ʴ�С
    void present();         //��ʾ
    void showAll();         //��ʾ������
    void myClose();         //�ر�
	void about();			//���ڱ����
	void closeActionUse();  //��ǩʹ�ùر�
	void openActionUse();   //��ǩʹ�ÿ���
	void openCamera();		//�������
	void controlImage();	//�ֿ�ͼƬ�л�
	void controlImage2();	//�ֿ�ͼƬ��ת

protected:
    void resizeEvent(QResizeEvent *);       //�ض����С�¼�
    void keyPressEvent(QKeyEvent *);        //�����¼�
};

#endif // MAINWINDOW_H
