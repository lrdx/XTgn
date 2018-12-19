#ifndef __SETTINGS_HOLDER__
#define __SETTINGS_HOLDER__

#include "openvr.h"
#include "boost/asio.hpp"

#include <QSettings>

class SettingsHolder
{
public:
	SettingsHolder();
	SettingsHolder(const SettingsHolder& settings);

	//General
	vr::EVREye GetVREye() const { return m_eye; }
	void SetVREye(vr::EVREye eye);

	//Video
	QString GetCodecName() const { return m_codec_name; }
	void SetCodecName(const QString& codecName);

	int GetVideoBitrate() const { return m_video_bitrate; }
	void SetVideoBitrate(int bitrate);

	int GetVideoWidth() const { return m_video_width; }
	void SetVideoWidth(int width);

	int GetVideoHeight() const { return m_video_height; }
	void SetVideoHeight(int height);

	int GetVideoFramerate() const { return m_video_framerate; }
	void SetVideoFramerate(int framerate);

	//Serial port
	QString GetPortName() const { return m_port_name; }
	void SetPortName(const QString& port);

	int GetPortRate() const { return m_port_rate; }
	void SetPortRate(int rate);

	int GetPortDataBits() const { return m_port_databits; }
	void SetPortDataBits(int databits);

	boost::asio::serial_port_base::parity GetPortParity() const { return m_port_parity; }
	void SetPortParity(boost::asio::serial_port_base::parity parity);

	boost::asio::serial_port_base::stop_bits GetPortStopbits() const { return m_port_stopbits; }
	void  SetPortStopbits(boost::asio::serial_port_base::stop_bits stopbits);

	void Save(QSettings* settings);
	void Load(QSettings* settings);
private:
	vr::EVREye m_eye;
	QString m_codec_name;
	int m_video_bitrate;
	int m_video_width;
	int m_video_height;
	int m_video_framerate;
	QString m_port_name;
	int m_port_rate;
	int m_port_databits;
	boost::asio::serial_port_base::parity m_port_parity;
	boost::asio::serial_port_base::stop_bits m_port_stopbits;
};

#endif	//__SETTINGS_HOLDER__