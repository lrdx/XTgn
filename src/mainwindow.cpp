#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

	setCentralWidget(ui->plainTextEdit);
	
	connect(ui->actionExit, &QAction::triggered, this, &QWidget::close);

	logger = std::make_unique<Logger>(this, "log.txt", ui->plainTextEdit);

	vr = new VRWorker(logger.get());
	vr->Initalize();

	vw = new XVideoWriter(logger.get());
	vw->Initialize("libx264", "test_video.avi");

	if (!vr->IsInitialized() || !vw->IsInitialized())
	{
		logger->write("Failed initialized");
		return;
	}


	uint8_t* buffer = new uint8_t()
	if (!vw->GetFrameBuffer(buffer))
	{
		logger->write("Failed get buffer");
		return;
	}

	vr->CopyScreenToBuffer(buffer);
}

MainWindow::~MainWindow()
{
    delete ui;
}
