#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTimer>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QDate>
#include <sys/stat.h>
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->labelWebcam->setText("");
    setWebcam();

	imageWidget = new ImageWidget;
    scrollArea = new QScrollArea;
    scrollArea->setBackgroundRole(QPalette::Dark);
    imageWidget->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Ignored);
    scrollArea->setWidget(imageWidget);
    scrollArea->widget()->setMinimumSize(320,240);
	scrollArea->setAlignment(Qt::AlignCenter);// set place of center area
    setCentralWidget(scrollArea);

	// window is aligned center in window
	QDesktopWidget* desktop = QApplication::desktop(); 
	move((desktop->width()-600)/2,(desktop->height()-500)/2);

    index = 0;
	setWindowTitle(tr("OTK Application"));// window's name
    resize(600, 500);// window's size
	QIcon icon=QIcon("../OTK/Resources/pic_icon.png");
	setWindowIcon(icon);

	trayIcon=new QSystemTrayIcon(this);
	trayIcon->setIcon(icon);
	trayIcon->setToolTip("OTK Application\n"
		"author: LY");
	

	createActions();
    createMenus();
    createToolBars();
	createStatusBar();
	createTrayIcon();
	closeActionUse();
	trayIcon->show();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setWebcam()
{
    //captureimage = 0;
    //capture = cvCaptureFromCAM(0);
    //timer = new QTimer();
    //connect(timer, SIGNAL(timeout()), this, SLOT(captureFrame()));
    connect(ui->actionOpen, SIGNAL(triggered()), this, SLOT(on_OpenImageFile()));
    connect(ui->actionSave, SIGNAL(triggered()), this, SLOT(on_SaveImageFile()));
    connect(ui->actionConvertImage, SIGNAL(triggered()), this, SLOT(on_ConvertImage()));
    //timer->start(15);
}

void MainWindow::captureFrame()
{
    //cvGrabFrame(capture);
    //image = cvRetrieveFrame(capture);

    //QImage temp((const uchar *)captureimage->imageData, captureimage->width, captureimage->height,
    //                captureimage->widthStep, QImage::Format_RGB888);
    //temp = temp.scaled(ui->labelWebcam->width(), ui->labelWebcam->height());
    //ui->labelWebcam->setPixmap(QPixmap::fromImage(temp.rgbSwapped()));
}

void MainWindow::on_OpenImageFile()
{
	QString appFilter;
	QString defaultPath = ".";
	int m_nImageWidth=256;
	__int64 m_nFileSizeWanted;
	int nPercent=100;

	struct _stat buf;
	unsigned long* m_pDataRaw=NULL;
	m_pDataRaw = new unsigned long[m_nImageWidth*m_nImageWidth];
	memset(m_pDataRaw, 0, sizeof(unsigned long)*m_nImageWidth*m_nImageWidth);


	appFilter += tr("RAW Files (*.raw *.RAW);;");
	appFilter += tr("Image Files (*.jpg *.png *.bmp *.jpeg *.tiff *.tif *.dib *.jp2 *.jpe *.ppm *.pgm *.pbm *.ras *.sr);;");

    filename = QFileDialog::getOpenFileName(this,
                                          tr("Open Image File"),
                                          defaultPath, 
										  appFilter);
    if (!filename.isEmpty()){

		if(filename.contains(".raw"))
		{
			loadRAWFile(filename);
		}
		else
		{
			// now load the image
			image = imread(filename.toLocal8Bit().data()/*.toStdString()*/, CV_LOAD_IMAGE_COLOR);   // Read the file

			if(! image.data )                              // Check for invalid input
			{
				QMessageBox msgBox;
				msgBox.setText("The selected image could not be opened!");
				msgBox.show();
				msgBox.exec();
			}
			else // If image file is fine. Show it in the label pixmap
			{
				QImage qigm=MatToQImage(image);
				OpenImage(QPixmap::fromImage(qigm));
				//ui->labelWebcam->setPixmap(QPixmap::fromImage(qigm).scaledToWidth(ui->labelWebcam->size().width(),Qt::FastTransformation));
			}
		}


    }
}

bool MainWindow::loadRAWFile(QString fileName){

	bool imgLoaded = true;

	void *pData;
	FILE * pFile;
	unsigned char test[3];
	unsigned char *buf;
	int rohW = 7616;
	int rohH = 5888;
	pData = malloc(sizeof(unsigned char) * (rohW*rohH*3));
	buf = (unsigned char *)malloc(sizeof(unsigned char) * (rohW*rohH));

	try{
		pFile = fopen (fileName.toStdString().c_str(), "rb" );

		fread(pData, 3, rohW*rohH, pFile);

		fclose(pFile);

		for (long i=0; i < (rohW*rohH); i++){
		
			test[0] = ((unsigned char*)pData)[i*3];
			test[1] = ((unsigned char*)pData)[i*3+1];
			test[2] = ((unsigned char*)pData)[i*3+2];
			//test[0] = test[0] >> 4;
			//test[0] = test[0] & 15;
			//test[1] = test[1] << 4;
			//test[1] = test[1] & 240;

			buf[i] = (test[0] & test[1] & test[2]);
		
		}

		QImage qImg = QImage((const uchar*) buf, rohW, rohH, QImage::Format_Indexed8);

		if (qImg.isNull())
			return false;
		//img = img.copy();
		QVector<QRgb> colorTable;

		for (int i = 0; i < 256; i++)
			colorTable.push_back(QColor(i, i, i).rgb());
		qImg.setColorTable(colorTable);

		OpenImage(QPixmap::fromImage(qImg));

	} catch(...) {
		imgLoaded = false;
	}
	
	free(buf);
	free(pData);


	return imgLoaded;

}


