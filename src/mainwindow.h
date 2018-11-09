#ifndef __MAIN_WINDOW_H__
#define __MAIN_WINDOW_H__

#include "Logger.h"
#include "VRWorker.h"
#include "XVideoWriter.h"

#include <QMainWindow>


QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
class QPlainTextEdit;
class QSessionManager;
QT_END_NAMESPACE

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
	void showEvent(QShowEvent* event);

private:
    Ui::MainWindow* ui;

	std::unique_ptr<Logger> logger;
	std::thread* pWatchdogThread;
	VRWorker* vr;
	XVideoWriter* vw;

	bool thread_worked = false;

	void WatchdogThreadFunction();
	void StartExpirement();
	void StopExpirement();
};

#endif // __MAIN_WINDOW_H__
