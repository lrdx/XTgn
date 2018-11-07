#ifndef __MAIN_WINDOW_H__
#define __MAIN_WINDOW_H__

#include "Logger.h"
#include "VRWorker.h"
#include "XVideoWriter.h"

#include <QMainWindow>

#include <boost/asio.hpp>

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

	boost::asio::io_context io;
	std::thread* m_pWatchdogThread;
	std::unique_ptr<Logger> logger;
	VRWorker* vr;
	XVideoWriter* vw;

	void WatchdogThreadFunction();
};

#endif // __MAIN_WINDOW_H__
