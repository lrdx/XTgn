#ifndef __STRESS_MONITOR_H__
#define __STRESS_MONITOR_H__

#include <QString>

class StressMonitor
{
public:
	StressMonitor(QString username, QString person);

	bool SendMark(const QString& date_begin, const QString& date_end, int context);

private:
	QString m_username;
	QString m_person;
};

#endif	//__STRESS_MONITOR_H__