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
        showNormal();//QWidget自带从全屏恢复正常
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

    //上一张图片
    previousAct = new QAction(QIcon(tr("../OTK/Resources/previous.ico")),tr("上一张"),this);
    previousAct->setShortcut(QKeySequence::Back);
	previousAct->setIconText(tr("上一张"));
	previousAct->setStatusTip(tr("切换上一张图片"));
    connect(previousAct,SIGNAL(triggered()),this,SLOT(previous()));

    //向左旋转
    leftAct = new QAction(QIcon(tr("../OTK/Resources/left.ico")),tr("向左旋转"),this);
    leftAct->setShortcut(tr("Ctrl+L"));
	leftAct->setIconText(tr("左旋转"));
	leftAct->setStatusTip(tr("向左旋转90度"));
    connect(leftAct,SIGNAL(triggered()),this,SLOT(rotateLeft()));

    //向右旋转
    rightAct = new QAction(QIcon(tr("../OTK/Resources/right.ico")),tr("向右旋转"),this);
    rightAct->setShortcut(tr("Ctrl+R"));
	rightAct->setIconText(tr("右旋转"));
	rightAct->setStatusTip(tr("向右旋转90度"));
    connect(rightAct,SIGNAL(triggered()),this,SLOT(rotateRight()));

    //放大图片
    zoomInAct = new QAction(QIcon(tr("../OTK/Resources/zoomin.ico")),tr("放大"),this);
    zoomInAct->setShortcut(QKeySequence::ZoomIn);
	zoomInAct->setIconText(tr("放大"));
	zoomInAct->setStatusTip(tr("放大图片"));
    connect(zoomInAct,SIGNAL(triggered()),this,SLOT(zoomIn()));

    //缩小图片
    zoomOutAct = new QAction(QIcon(tr("../OTK/Resources/zoomout.ico")),tr("缩小"),this);
    zoomOutAct->setShortcut(QKeySequence::ZoomOut);
	zoomOutAct->setIconText(tr("缩小"));
	zoomOutAct->setStatusTip(tr("缩小图片"));
    connect(zoomOutAct,SIGNAL(triggered()),this,SLOT(zoomOut()));

    //实际大小
    actualSizeAct = new QAction(QIcon(tr("../OTK/Resources/actualsize.ico")),tr("实际大小"),this);
    actualSizeAct->setShortcut(Qt::Key_Home);
	actualSizeAct->setIconText(tr("实际大小"));
	actualSizeAct->setStatusTip(tr("点击使图片恢复原有大小"));
    connect(actualSizeAct,SIGNAL(triggered()),this,SLOT(actualSize()));

    //适应窗口大小
    fitSizeAct = new QAction(QIcon(tr("../OTK/Resources/fitscreen.ico")),tr("适应窗口"),this);
    fitSizeAct->setShortcut(Qt::Key_End);
	fitSizeAct->setIconText(tr("适应窗口"));
	fitSizeAct->setStatusTip(tr("将图片大小设置为窗口大小"));
    connect(fitSizeAct,SIGNAL(triggered()),this,SLOT(fitSize()));

    //全屏显示
    fullScreenAct = new QAction(QIcon(tr("../OTK/Resources/fullwindow.ico")),tr("全屏"),this);
    fullScreenAct->setShortcut(Qt::Key_F11);
	fullScreenAct->setIconText(tr("全屏"));
	fullScreenAct->setStatusTip(tr("全屏显示图像，全屏后按ESC键即可恢复"));
    connect(fullScreenAct,SIGNAL(triggered()),this,SLOT(present()));

    //退出
    closeAct = new QAction(QIcon(tr("../OTK/Resources/exit.ico")),tr("退出"),this);
    closeAct->setShortcut(QKeySequence::Close);
	closeAct->setIconText(tr("关闭"));
	closeAct->setStatusTip(tr("退出本软件"));
    connect(closeAct,SIGNAL(triggered()),this,SLOT(myClose()));

	//关于本软件
	aboutAct = new QAction(tr("关于"), this);
	aboutAct->setIcon(QIcon("../OTK/Resources/about.ico"));
	aboutAct->setIconText(tr("关于"));
	aboutAct->setStatusTip(tr("关于本软件的相关消息，以及作者消息等"));
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));
}

