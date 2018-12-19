#ifndef __MAIN_WINDOW_H__
#define __MAIN_WINDOW_H__

#include "Logger.h"
#include "VRWorker.h"
#include "XVideoWriter.h"
#include "SettingsHolder.h"

#include <QMainWindow>

QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
class QPlainTextEdit;
class QSessionManager;
QT_END_NAMESPACE

namespace boost {
	namespace asio {
		class serial_port;
	}
}

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
	void showEvent(QShowEvent* e) override;
	void closeEvent(QCloseEvent* e) override;

private:
    Ui::MainWindow* ui;
	std::unique_ptr<SettingsHolder> settings;

	std::vector<std::string> encoders;

	Logger* logger;
	VRWorker* vr;
	XVideoWriter* vw;

	std::unique_ptr<std::thread> pWatchdogThread;
	bool thread_worked = false;

	void StopThreadIfWorked();

public slots:
	void StartExpirement();
	void StopExpirement();
	void NewExpirement();
	void OpenSettingsWindow();
};

#endif // __MAIN_WINDOW_H__
