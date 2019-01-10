#include "SettingsWindow.h"
#include "XVideoWriter.h"

#include "ui_settingswindow.h"

Q_DECLARE_METATYPE(boost::asio::serial_port_base::parity)
Q_DECLARE_METATYPE(boost::asio::serial_port_base::stop_bits)

SettingsDialog::SettingsDialog(SettingsHolder* set, QWidget* parent)
	: QDialog(parent),
	ui(new Ui::SettingsDialog)
{
	settings = set;

	ui->setupUi(this);

	if(settings->GetVREye() == vr::Eye_Right)
		ui->rightEyeRadioButton->setChecked(true);
	else
		ui->leftEyeRadioButton->setChecked(true);

	connect(ui->eyeButtonGroup, QOverload<QAbstractButton*, bool>::of(&QButtonGroup::buttonToggled),
		[=](QAbstractButton* button, const bool checked)
	{
		if (checked)
		{
			settings->SetVREye(button == ui->rightEyeRadioButton ? vr::Eye_Right : vr::Eye_Left);
		}
	});

	const auto lastCodec = settings->GetCodecName();
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
		settings->SetCodecName(ui->codecListWidget->selectedItems().at(0)->text());
	});

	ui->videoBitrateSpinBox->setValue(settings->GetVideoBitrate());
	connect(ui->videoBitrateSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
		[=](const int i)
	{
		settings->SetVideoBitrate(i);
	});

	ui->videoWidthSpinBox->setValue(settings->GetVideoWidth());
	connect(ui->videoWidthSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
		[=](const int i)
	{
		settings->SetVideoWidth(i);
	});

	ui->videoHeightSpinBox->setValue(settings->GetVideoHeight());
	connect(ui->videoHeightSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
		[=](const int i)
	{
		settings->SetVideoHeight(i);
	});

	ui->videoFramerateSpinBox->setValue(settings->GetVideoFramerate());
	connect(ui->videoWidthSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
		[=](const int i)
	{
		settings->SetVideoFramerate(i);
	});

	ui->portLineEdit->setText(settings->GetPortName());
	connect(ui->portLineEdit, &QLineEdit::textChanged,
		[=](const QString& text)
	{
		settings->SetPortName(text);
	});

	ui->portRateSpinBox->setValue(settings->GetPortRate());
	connect(ui->portRateSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
		[=](const int i)
	{
		settings->SetPortRate(i);
	});

	const auto lastDataBits = settings->GetPortDataBits();
	ui->portDataBitsComboBox->addItem("4", 4);
	ui->portDataBitsComboBox->addItem("5", 5);
	ui->portDataBitsComboBox->addItem("6", 6);
	ui->portDataBitsComboBox->addItem("7", 7);
	ui->portDataBitsComboBox->addItem("8", 8);
	ui->portDataBitsComboBox->setCurrentIndex(lastDataBits - 4);

	connect(ui->portDataBitsComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), 
		[=](const int index)
	{
		settings->SetPortDataBits(index);
	});

	ui->portParityComboBox->addItem("none", boost::asio::serial_port_base::parity::none);
	ui->portParityComboBox->addItem("odd", boost::asio::serial_port_base::parity::odd);
	ui->portParityComboBox->addItem("even", boost::asio::serial_port_base::parity::even);
	ui->portParityComboBox->setCurrentIndex(ui->portParityComboBox->findData(settings->GetPortParity().value()));
	connect(ui->portParityComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
		[=](const int index)
	{
		settings->SetPortParity(ui->portParityComboBox->itemData(index).value<boost::asio::serial_port_base::parity>());
	});

	ui->portStopBitsComboBox->addItem("one", boost::asio::serial_port_base::stop_bits::one);
	ui->portStopBitsComboBox->addItem("onepointfive", boost::asio::serial_port_base::stop_bits::onepointfive);
	ui->portStopBitsComboBox->addItem("two", boost::asio::serial_port_base::stop_bits::two);
	ui->portStopBitsComboBox->setCurrentIndex(ui->portStopBitsComboBox->findData(settings->GetPortStopbits().value()));
	connect(ui->portStopBitsComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
		[=](const int index)
	{
		settings->SetPortStopbits(ui->portStopBitsComboBox->itemData(index).value<boost::asio::serial_port_base::stop_bits>());
	});

	ui->goProGroupBox->setChecked(settings->GetGoProSync());
	connect(ui->goProGroupBox, &QGroupBox::toggled, 
		[=](const bool on)
	{
		settings->SetGoProSync(on);
	});

	ui->udpPortSpinBox->setValue(settings->GetGoProPort());
	connect(ui->udpPortSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
		[=](const int i)
	{
		settings->SetGoProPort(i);
	});
}

SettingsDialog::~SettingsDialog()
{
	delete ui;
}