QImage MainWindow::MatToQImage(const Mat& mat)
{
    // 8-bits unsigned, NO. OF CHANNELS=1
    if(mat.type()==CV_8UC1)
    {
        // Set the color table (used to translate colour indexes to qRgb values)
        QVector<QRgb> colorTable;
        for (int i=0; i<256; i++)
            colorTable.push_back(qRgb(i,i,i));
        // Copy input Mat
        const uchar *qImageBuffer = (const uchar*)mat.data;
        // Create QImage with same dimensions as input Mat
        QImage img(qImageBuffer, mat.cols, mat.rows, mat.step, QImage::Format_Indexed8);
        img.setColorTable(colorTable);
        return img;
    }
    // 8-bits unsigned, NO. OF CHANNELS=3
    else if(mat.type()==CV_8UC3)
    {
        // Copy input Mat
        const uchar *qImageBuffer = (const uchar*)mat.data;
        // Create QImage with same dimensions as input Mat
        QImage img(qImageBuffer, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
        return img.rgbSwapped();
    }
    else
    {
        //qDebug() << "ERROR: Mat could not be converted to QImage.";
        return QImage();
    }
}

void MainWindow::on_ConvertImage() // canny edge detection part
{
    if (!filename.isEmpty())
    {
        if(image.data )                              // Check for invalid input
        {
            Mat src_gray;
            Mat detected_edges;

//          int edgeThresh = 1;
            int lowThreshold = 40;
//          int const max_lowThreshold = 100;
            int ratio = 3;
            int kernel_size = 3;

            /// Convert the image to grayscale
            cvtColor( image, src_gray, CV_BGR2GRAY );

            /// Reduce noise with a kernel 3x3
            blur( src_gray, detected_edges, Size(3,3) );

            /// Canny detector
            Canny( detected_edges, detected_edges, lowThreshold, lowThreshold*ratio, kernel_size );

            QImage qigm=MatToQImage(detected_edges);
            image=detected_edges.clone();
            ui->labelWebcam->setPixmap(QPixmap::fromImage(qigm).scaledToWidth(ui->labelWebcam->size().width(),Qt::FastTransformation));
        }
    }
}

void MainWindow::on_SaveImageFile()
{
    if (!filename.isEmpty())
    {
        if(image.data )                              // Check for invalid input
        {
            QString fname = QFileDialog::getSaveFileName(this,tr("Save Image File"), "", tr("Image Files (*.jpg *.png *.bmp *.jpeg *.tiff *.tif *.dib *.jp2 *.jpe *.ppm *.pgm *.pbm *.ras *.sr)"));
            imwrite( fname.toStdString(), image );
        }
    }
}

//resize window
void MainWindow::resizeEvent(QResizeEvent *)
{
    QRect childRect = scrollArea->childrenRect();
    imageWidget->resize(childRect.size());
}

// button method
void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Escape)
    {
        showNormal();//QWidget�Դ���ȫ���ָ�����
        showAll();
    }
}

// create action in Qt
void MainWindow::createActions()
{
    // open folder
    dirAct = new QAction(QIcon(tr("../OTK/Resources/open.ico")),tr("open folder"),this);
    dirAct->setShortcut(QKeySequence::Open);
	dirAct->setIconText(tr("open folder"));
	dirAct->setStatusTip(tr("open a image folder"));
    connect(dirAct,SIGNAL(triggered()),this,SLOT(selectDir()));

	//open the camera to take screenshot
    openCameraAct = new QAction(QIcon(tr("../OTK/Resources/camera.ico")),tr("screenshot"),this);
	openCameraAct->setShortcut(tr("Alt+M"));
	openCameraAct->setIconText(tr("screenshot"));
	openCameraAct->setStatusTip(tr("screenshot"));
    connect(openCameraAct,SIGNAL(triggered()),this,SLOT(openCamera()));
	
	// switch image
	imageControlByHandAct = new QAction(QIcon(tr("../OTK/Resources/hand.ico")),tr("switch image"),this);
	imageControlByHandAct->setShortcut(tr("Alt+H"));
	imageControlByHandAct->setIconText(tr("switch image"));
	imageControlByHandAct->setStatusTip(tr("switch image"));
    connect(imageControlByHandAct,SIGNAL(triggered()),this,SLOT(controlImage()));

	// rotate image
	imageControlByHandAct2 = new QAction(QIcon(tr("../OTK/Resources/hand.ico")),tr("rotate image"),this);
	imageControlByHandAct2->setShortcut(tr("Alt+R"));
	imageControlByHandAct2->setIconText(tr("rotate image"));
	imageControlByHandAct2->setStatusTip(tr("rotate image"));
    connect(imageControlByHandAct2,SIGNAL(triggered()),this,SLOT(controlImage2()));

    // switch next image
    nextAct = new QAction(QIcon(tr("../OTK/Resources/next.ico")),tr("next"),this);
    nextAct->setShortcut(QKeySequence::Forward);
	nextAct->setIconText(tr("next"));
	nextAct->setStatusTip(tr("switch next image"));
    connect(nextAct,SIGNAL(triggered()),this,SLOT(next()));

    //��һ��ͼƬ
    previousAct = new QAction(QIcon(tr("../OTK/Resources/previous.ico")),tr("��һ��"),this);
    previousAct->setShortcut(QKeySequence::Back);
	previousAct->setIconText(tr("��һ��"));
	previousAct->setStatusTip(tr("�л���һ��ͼƬ"));
    connect(previousAct,SIGNAL(triggered()),this,SLOT(previous()));

    //������ת
    leftAct = new QAction(QIcon(tr("../OTK/Resources/left.ico")),tr("������ת"),this);
    leftAct->setShortcut(tr("Ctrl+L"));
	leftAct->setIconText(tr("����ת"));
	leftAct->setStatusTip(tr("������ת90��"));
    connect(leftAct,SIGNAL(triggered()),this,SLOT(rotateLeft()));

    //������ת
    rightAct = new QAction(QIcon(tr("../OTK/Resources/right.ico")),tr("������ת"),this);
    rightAct->setShortcut(tr("Ctrl+R"));
	rightAct->setIconText(tr("����ת"));
	rightAct->setStatusTip(tr("������ת90��"));
    connect(rightAct,SIGNAL(triggered()),this,SLOT(rotateRight()));

    //�Ŵ�ͼƬ
    zoomInAct = new QAction(QIcon(tr("../OTK/Resources/zoomin.ico")),tr("�Ŵ�"),this);
    zoomInAct->setShortcut(QKeySequence::ZoomIn);
	zoomInAct->setIconText(tr("�Ŵ�"));
	zoomInAct->setStatusTip(tr("�Ŵ�ͼƬ"));
    connect(zoomInAct,SIGNAL(triggered()),this,SLOT(zoomIn()));

    //��СͼƬ
    zoomOutAct = new QAction(QIcon(tr("../OTK/Resources/zoomout.ico")),tr("��С"),this);
    zoomOutAct->setShortcut(QKeySequence::ZoomOut);
	zoomOutAct->setIconText(tr("��С"));
	zoomOutAct->setStatusTip(tr("��СͼƬ"));
    connect(zoomOutAct,SIGNAL(triggered()),this,SLOT(zoomOut()));

    //ʵ�ʴ�С
    actualSizeAct = new QAction(QIcon(tr("../OTK/Resources/actualsize.ico")),tr("ʵ�ʴ�С"),this);
    actualSizeAct->setShortcut(Qt::Key_Home);
	actualSizeAct->setIconText(tr("ʵ�ʴ�С"));
	actualSizeAct->setStatusTip(tr("���ʹͼƬ�ָ�ԭ�д�С"));
    connect(actualSizeAct,SIGNAL(triggered()),this,SLOT(actualSize()));

    //��Ӧ���ڴ�С
    fitSizeAct = new QAction(QIcon(tr("../OTK/Resources/fitscreen.ico")),tr("��Ӧ����"),this);
    fitSizeAct->setShortcut(Qt::Key_End);
	fitSizeAct->setIconText(tr("��Ӧ����"));
	fitSizeAct->setStatusTip(tr("��ͼƬ��С����Ϊ���ڴ�С"));
    connect(fitSizeAct,SIGNAL(triggered()),this,SLOT(fitSize()));

    //ȫ����ʾ
    fullScreenAct = new QAction(QIcon(tr("../OTK/Resources/fullwindow.ico")),tr("ȫ��"),this);
    fullScreenAct->setShortcut(Qt::Key_F11);
	fullScreenAct->setIconText(tr("ȫ��"));
	fullScreenAct->setStatusTip(tr("ȫ����ʾͼ��ȫ����ESC�����ɻָ�"));
    connect(fullScreenAct,SIGNAL(triggered()),this,SLOT(present()));

    //�˳�
    closeAct = new QAction(QIcon(tr("../OTK/Resources/exit.ico")),tr("�˳�"),this);
    closeAct->setShortcut(QKeySequence::Close);
	closeAct->setIconText(tr("�ر�"));
	closeAct->setStatusTip(tr("�˳������"));
    connect(closeAct,SIGNAL(triggered()),this,SLOT(myClose()));

	//���ڱ����
	aboutAct = new QAction(tr("����"), this);
	aboutAct->setIcon(QIcon("../OTK/Resources/about.ico"));
	aboutAct->setIconText(tr("����"));
	aboutAct->setStatusTip(tr("���ڱ�����������Ϣ���Լ�������Ϣ��"));
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));
}

