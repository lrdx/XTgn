#include "StressMonitor.h"

#include <QNetworkAccessManager>
#include <QJsonDocument>
#include <QJsonObject>

StressMonitor::StressMonitor(QString username, QString person)
{
	m_username = username;
	m_person = person;
}

bool StressMonitor::SendMark(const QString& date_begin, const QString& date_end, const int context)
{
	QNetworkAccessManager manager;

	QJsonObject json_object;
	json_object.insert("date_begin", "08.02.2016 16:55:00.000000");
	json_object.insert("date_end", "08.02.2016 16:56:00.000000");
	json_object.insert("num", 1);
	json_object.insert("notes", "");
	json_object.insert("username", "lrdx");
	json_object.insert("personnick", "stefan-08-02");

	QNetworkRequest request(QUrl("http://stressmonitor.ru/api/marker/"));
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

	manager.post(request, QJsonDocument(json_object).toJson());


	return true;
}