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

	//logger = new Logger(this, "log.txt", ui->plainTextEdit);
	logger = new Logger(this, "log.txt", nullptr);
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

	if (!vr->IsInitialized())
	{
		logger->write("VR failed initialization");
		return;
	}

	const auto filename = QFileDialog::getSaveFileName(this, "Save file", QDir::currentPath());

	if(filename.isEmpty())
	{
		logger->write("Error open file to save..");
		return;
	}

	vw->Initialize(filename.toStdString(), settings->value("video_codec", "libx264").toString().toStdString(),
		settings->value("video_bitrate", 1000).toInt() * 1000,	//kb/s -> b/s
		settings->value("video_width", 800).toInt(),
		settings->value("video_height", 600).toInt(),
		settings->value("video_framerate", 24).toInt(),
		vr->GetFormat(),
		vr->GetWidth(),
		vr->GetHeight());

	if (!vw->IsInitialized())
	{
		logger->write("VideoWriter failed initialization");
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

	if(thread_worked)
	{
		logger->write("Thread is working. Waiting shutdown...");
		thread_worked = false;
		pWatchdogThread->join();
	}

	thread_worked = true;

	pWatchdogThread.reset(std::make_unique<std::thread>([&]()
	{
		const auto framerate = settings->value("video_framerate", "24").toInt();
		boost::asio::io_context io;
		auto serial_port = boost::asio::serial_port(io, settings->value("port_name", "COM4").toString().toStdString());
		serial_port.set_option(boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::none));
		serial_port.set_option(boost::asio::serial_port_base::baud_rate(9600));
		serial_port.set_option(boost::asio::serial_port_base::character_size(8));
		serial_port.set_option(boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::one));

		serial_port.write_some(boost::asio::buffer("1"));
		while (thread_worked)
		{
			auto startTime = std::chrono::high_resolution_clock::now();
			vr->CopyScreenToBuffer();
			auto endTime = std::chrono::high_resolution_clock::now();
			auto lastedTime1 = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
			startTime = std::chrono::high_resolution_clock::now();
			vw->WriteFrame(vr->GetBuffer(), vr->GetBufferRowCount(), vr->GetBufferRowPitch());
			endTime = std::chrono::high_resolution_clock::now();
			auto lastedTime2 = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
			//const auto ll = QString("Copy screen to buffer lasted %ll ms").arg(lastedTime);
			logger->write(QString("Copy screen to buffer lasted %1 ms and %2 ms\r\n").arg(lastedTime1).arg(lastedTime2));
			//const auto tt = static_cast<int>(std::round(1000 / framerate - lastedTime));
			const auto sleepTime = std::chrono::milliseconds(static_cast<int>(std::round(1000 / framerate)));
			//logger->write(QString("Sleep on %1 ms\r\n").arg(qulonglong(sleepTime.count())));

			std::this_thread::sleep_for(sleepTime);
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
