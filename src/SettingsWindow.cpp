#include "SettingsWindow.h"
#include "XVideoWriter.h"

#include "ui_settingswindow.h"

#include <boost/asio.hpp>

SettingsDialog::SettingsDialog(QSettings* set, QWidget* parent)
	: QDialog(parent),
	ui(new Ui::SettingsDialog)
{
	settings = set;

	ui->setupUi(this);

	connect(ui->eyeButtonGroup, QOverload<QAbstractButton*, bool>::of(&QButtonGroup::buttonToggled),
		[=](QAbstractButton* button, bool checked)
	{
		if (checked)
		{
			settings->setValue("eyeVR", button == ui->rightEyeRadioButton ? "right" : "left");
		}
	});

	const auto lastCodec = settings->value("video_codec", "").toString();
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

	connect(ui->codecListWidget, &QListWidget::itemSelectionChanged,
		[=]()
	{
		settings->setValue("video_codec", ui->codecListWidget->selectedItems().at(0)->text());
	});

	ui->videoBitrateSpinBox->setValue(settings->value("video_bitrate", "1000").toInt());
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

	ui->videoFramerateSpinBox->setValue(settings->value("video_framerate", "24").toInt());
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
	for (int i = 4; i < 9; ++i)
	{
		ui->portDataBitsComboBox->addItem(QString(i), i);
		if (lastDataBits == i)
		{
			ui->portDataBitsComboBox->setCurrentIndex(i - 4);
		}
	}
	connect(ui->portDataBitsComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), 
		[=](const int index)
	{
		settings->setValue("port_databits", index);
	});

	ui->portParityComboBox->addItem("none", boost::asio::serial_port_base::parity::none);
	ui->portParityComboBox->addItem("odd", boost::asio::serial_port_base::parity::odd);
	ui->portParityComboBox->addItem("even", boost::asio::serial_port_base::parity::even);
	ui->portParityComboBox->setCurrentIndex(ui->portParityComboBox->findText(settings->value("port_parity", "none").toString(), Qt::MatchFixedString));
	connect(ui->portParityComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
		[=](const int index)
	{
		settings->setValue("port_parity", ui->portParityComboBox->itemData(index).toString());
	});

	ui->portStopBitsComboBox->addItem("one", boost::asio::serial_port_base::stop_bits::one);
	ui->portStopBitsComboBox->addItem("onepointfive", boost::asio::serial_port_base::stop_bits::onepointfive);
	ui->portStopBitsComboBox->addItem("two", boost::asio::serial_port_base::stop_bits::two);
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