//�����˵�
void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("�ļ�"));
    fileMenu->addAction(dirAct);
	fileMenu->addAction(openCameraAct);
	fileMenu->addAction(imageControlByHandAct);
	fileMenu->addAction(imageControlByHandAct2);
	fileMenu->addSeparator();
    fileMenu->addAction(closeAct);

    editMenu = menuBar()->addMenu(tr("�鿴"));
    editMenu->addAction(previousAct);
	editMenu->addAction(nextAct);
	editMenu->addSeparator();
    editMenu->addAction(leftAct);
    editMenu->addAction(rightAct);
	editMenu->addSeparator();
    editMenu->addAction(zoomInAct);
    editMenu->addAction(zoomOutAct);
	editMenu->addSeparator();
    editMenu->addAction(actualSizeAct);
    editMenu->addAction(fitSizeAct);
    editMenu->addAction(fullScreenAct);

	helpMenu=menuBar()->addMenu(tr("����"));
	helpMenu->addAction(aboutAct);
}

//����������
void MainWindow::createToolBars()
{
    fileToolBar = new QToolBar(this);
    fileToolBar->addAction(dirAct);
	fileToolBar->addAction(imageControlByHandAct);
	fileToolBar->addAction(imageControlByHandAct2);
    fileToolBar->addSeparator();
    fileToolBar->addAction(previousAct);
    fileToolBar->addAction(nextAct);

    editToolBar = new QToolBar(this);
    editToolBar->addAction(leftAct);
    editToolBar->addAction(rightAct);
    editToolBar->addAction(zoomInAct);
    editToolBar->addAction(zoomOutAct);
    editToolBar->addAction(actualSizeAct);
    editToolBar->addAction(fitSizeAct);
    editToolBar->addAction(fullScreenAct);
	editToolBar->addAction(aboutAct);
	editToolBar->addAction(closeAct);

    this->addToolBar(Qt::BottomToolBarArea,fileToolBar);
    this->addToolBar(Qt::BottomToolBarArea,editToolBar);

}

//����״̬��
void MainWindow::createStatusBar()
{
	statusLabel = new QLabel(tr(""));
	statusLabel->setMinimumSize(statusLabel->sizeHint());
	statusLabel->setAlignment(Qt::AlignHCenter); 
	statusBar()->setStyleSheet("QStatusBar::item {border: 0px;}");//ȥ��Ĭ�ϱ߿�
	statusBar()->showMessage(tr("Ready"));
    statusBar()->addWidget(statusLabel);
}

//��������ͼ��
void MainWindow::createTrayIcon()
{
	trayIconMenu = new QMenu(this);   
    trayIconMenu->addAction(dirAct);
    trayIconMenu->addAction(aboutAct);
	trayIconMenu->addAction(closeAct);
    trayIcon->setContextMenu(trayIconMenu);
}

//��һ��ͼƬ
void MainWindow::next()
{
	int num=imageList.size();
	index++;
	index=index%num;
    if(index < imageList.size())
    {	
		QString imageQstr=imageDir.absolutePath()+"/"+imageList.at(index);
		//QByteArray str1 = imageQstr.toLocal8Bit(); 
		//strfile = str1.data(); 
		//qDebug("ԭʼ");
		//qDebug(strfile);		
		//QMessageBox::about(this,tr("ͼƬ��ַ"),(imageQstr));
		//::commonQString=imageQstr;
		//openImage a = new openImage();
		//a.open();
        imageWidget->setPixmap(imageDir.absolutePath() + "/"
                               + imageList.at(index));
    }
}

//��һ��ͼƬ
void MainWindow::previous()
{
	int num=imageList.size();
	
    imageWidget->setPixmap(imageDir.absolutePath() + QDir::separator()
                               + imageList.at(index));
    if(index==0)
    {
		index=imageList.size()-1;
		
	}else{
		index=(index-1)%num;
		
	}	
}

