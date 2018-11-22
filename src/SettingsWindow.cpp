#include "SettingsWindow.h"
#include "XVideoWriter.h"

#include "ui_settingswindow.h"

SettingsDialog::SettingsDialog(QSettings* set, QWidget* parent)
	: QDialog(parent),
	ui(new Ui::SettingsDialog)
{
	settings = set;

	ui->setupUi(this);
	if(settings->value("eyeVR", "right").toString().compare("right", Qt::CaseInsensitive) == 0)
		ui->rightEyeRadioButton->setChecked(true);
	else
		ui->leftEyeRadioButton->setChecked(true);

	connect(ui->eyeButtonGroup, QOverload<QAbstractButton*, bool>::of(&QButtonGroup::buttonToggled),
		[=](QAbstractButton* button, bool checked)
	{
		if (checked)
		{
			settings->setValue("eyeVR", button == ui->rightEyeRadioButton ? "right" : "left");
		}
	});

	const auto lastCodec = settings->value("video_codec", "libx264").toString();
	const auto codecs = XVideoWriter::GetAllEncoders();
	for (const auto& codec : codecs)
	{
		ui->codecListWidget->addItem(codec.c_str());
		if (ui->codecListWidget->selectedItems().isEmpty() && !lastCodec.isEmpty() && lastCodec.compare(codec.c_str(), Qt::CaseInsensitive) == 0)
		{
			ui->codecListWidget->setItemSelected(ui->codecListWidget->item(ui->codecListWidget->count() - 1), true);
		}
	}
	if (ui->codecListWidget->selectedItems().isEmpty() && ui->codecListWidget->count() != 0)
	{
		ui->codecListWidget->setItemSelected(ui->codecListWidget->item(1), true);
	}
	ui->codecListWidget->scrollToItem(ui->codecListWidget->selectedItems().at(0), QAbstractItemView::PositionAtCenter);

	connect(ui->codecListWidget, &QListWidget::itemSelectionChanged,
		[=]()
	{
		settings->setValue("video_codec", ui->codecListWidget->selectedItems().at(0)->text());
	});

	ui->videoBitrateSpinBox->setValue(settings->value("video_bitrate", "2500").toInt());
	connect(ui->videoBitrateSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
		[=](const int i)
	{
		settings->setValue("video_bitrate", i);
	});

	ui->videoWidthSpinBox->setValue(settings->value("video_width", "800").toInt());
	connect(ui->videoWidthSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
		[=](const int i)
	{
		settings->setValue("video_width", i);
	});

	ui->videoHeightSpinBox->setValue(settings->value("video_height", "600").toInt());
	connect(ui->videoHeightSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
		[=](const int i)
	{
		settings->setValue("video_height", i);
	});

	ui->videoFramerateSpinBox->setValue(settings->value("video_framerate", "30").toInt());
	connect(ui->videoWidthSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
		[=](const int i)
	{
		settings->setValue("video_framerate", i);
	});

	ui->portLineEdit->setText(settings->value("port_name", "COM1").toString());
	connect(ui->portLineEdit, &QLineEdit::textChanged,
		[=](const QString& text)
	{
		settings->setValue("port_name", text);
	});

	ui->portRateSpinBox->setValue(settings->value("port_rate", "9600").toInt());
	connect(ui->portRateSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
		[=](const int i)
	{
		settings->setValue("port_rate", i);
	});

	const auto lastDataBits = settings->value("port_databits", "8").toInt();
	ui->portDataBitsComboBox->addItem("4", 4);
	ui->portDataBitsComboBox->addItem("5", 5);
	ui->portDataBitsComboBox->addItem("6", 6);
	ui->portDataBitsComboBox->addItem("7", 7);
	ui->portDataBitsComboBox->addItem("8", 8);
	ui->portDataBitsComboBox->setCurrentIndex(lastDataBits - 4);

	connect(ui->portDataBitsComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), 
		[=](const int index)
	{
		settings->setValue("port_databits", index);
	});

	ui->portParityComboBox->addItem("none");
	ui->portParityComboBox->addItem("odd");
	ui->portParityComboBox->addItem("even");
	ui->portParityComboBox->setCurrentIndex(ui->portParityComboBox->findText(settings->value("port_parity", "none").toString(), Qt::MatchFixedString));
	connect(ui->portParityComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
		[=](const int index)
	{
		settings->setValue("port_parity", ui->portParityComboBox->itemData(index).toString());
	});

	ui->portStopBitsComboBox->addItem("one");
	ui->portStopBitsComboBox->addItem("onepointfive");
	ui->portStopBitsComboBox->addItem("two");
	ui->portStopBitsComboBox->setCurrentIndex(ui->portStopBitsComboBox->findText(settings->value("port_stopbits", "one").toString(), Qt::MatchFixedString));
	connect(ui->portStopBitsComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
		[=](const int index)
	{
		settings->setValue("port_stopbits", ui->portStopBitsComboBox->itemData(index).toString());
	});
}

SettingsDialog::~SettingsDialog()
{
	delete ui;
}