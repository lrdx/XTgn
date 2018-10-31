#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

	setCentralWidget(ui->plainTextEdit);
	
	connect(ui->actionExit, &QAction::triggered, this, &QWidget::close);

	logger = new Logger(this, "log.txt", ui->plainTextEdit);

	vr = new VRWorker(logger);
	vr->Init();
}

MainWindow::~MainWindow()
{
    delete ui;
	delete vr;
	delete logger;
}