//ѡ�������Ŀ¼
void MainWindow::selectDir()
{
    QString dir = QFileDialog::getExistingDirectory(this,
                  tr("���ļ���"),QDir::currentPath(),
                  QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	//QMessageBox::about(this,tr("���ļ���ַ"),(dir));
	//qDebug("���ļ���");
	//qDebug(dir);
    if(dir.isEmpty())
        return;
    imageDir.setPath(dir);
    QStringList filter;
    filter<<"*.jpg"<<"*.bmp"<<"*.jpeg"<<"*.png"<<"*.xpm"<<"*.ico";
    imageList = imageDir.entryList(filter,QDir::Files);     //����imageDir������ļ�
    next();
	openActionUse();
}

//������ת
void MainWindow::rotateLeft()
{
    imageWidget->setAngle(-90);
}

//������ת
void MainWindow::rotateRight()
 {
    imageWidget->setAngle(90);
 }

//�Ŵ�ͼƬ
void MainWindow::zoomIn()
 {
    imageWidget->scale *= 1.25;
    zoomInAct->setEnabled(imageWidget->scale < 3);
    zoomOutAct->setEnabled(imageWidget->scale > 0.333);
    imageWidget->resize(imageWidget->scale * scrollArea->size());
 }

//��СͼƬ
void MainWindow::zoomOut()
 {
     imageWidget->scale *= 0.8;
     zoomInAct->setEnabled(imageWidget->scale < 3);
     zoomOutAct->setEnabled(imageWidget->scale > 0.333);
     imageWidget->resize(imageWidget->scale * scrollArea->size());
 }

//��ʾͼ��ʵ�ʴ�С
void MainWindow::actualSize()
{
     imageWidget->scale = 1;
     imageWidget->bFit = false;
     imageWidget->update();
}

//��Ӧ���ڴ�С
void MainWindow::fitSize()
{
    imageWidget->scale = 1;
    imageWidget->bFit = true;
    imageWidget->update();

    //qDebug("fit!");
}

//ȫ����ʾ
void MainWindow::present()
{
    menuBar()->hide();
    fileToolBar->hide();
    editToolBar->hide();
	fitSize();
    showFullScreen();
	
}

//��ʾ������
void MainWindow::showAll()
{
     menuBar()->show();
     fileToolBar->show();
     editToolBar->show();
}

//��ǩʹ�ùر�
void MainWindow::closeActionUse()
{
	imageControlByHandAct->setEnabled(false);
	imageControlByHandAct2->setEnabled(false);
	previousAct->setEnabled(false);
	nextAct->setEnabled(false);
	leftAct->setEnabled(false);
	rightAct->setEnabled(false);
	zoomInAct->setEnabled(false);
	zoomOutAct->setEnabled(false);
	actualSizeAct->setEnabled(false);
	fitSizeAct->setEnabled(false);
	fullScreenAct->setEnabled(false);
}

//��ǩʹ�ÿ���
void MainWindow::openActionUse()
{
	imageControlByHandAct->setEnabled(true);
	imageControlByHandAct2->setEnabled(true);
	previousAct->setEnabled(true);
	nextAct->setEnabled(true);
	leftAct->setEnabled(true);
	rightAct->setEnabled(true);
	zoomInAct->setEnabled(true);
	zoomOutAct->setEnabled(true);
	actualSizeAct->setEnabled(true);
	fitSizeAct->setEnabled(true);
	fullScreenAct->setEnabled(true);
}

//�رճ���
void MainWindow::myClose()
{
      int ret = QMessageBox::warning(this, tr("EXIT"),
                                      tr("��ȷ����Ҫ�˳������ô��"),
                                      QMessageBox::Yes | QMessageBox::No,
                                      QMessageBox::No);
      if(ret == QMessageBox::Yes)
      {
        this->close();
      }

}

//���ڱ����
void MainWindow::about()//����
 {
     QMessageBox::about(this,tr("����OTK Application"),
		 tr("<p><b>OTK Application</b>��һ����ͼ�����"
                "�����ñ����ʵ��ͼƬ�����ͼƬ��ӡ��ͼƬ�����"
                "���ֹ��ܣ����⻹����ʵ�ֻ���OPENCV�����Ʋٿ�ͼƬ��  "
                "����������������У������ڴ�</p>\n<p>�����ˣ�LY</p>"));
 }

//������ͷ
void MainWindow::openCamera()                                                                               
{	
	QMessageBox::about(this,tr("��ͼ˵��"),
		 tr("<p>���س������ɽ�ͼ����������ͼ��ESC�������˳�</p>"));
	CvCapture* capture;
	capture=cvCreateCameraCapture(0);
	IplImage* frame;
	cvNamedWindow("cameraWindow",0);
	cvMoveWindow("cameraWindow",0,0);
	int imageSaveNum=1;
	while(1){
		frame=cvQueryFrame(capture);
		if(!frame){break;}
		cvShowImage("cameraWindow",frame);
		char c=cvWaitKey(24);
		if(c==13)
		{
			QDate date = QDate::currentDate();
			QString dateString = date.toString("yyyyMMdd");  
			QString fname=dateString;//yyyy-MM-dd
       
			//get current time
			QTime time = QTime::currentTime();
			QString timeString = time.toString("hhmmss"); 
			fname=fname+timeString+".jpg";

			cvNamedWindow("��ͼ",0);
			cvMoveWindow("��ͼ",700,0);
			cvShowImage("��ͼ",frame);
			switch (QMessageBox::question(this,tr("��ͼȷ��"),tr("���Ž�ͼ����������Ҫô��<br/>�����ϵĻ����԰�Cancel���˳������س����½�ͼ"),QMessageBox::Ok|QMessageBox::Cancel,QMessageBox::Ok))
			{
			case QMessageBox::Ok:
				cvSaveImage(fname.toStdString().c_str(),frame);
				QMessageBox::about(this,tr("�������"),tr("<p>������ϣ�ȥ����Ŀ¼�²鿴</p>"));
				cvDestroyWindow("��ͼ");
				OpenImage(fname);
				//cvReleaseCapture(&capture);
				//cvDestroyWindow("cameraWindow");
				break;
			case QMessageBox::Cancel:
				cvDestroyWindow("��ͼ");
				break;
			default:
				break;
			}
		}
		if(c==27)
		{
			break;
		}
	}
	cvReleaseCapture(&capture);
	cvDestroyWindow("cameraWindow");
				
}

//�ֿ�ͼƬ�л�
void MainWindow::controlImage()
{
	IplImage* originalImage = 0; // ԭͼ
	IplImage* imageBalanced = 0;//ɫ��ƽ��
	IplImage* ycbcr = 0;//ycbcr
	IplImage* imageLikelihood = 0;//��ɫ��Ȼ
	CvCapture* pCapture=NULL;
	pCapture=cvCreateCameraCapture(0);//���������
	cvSetCaptureProperty(pCapture,CV_CAP_PROP_FPS,1);
	cvSetCaptureProperty(pCapture,CV_CAP_PROP_FRAME_WIDTH,64);
	cvSetCaptureProperty(pCapture,CV_CAP_PROP_FRAME_HEIGHT,48);
	IplImage* imageSkinColor = 0;//��ɫ
	CvPoint imagePoint;
	int count=1;
	int x=0;
	int y=0;
	
	cvNamedWindow("ԭͼ", 0); 
	cvResizeWindow("ԭͼ",200,200);
	cvMoveWindow("ԭͼ",400,400);
	cvNamedWindow("��ɫͼ", 0); 
	cvResizeWindow("��ɫͼ",200,200);
	cvMoveWindow("��ɫͼ",615,400); 
	cvNamedWindow("ɫ��ƽ��",0);
	cvResizeWindow("ɫ��ƽ��",200,200);
	cvNamedWindow("���շ�ɫ��ֵͼ", 0);
	cvResizeWindow("���շ�ɫ��ֵͼ",200,200);
	
	int nFrmNum=0;//��Ƶ֡������
	//��֡��ȡ��Ƶ,��֡����
	while(originalImage=cvQueryFrame(pCapture))
	{
		nFrmNum++;
		if(nFrmNum==1)//��һ֡�����ռ�
		{
			imageBalanced=cvCreateImage(cvSize(originalImage->width,originalImage->height),IPL_DEPTH_8U,3);//��ͷ���������ݣ�size,depth,ͨ������
			ycbcr=cvCreateImage(cvSize(originalImage->width,originalImage->height),IPL_DEPTH_8U,3);
			imageLikelihood=cvCreateImage(cvSize(originalImage->width,originalImage->height),IPL_DEPTH_8U,1);
	
		}
		cvShowImage("ԭͼ", originalImage );
		imageSkinColor=cvCreateImage(cvSize(originalImage->width,originalImage->height),IPL_DEPTH_8U,3);
		*imageBalanced=colorBalance(originalImage);//ɫ��ƽ��
		cvShowImage("ɫ��ƽ��",imageBalanced);//��ʾɫ��ƽ����ͼ
		//CV_MEDIAN����ͼ����к˴�СΪparam1��param1 ����ֵ�˲�����������Ƿ��ģ���
		cvSmooth(imageBalanced, imageBalanced,CV_MEDIAN ,3,0,0,0 );//ƽ�� 
		cvCvtColor(imageBalanced, ycbcr, CV_RGB2YCrCb);// ��ɫ�ռ�ת��rgb->YCrCb
		*imageLikelihood=skinColorLikelihood(ycbcr);//��ɫ����	
		imagePoint=gravity(imageLikelihood);	
		//cout<<"�������꣨"<<imagePoint.x<<","<<imagePoint.y<<")";
		/////////////////*����������Ϊ�������*/
		CvMoments momentTemp;
		cvMoments(imageLikelihood,&momentTemp,1);//�����ֵͼ��ľ���Ϣ
		double area=cvGetSpatialMoment(&momentTemp,0,0);//m00=��������
		int area1=(int)(area);
		if(area1>2200)
		{
			if(count==1)
			{
				x=imagePoint.x;
				y=imagePoint.y;
			}
			if(count<3)
			{
				count++;
			}
			else
			{
				count=1;
			}
			if(count==3)
			{
				if(imagePoint.x-x>10)
				{
					next();
					count=1;
					x=0;
					y=0;
				}
				else if(x-imagePoint.x>10)
				{
					previous();
					count=1;
					x=0;
					y=0;
				}
			/*
				if(imagePoint.y-y>10)
				{
					rotateRight();
					count=1;
					x=0;
					y=0;
				}
				else if(y-imagePoint.y>10)
				{
					rotateLeft();
					count=1;
					x=0;
					y=0;
				}
			*/
			}
		}
		//////////////////////�������ж϶����Ĵ���
		cvShowImage("���շ�ɫ��ֵͼ",imageLikelihood );//�����ķ�ɫͼ��ʾ		
		
		//��ʾ��ɫͼ
		int height,width,step,channels;
		uchar *data;
		height = originalImage->height; 
		width = originalImage->width; 
		step = originalImage->widthStep; 
		channels = originalImage->nChannels;
		data = (uchar *)originalImage->imageData;
		int i,j;
		for(i=0;i<height;i++)
		{
			for(j=0;j<width;j++)
			{
				if(((imageLikelihood->imageData + imageLikelihood->widthStep*i))[j]==0)
				{
					data[i*step+j*channels+0]=0;
					data[i*step+j*channels+1]=0;
					data[i*step+j*channels+2]=0;
				}
			}
		}
		cvShowImage("��ɫͼ", originalImage );
		
		
		if(cvWaitKey(1)>=0)
		{
			break;
		}
	}
	//�������β����
	///*
	cvDestroyWindow( "ԭͼ" );//���ٴ���
	//cvDestroyWindow( "��ɫͼ" );//���ٴ���
	cvDestroyWindow( "ɫ��ƽ��" );//���ٴ���
	cvReleaseImage( &imageBalanced ); //�ͷ�ͼ��
	cvDestroyWindow( "��ɫ��Ȼ" );//���ٴ���
	cvReleaseImage( &imageLikelihood ); //�ͷ�ͼ��
	cvDestroyWindow( "���շ�ɫ��ֵͼ" );//���ٴ���
	//*/
}

//ɫ��ƽ�⣨gray world��ɫ���ⷽ����
IplImage MainWindow::colorBalance( IplImage* imageBalanced)
{
	IplImage* dst;//ɫ��ƽ��
	IplImage* gray;//�Ҷ�ͼ
	dst=cvCreateImage(cvSize(imageBalanced->width,imageBalanced->height),IPL_DEPTH_8U,3);
	gray=cvCreateImage(cvSize(imageBalanced->width,imageBalanced->height),IPL_DEPTH_8U,1);
	int height,width,step,channels;
	uchar *data,*dataDST;
	height = imageBalanced->height; 
	width = imageBalanced->width;//width��ָÿ�е����ظ���������һ������ռһ���ֽ�Ҳ���������ĸ��ֽ�
	step = imageBalanced->widthStep; //widthstep�����ֽ���,Ӧ����4�ı���,��һ������width
	channels = imageBalanced->nChannels;
	data = (uchar *)imageBalanced->imageData;
	dataDST = (uchar *)dst->imageData;
	int i,j;
	double R,G,B,Gy,aR,aG,aB;
	cvZero(dst);//cvZero(IplImage��ͼƬ):��ʼ��ͼƬ,ֵ��Ϊ0,�����СΪ640*480
	CvMat* MR=cvCreateMat(height,width,CV_64FC1);//(rows,cols,type) CV_64FC1��64λ/ָ��double
	CvMat* MG=cvCreateMat(height,width,CV_64FC1);
	CvMat* MB=cvCreateMat(height,width,CV_64FC1);
	for(i=0;i<height;i++) 
	{
		for(j=0;j<width;j++)
		{
			B=data[i*step+j*channels+0];
			G=data[i*step+j*channels+1];
			R=data[i*step+j*channels+2];
			Gy=((gray->imageData + gray->widthStep*i))[j];
			cvmSet(MB,i,j,B);//set MB(i,j);ֵΪB
			cvmSet(MG,i,j,G);
			cvmSet(MR,i,j,R);
			((gray->imageData + gray->widthStep*i))[j]=(B+G+R)/3;
		}
	}
	CvScalar argR,argG,argB;//cvScalar��һ������ṹ�壬���ĸ�����
	double argI;
	argR=cvAvg(MR,0);//��������������Ԫ�ص�ƽ��ֵ��arr,mask����mask�ǿ�ʱ����ôƽ��ֵ������Щmask��0��Ԫ�����Ӧ���������
	argG=cvAvg(MG,0);
	argB=cvAvg(MB,0);
	argI=(argR.val[0]+argG.val[0]+argB.val[0])/3;//�õ�ƽ���Ҷ�ֵ
	//�������
	aR=argI/argR.val[0];
	aG=argI/argG.val[0];
	aB=argI/argB.val[0];
	for(i=0;i<height;i++)
	{
		for(j=0;j<width;j++)
		{
			//CV_MAT_ELEM( matrixҪ���ʵľ���, elemtype����Ԫ������, row, col )
			//CV_MAT_ELEM��������ÿ��Ԫ�صĺֻ꣬�Ե�ͨ����Ч
			B=CV_MAT_ELEM(*MB,double,i,j)*aB;
			G=CV_MAT_ELEM(*MG,double,i,j)*aG;
			R=CV_MAT_ELEM(*MR,double,i,j)*aR;
			if(B>255)dataDST[i*step+j*channels+0]=255;
			else dataDST[i*step+j*channels+0]=B;
			if(G>255)dataDST[i*step+j*channels+1]=255;
			else dataDST[i*step+j*channels+1]=G;
			if(R>255)dataDST[i*step+j*channels+2]=255;
			else dataDST[i*step+j*channels+2]=R;
		}
	}
	return *dst;
}

//��ɫ��Ȼ(��ɫ��⣬����ֵ���Ҷ�ͼ��ɶ�ֵͼ��������)
IplImage MainWindow::skinColorLikelihood(IplImage* imageLikelihood)
{	
	IplImage* dst;//��ɫͼ
	dst=cvCreateImage(cvSize(imageLikelihood->width,imageLikelihood->height),IPL_DEPTH_8U,1);
	int height,width,step,channels;
	uchar *data;
	height = imageLikelihood->height; 
	width = imageLikelihood->width; 
	step = imageLikelihood->widthStep; 
	channels = imageLikelihood->nChannels;
	data = (uchar *)imageLikelihood->imageData;
	double M[4]={0.0077 ,-0.0041,-0.0041 ,0.0047};//��ɫ��Э������������(����ֵ)
	CvMat* max=cvCreateMat(height,width,CV_64FC1);//��һ����imageLikelihoodһ����ʽ�Ĵ洢��ɫ�����ܶȾ������飩
	CvMat* mat=cvCreateMat(2,2,CV_64FC1);//2��2�д洢Э��������棩
	CvMat* cbcr=cvCreateMat(1,2,CV_64FC1);//1��2��{cb,cr}
	CvMat* cbcrChanged=cvCreateMat(2,1,CV_64FC1);//2��1��{cb,cr}ת����
	CvMat* res=cvCreateMat(1,1,CV_64FC1);//1��1��
	cvInitMatHeader(mat,2,2,CV_64FC1,M);//M��ֵ��mat,mat=[0.0077,-0.0041][-0.0041,0.0047]
	double valueToRes[1]={0};
	cvInitMatHeader(res,1,1,CV_64FC1,valueToRes);//��res��ʼ��=valueToRes[1](��0)
		
	int i,j;
	double r,b,cb,cr,u;
	//��������Ĺ�ʽ���з�ɫ��Ȼ�ȼ���
	for(i=0;i<height;i++)
	{
		for(j=0;j<width;j++)
		{	
			r=data[i*step+j*channels+2];//cr ����
			b=data[i*step+j*channels+1];//cb ����
			cb=b-103.0056;////117.4361
			cr=r-140.1309;////156.5599
			double p1[2]={cb,cr};
			cvInitMatHeader(cbcr,1,2,CV_64FC1,p1);//p1��ֵ��cbcr,cbcr={r-103.0056,b-140.1309}
			cvMatMulAdd(cbcr,mat,0,cbcr);//��src1,src2,src3,dst��dst=src1*src2+src3;��cbcr=cbcr*mat+0
			//cbcr={(r-103.0056)*0.0077-0.0041*(b-140.1309),-0.0041*(r-103.0056)+0.0047*(b-140.1309)}
			double p2[2]={cb,cr};
			cvInitMatHeader(cbcrChanged,2,1,CV_64FC1,p2);//(mat,rows,cols,type,data)dataΪ��ѡ�ģ���ָ������ָ����������ͷ. 
			cvMatMulAdd(cbcr,cbcrChanged,0,res);//res=cbcr*cbcrChanged+0;
			u=CV_MAT_ELEM(*res,double,0,0);//u=res�����У�0,0��λ�õ�ֵ
			u=exp(-0.5*u);//e��-0.5*u�η� ֵӰ������
			cvmSet(max,i,j,u);//max(i,j)=u;���������θ�ֵ
		}
	}
	double maxNumInMatrix=0; //��ʼ�����ֵ
	double numZeroToOne=0;
	cvMinMaxLoc(max,NULL,&maxNumInMatrix,NULL,NULL);//������max�����ֵ��ֵ��maxNumInMatrix
	//��ɫ��Ȼ��������0-255��Χ����
	for(i=0;i<height;i++)
	{
		for(j=0;j<width;j++)
		{
			numZeroToOne=CV_MAT_ELEM(*max,double,i,j)/maxNumInMatrix;//��ֵ���0-1��Χ��ֵ
			cvmSet(max,i,j,255*numZeroToOne);//max(i,j)=255*numZeroToOne;
			((dst->imageData + dst->widthStep*i))[j]=255*numZeroToOne;
		}
	}
	//cvNamedWindow("��ɫdst", CV_WINDOW_AUTOSIZE); 
	cvNamedWindow("��ɫdst",0);
	cvResizeWindow("��ɫdst",200,200);
	cvMoveWindow("��ɫdst",800,400);
	cvShowImage("��ɫdst", dst );
	double n[256]={0};
	double p[256]={0};
	int locationMatrix;
	for(i=0;i<height;i++)
	{
		for(j=0;j<width;j++)
		{
			locationMatrix=int(CV_MAT_ELEM(*max,double,i,j));
			n[locationMatrix]++;
		}
	}
	for(i=0;i<256;i++)
	{
		p[i]=n[i]/(height*width);
	}
	//�����䷽�(OTSU��)����ͼ���ֵ��������Ӧ��ֵ(�������Ҷ�ͼ��ת���ɶ�ֵͼ��)
	double g;//ǰ���ͱ���ͼ��ķ���g
	double w0;//ǰ������ռͼ�����Ϊw0
	double w1;//��������ռͼ�����Ϊw1
	double u0;//ǰ����ƽ���Ҷ�Ϊu0
	double u1;//������ƽ���Ҷ�Ϊu1
	double ut;//ͼ�����ƽ���Ҷ�Ϊ��ut=w0*u0+w1*u1��
	double Q[]={0,0,0,0,0,0,0,0,0,0};
	int flag=0;
	double max2;
	j=0;
	//���ȱ���ǵ�Ŀ����ͱ����ҶȲ�����ʱ������ִ���ɫ���������ᶪʧͼ����Ϣ
	//�ڴ�򷨵Ļ����ϰѻҶ����죬��ǿ�Ҷȼ����������1�ľ��Ǵ��
	/*
	for(int m=10;m<=190;m+=20)//���20
	{
		g=0;w0=0;w1=0;u0=0;u1=0;ut=0;//ѭ������Ϊ0Ϊ��ÿ��ѭ��ʱ�����¼���		
		for(i=0;i<=m;i++){
			w0+=p[i];
			u0+=i*p[i]/w0;
		}
		for(i=m+1;i<=255;i++){
			w1+=p[i];
			u1+=i*p[i]/w1;
		}
		
		for(i=0;i<=255;i++){ut+=i*p[i];}
		
		//ut=w0*u0+w1*u1;
		//ǰ���ͱ���ͼ��ķ��g=w0*(u0-ut)*(u0-ut)+w1*(u1-ut)*(u1-ut)=w0*w1*(u0-u1)*(u0-u1),
		//������g���ʱ��������Ϊ��ʱǰ���ͱ����������Ҳ���Ǵ�ʱ�ĻҶ��������ֵ
		g=w0*(u0-ut)*(u0-ut)+w1*(u1-ut)*(u1-ut);//����
		if(g<1){g=0;}
		Q[flag]=g;
		if(flag=0){
			max2=Q[flag];
		}
		if(max2<Q[flag]){
			max2=Q[flag];
			j=flag;
		}
		flag++;
	}
	*/
	for(int m=10;m<=190;m+=20)
		{
		g=0;w0=0;w1=0;u0=0;u1=0;ut=0;//ѭ������Ϊ0Ϊ��ÿ��ѭ��ʱ�����¼���		
		for(i=0;i<=m;i++){
			w0+=p[i];
			u0+=i*p[i]/w0;
		}
		for(i=m+1;i<=255;i++){
			w1+=p[i];
			u1+=i*p[i]/w1;
		}
		for(i=0;i<=255;i++){ut+=i*p[i];}
		
		g=w0*(u0-ut)*(u0-ut)+w1*(u1-ut)*(u1-ut);
			if (g<1)g=0;
			Q[flag]=g;
			flag++;
		}
		max2=Q[0];
		j=0;
		for(i=1;i<=9;i++)
			if(max2<Q[i])
			{
				max2=Q[i];
				j=i;
			}



	int thresholdValue;//��ֵ
	thresholdValue=20*j;//20Ϊ��ļ���ĳ���
	//����ֵ������ֵͼ
	IplImage* imageBinary;//��ֵͼ	
	IplImage* imageBinaryProcessed;//�����������Ķ�ֵͼ
	imageBinary=cvCreateImage(cvSize(imageLikelihood->width,imageLikelihood->height),IPL_DEPTH_8U,1);
	imageBinaryProcessed=cvCreateImage(cvSize(imageLikelihood->width,imageLikelihood->height),IPL_DEPTH_8U,1);
	cvZero(imageBinaryProcessed);
	cvZero(imageBinary);
	for(i=0;i<height;i++)
	{
		for(j=0;j<width;j++)
		{
			locationMatrix=int(CV_MAT_ELEM(*max,double,i,j));
			if(locationMatrix>thresholdValue)
			{
				((imageBinary->imageData + imageBinary->widthStep*i))[j]=255;//��Ϊȫ��
			}
		}
	}		
	CvMemStorage* storage = cvCreateMemStorage(0);//�ڴ�洢��������������
	CvSeq* contour = 0;//����
	cvThreshold(imageBinary,imageBinary,128,255,CV_THRESH_BINARY);//�ԻҶ�ͼ�������ֵ�����õ���ֵͼ��
	cvDilate( imageBinary, imageBinary, NULL, 1 );//����
	/*
����CV_RETR_CCOMP���������е�����������������֯Ϊ���㣺�����Ǹ����ֵ��ⲿ�߽磬�ڶ����ǿն��ı߽磻
����CV_CHAIN_APPROX_SIMPLE��ѹ��ˮƽ�ġ���ֱ�ĺ�б�Ĳ��֣�Ҳ���ǣ�����ֻ�������ǵ��յ㲿�֡�
	*/
	cvFindContours( imageBinary, storage, &contour, sizeof(CvContour), CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE );//��ֵͼ���м������
	double contour_area = 0;	
	for(;contour!=0;contour=contour->h_next)
	{
		contour_area=100*fabs(cvContourArea(contour,CV_WHOLE_SEQ )); //fabs()�����ֵ��cvContourArea()���������
		if(contour_area>height*width)//((contour->v_next!=0)&&(contour_area>height*width))
		{
			cvDrawContours(imageBinaryProcessed,contour,CV_RGB(255,255,255),CV_RGB(255,255,255),0,CV_FILLED,8 );//��ͼ���ϻ����ⲿ���ڲ�����
		}
	}
	return *imageBinaryProcessed;
}

//��������
CvPoint MainWindow::gravity(IplImage *imageLikelihood)//�����ֵͼ��
{
	CvPoint center;
	double m00,m10,m01;
	CvMoments moment;
	cvMoments(imageLikelihood,&moment,1);//�����ֵͼ��ľ���Ϣ
	m00=cvGetSpatialMoment(&moment,0,0);//m00=��������
	m10=cvGetSpatialMoment(&moment,1,0);//���ָ��ά�ľ���Ϣ
	m01=cvGetSpatialMoment(&moment,0,1);
	center.x =(int)(m10/m00);//������
	center.y =(int)(m01/m00);
	return center;
}

//�ֿ�ͼƬ��ת
void MainWindow::controlImage2()
{
	IplImage* originalImage = 0; // ԭͼ
	IplImage* imageBalanced = 0;//ɫ��ƽ��
	IplImage* ycbcr = 0;//ycbcr
	IplImage* imageLikelihood = 0;//��ɫ��Ȼ
	CvCapture* pCapture=NULL;
	pCapture=cvCreateCameraCapture(0);//���������
	cvSetCaptureProperty(pCapture,CV_CAP_PROP_FPS,1);
	cvSetCaptureProperty(pCapture,CV_CAP_PROP_FRAME_WIDTH,64);
	cvSetCaptureProperty(pCapture,CV_CAP_PROP_FRAME_HEIGHT,48);
	IplImage* imageSkinColor = 0;//��ɫ
	CvPoint imagePoint;
	int count=1;
	int x=0;
	int y=0;
	
	cvNamedWindow("ԭͼ", 0); 
	cvResizeWindow("ԭͼ",200,200);
	cvMoveWindow("ԭͼ",400,400);
	cvNamedWindow("��ɫͼ", 0); 
	cvResizeWindow("��ɫͼ",200,200);
	cvMoveWindow("��ɫͼ",615,400); 
	cvNamedWindow("ɫ��ƽ��",0);
	cvResizeWindow("ɫ��ƽ��",200,200);
	cvNamedWindow("���շ�ɫ��ֵͼ", 0);
	cvResizeWindow("���շ�ɫ��ֵͼ",200,200);
	
	int nFrmNum=0;//��Ƶ֡������
	//��֡��ȡ��Ƶ,��֡����
	while(originalImage=cvQueryFrame(pCapture))
	{
		nFrmNum++;
		if(nFrmNum==1)//��һ֡�����ռ�
		{
			imageBalanced=cvCreateImage(cvSize(originalImage->width,originalImage->height),IPL_DEPTH_8U,3);//��ͷ���������ݣ�size,depth,ͨ������
			ycbcr=cvCreateImage(cvSize(originalImage->width,originalImage->height),IPL_DEPTH_8U,3);
			imageLikelihood=cvCreateImage(cvSize(originalImage->width,originalImage->height),IPL_DEPTH_8U,1);
	
		}
		cvShowImage("ԭͼ", originalImage );
		imageSkinColor=cvCreateImage(cvSize(originalImage->width,originalImage->height),IPL_DEPTH_8U,3);
		*imageBalanced=colorBalance(originalImage);//ɫ��ƽ��
		cvShowImage("ɫ��ƽ��",imageBalanced);//��ʾɫ��ƽ����ͼ
		//CV_MEDIAN����ͼ����к˴�СΪparam1��param1 ����ֵ�˲�����������Ƿ��ģ���
		cvSmooth(imageBalanced, imageBalanced,CV_MEDIAN ,3,0,0,0 );//ƽ�� 
		cvCvtColor(imageBalanced, ycbcr, CV_RGB2YCrCb);// ��ɫ�ռ�ת��rgb->YCrCb
		*imageLikelihood=skinColorLikelihood(ycbcr);//��ɫ����	
		imagePoint=gravity(imageLikelihood);	
		//cout<<"�������꣨"<<imagePoint.x<<","<<imagePoint.y<<")";
		/////////////////*����������Ϊ�������*/
		CvMoments momentTemp;
		cvMoments(imageLikelihood,&momentTemp,1);//�����ֵͼ��ľ���Ϣ
		double area=cvGetSpatialMoment(&momentTemp,0,0);//m00=��������
		int area1=(int)(area);
		if(area1>2200)
		{
			if(count==1)
			{
				x=imagePoint.x;
				y=imagePoint.y;
			}
			if(count<3)
			{
				count++;
			}
			else
			{
				count=1;
			}
			if(count==3)
			{
				/*
				if(imagePoint.x-x>10)
				{
					next();
					count=1;
					x=0;
					y=0;
				}
				else if(x-imagePoint.x>10)
				{
					previous();
					count=1;
					x=0;
					y=0;
				}
			*/
				if(imagePoint.y-y>5)
				{
					rotateRight();
					count=1;
					x=0;
					y=0;
				}
				else if(y-imagePoint.y>5)
				{
					rotateLeft();
					count=1;
					x=0;
					y=0;
				}
			
			}
		}
		//////////////////////�������ж϶����Ĵ���
		cvShowImage("���շ�ɫ��ֵͼ",imageLikelihood );//�����ķ�ɫͼ��ʾ		
		/*
		//��ʾ��ɫͼ
		int height,width,step,channels;
		uchar *data;
		height = originalImage->height; 
		width = originalImage->width; 
		step = originalImage->widthStep; 
		channels = originalImage->nChannels;
		data = (uchar *)originalImage->imageData;
		int i,j;
		for(i=0;i<height;i++)
		{
			for(j=0;j<width;j++)
			{
				if(((imageLikelihood->imageData + imageLikelihood->widthStep*i))[j]==0)
				{
					data[i*step+j*channels+0]=0;
					data[i*step+j*channels+1]=0;
					data[i*step+j*channels+2]=0;
				}
			}
		}
		cvShowImage("��ɫͼ", originalImage );
		*/
		
		if(cvWaitKey(1)>=0)
		{
			break;
		}
	}
	//�������β����
	///*
	cvDestroyWindow( "ԭͼ" );//���ٴ���
	//cvDestroyWindow( "��ɫͼ" );//���ٴ���
	cvDestroyWindow( "ɫ��ƽ��" );//���ٴ���
	cvReleaseImage( &imageBalanced ); //�ͷ�ͼ��
	cvDestroyWindow( "��ɫ��Ȼ" );//���ٴ���
	cvReleaseImage( &imageLikelihood ); //�ͷ�ͼ��
	cvDestroyWindow( "���շ�ɫ��ֵͼ" );//���ٴ���
	//*/
}

void MainWindow::OpenImage(QString fileName)
{
	imageWidget->setPixmap(filename);
	leftAct->setEnabled(true);
	rightAct->setEnabled(true);
	zoomInAct->setEnabled(true);
	zoomOutAct->setEnabled(true);
	actualSizeAct->setEnabled(true);
	fitSizeAct->setEnabled(true);
	fullScreenAct->setEnabled(true);
}

void MainWindow::OpenImage(QPixmap pix)
{
	imageWidget->setPixmap(pix);
	leftAct->setEnabled(true);
	rightAct->setEnabled(true);
	zoomInAct->setEnabled(true);
	zoomOutAct->setEnabled(true);
	actualSizeAct->setEnabled(true);
	fitSizeAct->setEnabled(true);
	fullScreenAct->setEnabled(true);
}