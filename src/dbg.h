#ifndef DBG_H
#define DBG_H

#include <QString>
#include <QObject>
#include <QDebug>
#include <QFile>
#include <QCoreApplication>
#include <iostream>
#include <QPlainTextEdit>

#include "config.h"

/*--------------------------------------------
 * DBG CLASS: utility class that allows output
 * operations to log errors of different levels
 *------------------------------------------*/

class Dbg : public QObject
{
	Q_OBJECT
public:
	Dbg();
	Dbg& operator<<(const QString&);
	Dbg& operator<<(const int&);
	
public slots:
	void silent(const QString&);
	void silent(const QByteArray&);
	void silent(const char*);
	void silent(const int&);
	void crit(const QString&);
	void html(const QString&);
	
signals:
	void console(const QString&);
	void log(const QString&);
	void msgBox(const QString&);
	
private:
	void write(const QString&);
	
//	long long unsigned int m_index;
//	QString m_log;
};

extern Dbg d;

#endif // DBG_H
