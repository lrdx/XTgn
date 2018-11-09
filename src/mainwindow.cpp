#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <boost/asio.hpp>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

	setCentralWidget(ui->plainTextEdit);
	
	connect(ui->actionExit, &QAction::triggered, this, &QWidget::close);
	connect(ui->actionStart, &QAction::triggered, this, &MainWindow::StartExpirement);
	connect(ui->actionStop, &QAction::triggered, this, &MainWindow::StopExpirement);

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

void MainWindow::StartExpirement()
{
	thread_worked = true;
	pWatchdogThread = new std::thread(&MainWindow::WatchdogThreadFunction, this);
}

void MainWindow::StopExpirement()
{
	thread_worked = false;
}

void MainWindow::WatchdogThreadFunction()
{
	int count = 0;

	boost::asio::io_context io;
	auto serial_port = boost::asio::serial_port(io, "COM4");
	serial_port.write_some(boost::asio::buffer("1"));
	while (thread_worked)
	{
		vr->CopyScreenToBuffer();
		vw->WriteFrame(vr->GetBuffer(), vr->GetBufferRowCount(), vr->GetBufferRowPitch());

		++count;
		std::this_thread::sleep_for(std::chrono::milliseconds(33));
	}

	vw->CloseFile();
	serial_port.close();
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
