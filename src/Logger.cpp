#include "Logger.h"

#include <QDateTime>
#include <QTextStream>

Logger::Logger(QObject *parent, QString fileName, QPlainTextEdit *editor) : QObject(parent)
{
	m_editor = editor;
	m_showDate = true;
	if (!fileName.isEmpty())
	{
		m_file = new QFile;
		m_file->setFileName(fileName);
		m_file->open(QIODevice::Append | QIODevice::Text);
	}
}

void Logger::Write(const QString& value) 
{
	QString text = value;// + "";
	if (m_showDate)
		text = QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss ") + text;

	QTextStream out(m_file);
	out.setCodec("UTF-8");
	if (m_file != nullptr)
	{
		out << text;
	}
	if (m_editor != nullptr)
		m_editor->appendPlainText(text);

	m_editor->update();
}

void Logger::WriteInfo(const QString& value)
{
	Write("[Info] " + value);
}

void Logger::WriteDebug(const QString& value)
{
	Write("[Debug] " + value);
}

void Logger::WriteError(const QString& value)
{
	Write("[Error] " + value);
}

void Logger::SetShowDateTime(const bool value) 
{
	m_showDate = value;
}

Logger::~Logger() 
{
	if (m_file != nullptr)
		m_file->close();
}