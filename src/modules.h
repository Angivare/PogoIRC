#ifndef FACTORY_H
#define FACTORY_H

#include <QLocalServer>
#include <QLocalSocket>
#include <QString>

#include "dbg.h"

/*
  Handles incoming connection from a SINGLE other program
*/
class AngiServer : public QLocalServer
{ Q_OBJECT
public:
	explicit AngiServer(QObject* parent = 0);
	void setup();

signals:
	void request(QString);
	
protected:
	virtual const char* getServerName() = 0;
	virtual void onRequest(const QString&); //Called on outside message received
	
private slots:
	void new_connection();
	void ready_read();
	void finish();
	
private:
	QString m_tmp;
	QLocalSocket* m_s;
};

class Updater : public AngiServer
{ Q_OBJECT
public:
	explicit Updater(QObject* parent = 0);
	
protected:
	const char* getServerName();
};

class Factory : public AngiServer
{ Q_OBJECT
public:
	explicit Factory(QObject* parent = 0);
	
protected:
	const char* getServerName();
};

#endif // FACTORY_H
