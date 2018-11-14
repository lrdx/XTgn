#ifndef __SETTINGS_WINDOW_H__
#define __SETTINGS_WINDOW_H__

#include <QDialog>
#include <QSettings>

namespace Ui {
	class SettingsDialog;
}

class SettingsDialog : public QDialog
{
	Q_OBJECT

public:
	explicit SettingsDialog(QWidget* parent) = delete;
	explicit SettingsDialog(QSettings* set, QWidget *parent);
	~SettingsDialog();

private:
	Ui::SettingsDialog* ui;
	QSettings* settings;
};

#endif	//__SETTINGS_WINDOW_H__