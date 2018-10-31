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

private:
    Ui::MainWindow *ui;

	Logger* logger;
	VRWorker* vr;
	XVideoWriter* vw;
};

#endif // __MAIN_WINDOW_H__
