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
	vw->Initialize("libx264", "test_video.avi",
		AVPixelFormat::AV_PIX_FMT_RGBA, vr->GetHeight(), vr->GetWidth());

	if (!vr->IsInitialized() || !vw->IsInitialized())
	{
		logger->write("Failed initialized");
		return;
	}

	vr->CopyScreenToBuffer();
	auto buf = vr->GetBuffer();

	vw->WriteFrame(buf);

}

MainWindow::~MainWindow()
{
    delete ui;
}