//创建菜单
void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("文件"));
    fileMenu->addAction(dirAct);
	fileMenu->addAction(openCameraAct);
	fileMenu->addAction(imageControlByHandAct);
	fileMenu->addAction(imageControlByHandAct2);
	fileMenu->addSeparator();
    fileMenu->addAction(closeAct);

    editMenu = menuBar()->addMenu(tr("查看"));
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

	helpMenu=menuBar()->addMenu(tr("帮助"));
	helpMenu->addAction(aboutAct);
}

//创建工具条
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

//创建状态栏
void MainWindow::createStatusBar()
{
	statusLabel = new QLabel(tr(""));
	statusLabel->setMinimumSize(statusLabel->sizeHint());
	statusLabel->setAlignment(Qt::AlignHCenter); 
	statusBar()->setStyleSheet("QStatusBar::item {border: 0px;}");//去掉默认边框
	statusBar()->showMessage(tr("Ready"));
    statusBar()->addWidget(statusLabel);
}

//创建托盘图标
void MainWindow::createTrayIcon()
{
	trayIconMenu = new QMenu(this);   
    trayIconMenu->addAction(dirAct);
    trayIconMenu->addAction(aboutAct);
	trayIconMenu->addAction(closeAct);
    trayIcon->setContextMenu(trayIconMenu);
}

//下一个图片
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
		//qDebug("原始");
		//qDebug(strfile);		
		//QMessageBox::about(this,tr("图片地址"),(imageQstr));
		//::commonQString=imageQstr;
		//openImage a = new openImage();
		//a.open();
        imageWidget->setPixmap(imageDir.absolutePath() + "/"
                               + imageList.at(index));
    }
}

//上一个图片
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

