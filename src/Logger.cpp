#include "Logger.h"

#include <QDateTime>
#include <QTextStream>

Logger::Logger(QObject *parent, QString fileName, QPlainTextEdit *editor) : QObject(parent)
{
	m_editor = editor;
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
	text = QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss:ms ") + text;

	QTextStream out(m_file);
	out.setCodec("UTF-8");
	if (m_file != nullptr)
	{
		out << text;
	}

	if (m_editor != nullptr)
	{
		QMetaObject::invokeMethod(m_editor, "appendPlainText", Q_ARG(QString, text));
	}

	QMetaObject::invokeMethod(m_editor, "update");
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

Logger::~Logger() 
{
	if (m_file != nullptr)
		m_file->close();
}