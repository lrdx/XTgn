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
		logger->write("Failed initialization");
		return;
	}
}

void MainWindow::WatchdogThreadFunction()
{
	int count = 0;

	boost::asio::serial_port serial(io);
	serial.open("COM4");
	serial.write_some(boost::asio::buffer("1"));
	while (count < 50)
	{
		vr->CopyScreenToBuffer();
		vw->WriteFrame(vr->GetBuffer(), vr->GetBufferRowCount(), vr->GetBufferRowPitch());

		++count;
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	logger->write("Write file..");
	vw->CloseFile();
}

void MainWindow::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);

	m_pWatchdogThread = new std::thread(&MainWindow::WatchdogThreadFunction, this);
}

MainWindow::~MainWindow()
{
    delete ui;
}