//选择浏览的目录
void MainWindow::selectDir()
{
    QString dir = QFileDialog::getExistingDirectory(this,
                  tr("打开文件夹"),QDir::currentPath(),
                  QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	//QMessageBox::about(this,tr("打开文件地址"),(dir));
	//qDebug("打开文件夹");
	//qDebug(dir);
    if(dir.isEmpty())
        return;
    imageDir.setPath(dir);
    QStringList filter;
    filter<<"*.jpg"<<"*.bmp"<<"*.jpeg"<<"*.png"<<"*.xpm"<<"*.ico";
    imageList = imageDir.entryList(filter,QDir::Files);     //返回imageDir里面的文件
    next();
	openActionUse();
}

//向左旋转
void MainWindow::rotateLeft()
{
    imageWidget->setAngle(-90);
}

//向右旋转
void MainWindow::rotateRight()
 {
    imageWidget->setAngle(90);
 }

//放大图片
void MainWindow::zoomIn()
 {
    imageWidget->scale *= 1.25;
    zoomInAct->setEnabled(imageWidget->scale < 3);
    zoomOutAct->setEnabled(imageWidget->scale > 0.333);
    imageWidget->resize(imageWidget->scale * scrollArea->size());
 }

//缩小图片
void MainWindow::zoomOut()
 {
     imageWidget->scale *= 0.8;
     zoomInAct->setEnabled(imageWidget->scale < 3);
     zoomOutAct->setEnabled(imageWidget->scale > 0.333);
     imageWidget->resize(imageWidget->scale * scrollArea->size());
 }

//显示图像实际大小
void MainWindow::actualSize()
{
     imageWidget->scale = 1;
     imageWidget->bFit = false;
     imageWidget->update();
}

//适应窗口大小
void MainWindow::fitSize()
{
    imageWidget->scale = 1;
    imageWidget->bFit = true;
    imageWidget->update();

    //qDebug("fit!");
}

//全屏显示
void MainWindow::present()
{
    menuBar()->hide();
    fileToolBar->hide();
    editToolBar->hide();
	fitSize();
    showFullScreen();
	
}

//显示各个栏
void MainWindow::showAll()
{
     menuBar()->show();
     fileToolBar->show();
     editToolBar->show();
}

//标签使用关闭
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

//标签使用开启
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

//关闭程序
void MainWindow::myClose()
{
      int ret = QMessageBox::warning(this, tr("EXIT"),
                                      tr("你确定你要退出本软件么？"),
                                      QMessageBox::Yes | QMessageBox::No,
                                      QMessageBox::No);
      if(ret == QMessageBox::Yes)
      {
        this->close();
      }

}

//关于本软件
void MainWindow::about()//关于
 {
     QMessageBox::about(this,tr("关于OTK Application"),
		 tr("<p><b>OTK Application</b>是一个读图软件，"
                "可以用本软件实现图片浏览、图片打印、图片处理等"
                "多种功能，此外还可以实现基于OPENCV的手势操控图片等  "
                "后续功能正在添加中，敬请期待</p>\n<p>制作人：LY</p>"));
 }

//打开摄像头
void MainWindow::openCamera()                                                                               
{	
	QMessageBox::about(this,tr("截图说明"),
		 tr("<p>按回车键即可截图，如果不想截图则按ESC键即可退出</p>"));
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

			cvNamedWindow("截图",0);
			cvMoveWindow("截图",700,0);
			cvShowImage("截图",frame);
			switch (QMessageBox::question(this,tr("截图确认"),tr("这张截图符合您的需要么？<br/>不符合的话可以按Cancel键退出并按回车重新截图"),QMessageBox::Ok|QMessageBox::Cancel,QMessageBox::Ok))
			{
			case QMessageBox::Ok:
				cvSaveImage(fname.toStdString().c_str(),frame);
				QMessageBox::about(this,tr("保存完毕"),tr("<p>保存完毕，去工程目录下查看</p>"));
				cvDestroyWindow("截图");
				OpenImage(fname);
				//cvReleaseCapture(&capture);
				//cvDestroyWindow("cameraWindow");
				break;
			case QMessageBox::Cancel:
				cvDestroyWindow("截图");
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

//手控图片切换
void MainWindow::controlImage()
{
	IplImage* originalImage = 0; // 原图
	IplImage* imageBalanced = 0;//色彩平衡
	IplImage* ycbcr = 0;//ycbcr
	IplImage* imageLikelihood = 0;//肤色似然
	CvCapture* pCapture=NULL;
	pCapture=cvCreateCameraCapture(0);//设置摄像机
	cvSetCaptureProperty(pCapture,CV_CAP_PROP_FPS,1);
	cvSetCaptureProperty(pCapture,CV_CAP_PROP_FRAME_WIDTH,64);
	cvSetCaptureProperty(pCapture,CV_CAP_PROP_FRAME_HEIGHT,48);
	IplImage* imageSkinColor = 0;//肤色
	CvPoint imagePoint;
	int count=1;
	int x=0;
	int y=0;
	
	cvNamedWindow("原图", 0); 
	cvResizeWindow("原图",200,200);
	cvMoveWindow("原图",400,400);
	cvNamedWindow("肤色图", 0); 
	cvResizeWindow("肤色图",200,200);
	cvMoveWindow("肤色图",615,400); 
	cvNamedWindow("色彩平衡",0);
	cvResizeWindow("色彩平衡",200,200);
	cvNamedWindow("最终肤色二值图", 0);
	cvResizeWindow("最终肤色二值图",200,200);
	
	int nFrmNum=0;//视频帧数计数
	//逐帧读取视频,逐帧处理
	while(originalImage=cvQueryFrame(pCapture))
	{
		nFrmNum++;
		if(nFrmNum==1)//第一帧创建空间
		{
			imageBalanced=cvCreateImage(cvSize(originalImage->width,originalImage->height),IPL_DEPTH_8U,3);//建头并分配数据（size,depth,通道数）
			ycbcr=cvCreateImage(cvSize(originalImage->width,originalImage->height),IPL_DEPTH_8U,3);
			imageLikelihood=cvCreateImage(cvSize(originalImage->width,originalImage->height),IPL_DEPTH_8U,1);
	
		}
		cvShowImage("原图", originalImage );
		imageSkinColor=cvCreateImage(cvSize(originalImage->width,originalImage->height),IPL_DEPTH_8U,3);
		*imageBalanced=colorBalance(originalImage);//色彩平衡
		cvShowImage("色彩平衡",imageBalanced);//显示色彩平衡后的图
		//CV_MEDIAN：对图像进行核大小为param1×param1 的中值滤波（邻域必须是方的）。
		cvSmooth(imageBalanced, imageBalanced,CV_MEDIAN ,3,0,0,0 );//平滑 
		cvCvtColor(imageBalanced, ycbcr, CV_RGB2YCrCb);// 颜色空间转换rgb->YCrCb
		*imageLikelihood=skinColorLikelihood(ycbcr);//肤色处理	
		imagePoint=gravity(imageLikelihood);	
		//cout<<"重心坐标（"<<imagePoint.x<<","<<imagePoint.y<<")";
		/////////////////*下列三行是为了求面积*/
		CvMoments momentTemp;
		cvMoments(imageLikelihood,&momentTemp,1);//计算二值图像的矩信息
		double area=cvGetSpatialMoment(&momentTemp,0,0);//m00=区域的面积
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
		//////////////////////以上是判断动作的代码
		cvShowImage("最终肤色二值图",imageLikelihood );//处理后的肤色图显示		
		
		//显示肤色图
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
		cvShowImage("肤色图", originalImage );
		
		
		if(cvWaitKey(1)>=0)
		{
			break;
		}
	}
	//处理后收尾工作
	///*
	cvDestroyWindow( "原图" );//销毁窗口
	//cvDestroyWindow( "肤色图" );//销毁窗口
	cvDestroyWindow( "色彩平衡" );//销毁窗口
	cvReleaseImage( &imageBalanced ); //释放图像
	cvDestroyWindow( "肤色似然" );//销毁窗口
	cvReleaseImage( &imageLikelihood ); //释放图像
	cvDestroyWindow( "最终肤色二值图" );//销毁窗口
	//*/
}

//色彩平衡（gray world彩色均衡方法）
IplImage MainWindow::colorBalance( IplImage* imageBalanced)
{
	IplImage* dst;//色彩平衡
	IplImage* gray;//灰度图
	dst=cvCreateImage(cvSize(imageBalanced->width,imageBalanced->height),IPL_DEPTH_8U,3);
	gray=cvCreateImage(cvSize(imageBalanced->width,imageBalanced->height),IPL_DEPTH_8U,1);
	int height,width,step,channels;
	uchar *data,*dataDST;
	height = imageBalanced->height; 
	width = imageBalanced->width;//width是指每行的像素个数，可能一个像素占一个字节也可能是三四个字节
	step = imageBalanced->widthStep; //widthstep是行字节数,应该是4的倍数,不一定等于width
	channels = imageBalanced->nChannels;
	data = (uchar *)imageBalanced->imageData;
	dataDST = (uchar *)dst->imageData;
	int i,j;
	double R,G,B,Gy,aR,aG,aB;
	cvZero(dst);//cvZero(IplImage型图片):初始化图片,值都为0,矩阵大小为640*480
	CvMat* MR=cvCreateMat(height,width,CV_64FC1);//(rows,cols,type) CV_64FC1：64位/指针double
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
			cvmSet(MB,i,j,B);//set MB(i,j);值为B
			cvmSet(MG,i,j,G);
			cvmSet(MR,i,j,R);
			((gray->imageData + gray->widthStep*i))[j]=(B+G+R)/3;
		}
	}
	CvScalar argR,argG,argB;//cvScalar是一个数组结构体，有四个参数
	double argI;
	argR=cvAvg(MR,0);//计算数组中所有元素的平均值（arr,mask）当mask非空时，那么平均值仅有那些mask非0的元素相对应的像素算出
	argG=cvAvg(MG,0);
	argB=cvAvg(MB,0);
	argI=(argR.val[0]+argG.val[0]+argB.val[0])/3;//得到平均灰度值
	//算出因子
	aR=argI/argR.val[0];
	aG=argI/argG.val[0];
	aB=argI/argB.val[0];
	for(i=0;i<height;i++)
	{
		for(j=0;j<width;j++)
		{
			//CV_MAT_ELEM( matrix要访问的矩阵, elemtype矩阵元素类型, row, col )
			//CV_MAT_ELEM用来访问每个元素的宏，只对单通道有效
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

//肤色似然(肤色检测，算阈值，灰度图变成二值图，描轮廓)
IplImage MainWindow::skinColorLikelihood(IplImage* imageLikelihood)
{	
	IplImage* dst;//肤色图
	dst=cvCreateImage(cvSize(imageLikelihood->width,imageLikelihood->height),IPL_DEPTH_8U,1);
	int height,width,step,channels;
	uchar *data;
	height = imageLikelihood->height; 
	width = imageLikelihood->width; 
	step = imageLikelihood->widthStep; 
	channels = imageLikelihood->nChannels;
	data = (uchar *)imageLikelihood->imageData;
	double M[4]={0.0077 ,-0.0041,-0.0041 ,0.0047};//肤色的协方差矩阵求逆后(经验值)
	CvMat* max=cvCreateMat(height,width,CV_64FC1);//建一个和imageLikelihood一样格式的存储肤色概率密度矩阵（数组）
	CvMat* mat=cvCreateMat(2,2,CV_64FC1);//2行2列存储协方差矩阵（逆）
	CvMat* cbcr=cvCreateMat(1,2,CV_64FC1);//1行2列{cb,cr}
	CvMat* cbcrChanged=cvCreateMat(2,1,CV_64FC1);//2行1列{cb,cr}转换后
	CvMat* res=cvCreateMat(1,1,CV_64FC1);//1行1列
	cvInitMatHeader(mat,2,2,CV_64FC1,M);//M赋值给mat,mat=[0.0077,-0.0041][-0.0041,0.0047]
	double valueToRes[1]={0};
	cvInitMatHeader(res,1,1,CV_64FC1,valueToRes);//给res初始化=valueToRes[1](即0)
		
	int i,j;
	double r,b,cb,cr,u;
	//按照上面的公式进行肤色似然度计算
	for(i=0;i<height;i++)
	{
		for(j=0;j<width;j++)
		{	
			r=data[i*step+j*channels+2];//cr 分量
			b=data[i*step+j*channels+1];//cb 分量
			cb=b-103.0056;////117.4361
			cr=r-140.1309;////156.5599
			double p1[2]={cb,cr};
			cvInitMatHeader(cbcr,1,2,CV_64FC1,p1);//p1赋值给cbcr,cbcr={r-103.0056,b-140.1309}
			cvMatMulAdd(cbcr,mat,0,cbcr);//（src1,src2,src3,dst）dst=src1*src2+src3;即cbcr=cbcr*mat+0
			//cbcr={(r-103.0056)*0.0077-0.0041*(b-140.1309),-0.0041*(r-103.0056)+0.0047*(b-140.1309)}
			double p2[2]={cb,cr};
			cvInitMatHeader(cbcrChanged,2,1,CV_64FC1,p2);//(mat,rows,cols,type,data)data为可选的，将指向数据指针分配给矩阵头. 
			cvMatMulAdd(cbcr,cbcrChanged,0,res);//res=cbcr*cbcrChanged+0;
			u=CV_MAT_ELEM(*res,double,0,0);//u=res矩阵中（0,0）位置的值
			u=exp(-0.5*u);//e的-0.5*u次方 值影响区域
			cvmSet(max,i,j,u);//max(i,j)=u;给矩阵依次赋值
		}
	}
	double maxNumInMatrix=0; //初始化最大值
	double numZeroToOne=0;
	cvMinMaxLoc(max,NULL,&maxNumInMatrix,NULL,NULL);//将矩阵max中最大值赋值给maxNumInMatrix
	//肤色似然——换成0-255范围的数
	for(i=0;i<height;i++)
	{
		for(j=0;j<width;j++)
		{
			numZeroToOne=CV_MAT_ELEM(*max,double,i,j)/maxNumInMatrix;//将值变成0-1范围的值
			cvmSet(max,i,j,255*numZeroToOne);//max(i,j)=255*numZeroToOne;
			((dst->imageData + dst->widthStep*i))[j]=255*numZeroToOne;
		}
	}
	//cvNamedWindow("肤色dst", CV_WINDOW_AUTOSIZE); 
	cvNamedWindow("肤色dst",0);
	cvResizeWindow("肤色dst",200,200);
	cvMoveWindow("肤色dst",800,400);
	cvShowImage("肤色dst", dst );
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
	//最大类间方差法(OTSU法)计算图像二值化的自适应阈值(用来将灰度图像转化成二值图像)
	double g;//前景和背景图象的方差g
	double w0;//前景点数占图像比例为w0
	double w1;//背景点数占图像比例为w1
	double u0;//前景的平均灰度为u0
	double u1;//背景的平均灰度为u1
	double ut;//图像的总平均灰度为：ut=w0*u0+w1*u1。
	double Q[]={0,0,0,0,0,0,0,0,0,0};
	int flag=0;
	double max2;
	j=0;
	//大津法缺陷是当目标物和背景灰度不明显时，会出现大块黑色区域，甚至会丢失图像信息
	//在大津法的基础上把灰度拉伸，增强灰度级数，间隔是1的就是大津法
	/*
	for(int m=10;m<=190;m+=20)//间隔20
	{
		g=0;w0=0;w1=0;u0=0;u1=0;ut=0;//循环内置为0为了每次循环时都重新计算		
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
		//前景和背景图象的方差：g=w0*(u0-ut)*(u0-ut)+w1*(u1-ut)*(u1-ut)=w0*w1*(u0-u1)*(u0-u1),
		//当方差g最大时，可以认为此时前景和背景差异最大，也就是此时的灰度是最佳阈值
		g=w0*(u0-ut)*(u0-ut)+w1*(u1-ut)*(u1-ut);//方差
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
		g=0;w0=0;w1=0;u0=0;u1=0;ut=0;//循环内置为0为了每次循环时都重新计算		
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



	int thresholdValue;//阈值
	thresholdValue=20*j;//20为设的间隔的长度
	//用阈值来做二值图
	IplImage* imageBinary;//二值图	
	IplImage* imageBinaryProcessed;//用来画轮廓的二值图
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
				((imageBinary->imageData + imageBinary->widthStep*i))[j]=255;//置为全白
			}
		}
	}		
	CvMemStorage* storage = cvCreateMemStorage(0);//内存存储器，用来存轮廓
	CvSeq* contour = 0;//轮廓
	cvThreshold(imageBinary,imageBinary,128,255,CV_THRESH_BINARY);//对灰度图像进行阈值操作得到二值图像
	cvDilate( imageBinary, imageBinary, NULL, 1 );//膨胀
	/*
　　CV_RETR_CCOMP：检索所有的轮廓，并将他们组织为两层：顶层是各部分的外部边界，第二层是空洞的边界；
　　CV_CHAIN_APPROX_SIMPLE：压缩水平的、垂直的和斜的部分，也就是，函数只保留他们的终点部分。
	*/
	cvFindContours( imageBinary, storage, &contour, sizeof(CvContour), CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE );//二值图像中检测轮廓
	double contour_area = 0;	
	for(;contour!=0;contour=contour->h_next)
	{
		contour_area=100*fabs(cvContourArea(contour,CV_WHOLE_SEQ )); //fabs()求绝对值，cvContourArea()求轮廓面积
		if(contour_area>height*width)//((contour->v_next!=0)&&(contour_area>height*width))
		{
			cvDrawContours(imageBinaryProcessed,contour,CV_RGB(255,255,255),CV_RGB(255,255,255),0,CV_FILLED,8 );//在图像上绘制外部和内部轮廓
		}
	}
	return *imageBinaryProcessed;
}

//计算重心
CvPoint MainWindow::gravity(IplImage *imageLikelihood)//输入二值图像
{
	CvPoint center;
	double m00,m10,m01;
	CvMoments moment;
	cvMoments(imageLikelihood,&moment,1);//计算二值图像的矩信息
	m00=cvGetSpatialMoment(&moment,0,0);//m00=区域的面积
	m10=cvGetSpatialMoment(&moment,1,0);//获得指定维的矩信息
	m01=cvGetSpatialMoment(&moment,0,1);
	center.x =(int)(m10/m00);//算坐标
	center.y =(int)(m01/m00);
	return center;
}

//手控图片旋转
void MainWindow::controlImage2()
{
	IplImage* originalImage = 0; // 原图
	IplImage* imageBalanced = 0;//色彩平衡
	IplImage* ycbcr = 0;//ycbcr
	IplImage* imageLikelihood = 0;//肤色似然
	CvCapture* pCapture=NULL;
	pCapture=cvCreateCameraCapture(0);//设置摄像机
	cvSetCaptureProperty(pCapture,CV_CAP_PROP_FPS,1);
	cvSetCaptureProperty(pCapture,CV_CAP_PROP_FRAME_WIDTH,64);
	cvSetCaptureProperty(pCapture,CV_CAP_PROP_FRAME_HEIGHT,48);
	IplImage* imageSkinColor = 0;//肤色
	CvPoint imagePoint;
	int count=1;
	int x=0;
	int y=0;
	
	cvNamedWindow("原图", 0); 
	cvResizeWindow("原图",200,200);
	cvMoveWindow("原图",400,400);
	cvNamedWindow("肤色图", 0); 
	cvResizeWindow("肤色图",200,200);
	cvMoveWindow("肤色图",615,400); 
	cvNamedWindow("色彩平衡",0);
	cvResizeWindow("色彩平衡",200,200);
	cvNamedWindow("最终肤色二值图", 0);
	cvResizeWindow("最终肤色二值图",200,200);
	
	int nFrmNum=0;//视频帧数计数
	//逐帧读取视频,逐帧处理
	while(originalImage=cvQueryFrame(pCapture))
	{
		nFrmNum++;
		if(nFrmNum==1)//第一帧创建空间
		{
			imageBalanced=cvCreateImage(cvSize(originalImage->width,originalImage->height),IPL_DEPTH_8U,3);//建头并分配数据（size,depth,通道数）
			ycbcr=cvCreateImage(cvSize(originalImage->width,originalImage->height),IPL_DEPTH_8U,3);
			imageLikelihood=cvCreateImage(cvSize(originalImage->width,originalImage->height),IPL_DEPTH_8U,1);
	
		}
		cvShowImage("原图", originalImage );
		imageSkinColor=cvCreateImage(cvSize(originalImage->width,originalImage->height),IPL_DEPTH_8U,3);
		*imageBalanced=colorBalance(originalImage);//色彩平衡
		cvShowImage("色彩平衡",imageBalanced);//显示色彩平衡后的图
		//CV_MEDIAN：对图像进行核大小为param1×param1 的中值滤波（邻域必须是方的）。
		cvSmooth(imageBalanced, imageBalanced,CV_MEDIAN ,3,0,0,0 );//平滑 
		cvCvtColor(imageBalanced, ycbcr, CV_RGB2YCrCb);// 颜色空间转换rgb->YCrCb
		*imageLikelihood=skinColorLikelihood(ycbcr);//肤色处理	
		imagePoint=gravity(imageLikelihood);	
		//cout<<"重心坐标（"<<imagePoint.x<<","<<imagePoint.y<<")";
		/////////////////*下列三行是为了求面积*/
		CvMoments momentTemp;
		cvMoments(imageLikelihood,&momentTemp,1);//计算二值图像的矩信息
		double area=cvGetSpatialMoment(&momentTemp,0,0);//m00=区域的面积
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
		//////////////////////以上是判断动作的代码
		cvShowImage("最终肤色二值图",imageLikelihood );//处理后的肤色图显示		
		/*
		//显示肤色图
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
		cvShowImage("肤色图", originalImage );
		*/
		
		if(cvWaitKey(1)>=0)
		{
			break;
		}
	}
	//处理后收尾工作
	///*
	cvDestroyWindow( "原图" );//销毁窗口
	//cvDestroyWindow( "肤色图" );//销毁窗口
	cvDestroyWindow( "色彩平衡" );//销毁窗口
	cvReleaseImage( &imageBalanced ); //释放图像
	cvDestroyWindow( "肤色似然" );//销毁窗口
	cvReleaseImage( &imageLikelihood ); //释放图像
	cvDestroyWindow( "最终肤色二值图" );//销毁窗口
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