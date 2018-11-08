#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

	setCentralWidget(ui->plainTextEdit);
	
	connect(ui->actionExit, &QAction::triggered, this, &QWidget::close);
	connect(ui->actionStart, &QAction::triggered, this, &MainWindow::StartWrite);
	connect(ui->actionStop, &QAction::triggered, this, &MainWindow::StartWrite);

	logger = std::make_unique<Logger>(this, "log.txt", ui->plainTextEdit);

	boost::asio::io_context io;
	serial_port = std::make_unique<boost::asio::serial_port>(io, "COM4");

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

void MainWindow::StartWrite()
{
	thread_worked = true;
	m_pWatchdogThread = new std::thread(&MainWindow::WatchdogThreadFunction, this);
}

void MainWindow::StopWrite()
{
	thread_worked = false;
}

void MainWindow::WatchdogThreadFunction()
{
	int count = 0;

	serial_port->open("COM4");
	serial_port->write_some(boost::asio::buffer("1"));
	while (thread_worked)
	{
		vr->CopyScreenToBuffer();
		vw->WriteFrame(vr->GetBuffer(), vr->GetBufferRowCount(), vr->GetBufferRowPitch());

		++count;
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	vw->CloseFile();
	logger->write("Write file..");
}

void MainWindow::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);
}

MainWindow::~MainWindow()
{
    delete ui;
}
