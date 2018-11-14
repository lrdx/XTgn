#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "SettingsWindow.h"

#include <boost/asio.hpp>

#include <chrono>

#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

	setCentralWidget(ui->plainTextEdit);

	settings = new QSettings("xtgn.ini", QSettings::Format::IniFormat, this);
	
	connect(ui->actionExit, &QAction::triggered, this, &QWidget::close);
	connect(ui->actionStart, &QAction::triggered, this, &MainWindow::StartExpirement);
	connect(ui->actionStop, &QAction::triggered, this, &MainWindow::StopExpirement);
	connect(ui->actionNew_Expirement, &QAction::triggered, this, &MainWindow::NewExpirement);
	connect(ui->actionSettings, &QAction::triggered, this, &MainWindow::OpenSettingsWindow);


	encoders = XVideoWriter::GetAllEncoders();

	logger = new Logger(this, "log.txt", ui->plainTextEdit);
	vr = new VRWorker(logger);
	vw = new XVideoWriter(logger);
}

void MainWindow::OpenSettingsWindow()
{
	const auto settings_copy = new QSettings(settings);
	auto dlg = new SettingsDialog(settings_copy, this);
	if(dlg->exec() == QDialog::Accepted)
	{
		settings = settings_copy;
	}
}

void MainWindow::NewExpirement()
{
	vr->Initalize(settings->value("eyeVR", "right").toString().compare("right", Qt::CaseInsensitive) == 0);

	const auto filename = QFileDialog::getSaveFileName(this, "Save file", QDir::currentPath());

	if(filename.isEmpty())
	{
		logger->write("Error open file to save..");
		return;
	}

	vw->Initialize(filename.toStdString(), settings->value("video_codec", "libx264").toString().toStdString(),
		settings->value("video_bitrate", 1000).toInt(),
		settings->value("video_width", 800).toInt(),
		settings->value("video_height", 600).toInt(),
		settings->value("video_framerate", 24).toInt(),
		vr->GetFormat(),
		vr->GetWidth(),
		vr->GetHeight());

	if (!vr->IsInitialized() || !vw->IsInitialized())
	{
		logger->write("Failed initialization");
		return;
	}
}

void MainWindow::StartExpirement()
{
	if (!vr->IsInitialized() || !vw->IsInitialized())
	{
		logger->write("VR or videowriter uninitialized");
		return;
	}

	thread_worked = true;

	pWatchdogThread.reset(std::make_unique<std::thread>([&]()
	{
		const auto framerate = settings->value("video_framerate", "24").toInt();
		boost::asio::io_context io;
		auto serial_port = boost::asio::serial_port(io, settings->value("port_name", "COM1").toString().toStdString());
		serial_port.set_option(boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::none));
		serial_port.set_option(boost::asio::serial_port_base::baud_rate(9600));
		serial_port.set_option(boost::asio::serial_port_base::character_size(8));
		serial_port.set_option(boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::one));

		serial_port.write_some(boost::asio::buffer("1"));
		while (thread_worked)
		{
			const auto startTime = std::chrono::high_resolution_clock::now();
			vr->CopyScreenToBuffer();
			vw->WriteFrame(vr->GetBuffer(), vr->GetBufferRowCount(), vr->GetBufferRowPitch());
			const auto endTime = std::chrono::high_resolution_clock::now();
			const auto lastedTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
			logger->write(QString("Copy screen to buffer lasted %d ms").arg(lastedTime));
			std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(std::round(1000 / framerate - lastedTime))));
		}

		vw->CloseFile();
		serial_port.close();
		logger->write("Write file..");
	}).release());
}

void MainWindow::StopExpirement()
{
	thread_worked = false;
}

void MainWindow::showEvent(QShowEvent* e)
{
	QWidget::showEvent(e);
}

void MainWindow::closeEvent(QCloseEvent* e)
{
	if(thread_worked)
	{
		thread_worked = false;
		pWatchdogThread->join();
	}

	QWidget::closeEvent(e);
}

MainWindow::~MainWindow()
{
	delete vr;
	delete vw;
	delete logger;
    delete ui;
}
