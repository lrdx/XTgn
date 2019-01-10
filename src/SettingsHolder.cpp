#include "SettingsHolder.h"

SettingsHolder::SettingsHolder()
{
	m_eye = vr::EVREye::Eye_Right;

	m_codec_name = "libx264";
	m_video_bitrate = 2500;
	m_video_width = 800;
	m_video_height = 600;
	m_video_framerate = 30;

	m_port_name = "COM1";
	m_port_rate = 9600;
	m_port_databits = 8;	
	m_port_parity = boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::none);
	m_port_stopbits = boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::one);

	m_gopro_sync_use = false;
	m_gopro_port = 7755;
}

SettingsHolder::SettingsHolder(const SettingsHolder& settings)
{
	//Initalize properties after load from QSettings
	SetVREye(settings.m_eye);
	SetCodecName(settings.m_codec_name);
	SetVideoBitrate(settings.m_video_bitrate);
	SetVideoWidth(settings.m_video_width);
	SetVideoHeight(settings.m_video_height);
	SetVideoFramerate(settings.m_video_framerate);
	SetPortName(settings.m_port_name);
	SetPortRate(settings.m_port_rate);
	SetPortDataBits(settings.m_port_databits);
	SetPortParity(settings.m_port_parity);
	SetPortStopbits(settings.m_port_stopbits);
	SetGoProSync(settings.m_gopro_sync_use);
	SetGoProPort(settings.m_gopro_port);
}

void SettingsHolder::Load(QSettings* settings)
{
	if (settings->value("eyeVR", "right").toString().compare("right", Qt::CaseInsensitive) == 0)
		m_eye = vr::EVREye::Eye_Right;
	else
		m_eye = vr::EVREye::Eye_Left;

	m_codec_name = settings->value("video_codec", "libx264").toString();
	m_video_bitrate = settings->value("video_bitrate", "2500").toInt();
	m_video_width = settings->value("video_width", "800").toInt();
	m_video_height = settings->value("video_height", "600").toInt();
	m_video_framerate = settings->value("video_framerate", "30").toInt();

	m_port_name = settings->value("port_name", "COM1").toString();
	m_port_rate = settings->value("port_rate", "9600").toInt();
	m_port_databits = settings->value("port_databits", "8").toInt();

	const auto parity = settings->value("port_parity", "none").toString();
	if (parity.compare("even", Qt::CaseInsensitive) == 0)
	{
		m_port_parity = boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::even);
	}
	else if (parity.compare("odd", Qt::CaseInsensitive) == 0)
	{
		m_port_parity = boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::odd);
	}
	else
	{
		m_port_parity = boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::none);
	}

	const auto stopbits = settings->value("port_stopbits", "one").toString();
	if (stopbits.compare("onepointfive") == 0)
	{
		m_port_stopbits = boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::onepointfive);
	}
	else if (stopbits.compare("two") == 0)
	{
		m_port_stopbits = boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::two);
	}
	else
	{
		m_port_stopbits = boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::one);
	}

	m_gopro_sync_use = settings->value("gopro_port_use", "true").toBool();
	m_gopro_port = settings->value("gopro_port", "7755").toInt();
}

void SettingsHolder::Save(QSettings* settings)
{
	switch (m_eye)
	{
	case vr::EVREye::Eye_Right:
		settings->setValue("eyeVR", "right");
		break;
	case vr::EVREye::Eye_Left:
		settings->setValue("eyeVR", "left");
		break;
	}

	settings->setValue("video_codec", m_codec_name);
	settings->setValue("video_bitrate", m_video_bitrate);
	settings->setValue("video_width", m_video_width);
	settings->setValue("video_height", m_video_height);
	settings->setValue("video_framerate", m_video_framerate);
	settings->setValue("port_name", m_port_name);
	settings->setValue("port_rate", m_port_rate);
	settings->setValue("port_databits", m_port_databits);

	switch (m_port_parity.value())
	{
	case boost::asio::serial_port_base::parity::even:
		settings->setValue("port_parity", "even");
		break;
	case boost::asio::serial_port_base::parity::odd:
		settings->setValue("port_parity", "odd");
		break;
	case boost::asio::serial_port_base::parity::none:
		settings->setValue("port_parity", "none");
		break;
	}

	switch (m_port_stopbits.value())
	{
	case boost::asio::serial_port_base::stop_bits::one:
		settings->setValue("port_stopbits", "one");
		break;
	case boost::asio::serial_port_base::stop_bits::onepointfive:
		settings->setValue("port_stopbits", "onepointfive");
		break;
	case boost::asio::serial_port_base::stop_bits::two:
		settings->setValue("port_stopbits", "two");
		break;
	}

	settings->setValue("gopro_port_use", m_gopro_sync_use);
	settings->setValue("gopro_port", m_gopro_port);

	settings->sync();
}

void SettingsHolder::SetVREye(vr::EVREye eye)
{
	m_eye = eye;
}

void SettingsHolder::SetCodecName(const QString& codecName)
{
	m_codec_name = codecName;
}

void SettingsHolder::SetVideoBitrate(int bitrate)
{
	if (bitrate <= 0)
		return;

	m_video_bitrate = bitrate;
}

void SettingsHolder::SetVideoWidth(int width)
{
	if (width <= 0)
		return;

	m_video_width = width;
}

void SettingsHolder::SetVideoHeight(int height)
{
	if (height <= 0)
		return;

	m_video_height = height;
}

void SettingsHolder::SetVideoFramerate(int framerate)
{
	if (framerate <= 0)
		return;

	m_video_framerate = framerate;
}

void SettingsHolder::SetPortName(const QString& port)
{
	m_port_name = port;
}

void SettingsHolder::SetPortRate(int rate)
{
	if (rate <= 0)
		return;

	m_port_rate = rate;
}

void SettingsHolder::SetPortDataBits(int databits)
{
	if (databits < 4 || databits > 8)
		return;

	m_port_databits = databits;
}

void SettingsHolder::SetPortParity(boost::asio::serial_port_base::parity parity)
{
	m_port_parity = parity;
}

void SettingsHolder::SetPortStopbits(boost::asio::serial_port_base::stop_bits stopbits)
{
	m_port_stopbits = stopbits;
}

void SettingsHolder::SetGoProSync(bool use)
{
	m_gopro_sync_use = use;
}

void SettingsHolder::SetGoProPort(int port)
{
	m_gopro_port = port;
}