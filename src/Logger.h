#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <QPlainTextEdit>
#include <QFile>
#include <QTextStream>

class Logger : public QObject
{
	Q_OBJECT
public:
	explicit Logger(QObject *parent, QString fileName, QPlainTextEdit *editor = nullptr);
	~Logger();
	void SetShowDateTime(bool value);

private:
	QFile* file;
	QPlainTextEdit *m_editor;
	bool m_showDate;

signals:

public slots:
	void WriteError(const QString& value);
	void WriteDebug(const QString& value);
	void WriteInfo(const QString& value);

private slots:
	void Write(const QString &value);

};

#endif //__LOGGER_H__