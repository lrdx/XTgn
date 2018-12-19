#ifndef __SETTINGS_WINDOW_H__
#define __SETTINGS_WINDOW_H__

#include "SettingsHolder.h"

#include <QDialog>

namespace Ui {
	class SettingsDialog;
}

class SettingsDialog : public QDialog
{
	Q_OBJECT

public:
	explicit SettingsDialog(QWidget* parent) = delete;
	explicit SettingsDialog(SettingsHolder* set, QWidget *parent);
	~SettingsDialog();

private:
	Ui::SettingsDialog* ui;
	SettingsHolder* settings;
};

#endif	//__SETTINGS_WINDOW_H__