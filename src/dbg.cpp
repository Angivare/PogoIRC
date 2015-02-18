#include "dbg.h"

Dbg d;

Dbg::Dbg()// : m_index(0)
{
}

void Dbg::write(const QString& log) {
	const QString line = "# " + log;
	emit console(line);
}

Dbg& Dbg::operator<<(const QString& err) {
	emit log(err); //Status bar
	
	qDebug() << err; //Console output
	
	write(err);
	
	return *this;
}
Dbg& Dbg::operator<<(const int& err) {
	return operator<<(QString::number(err));
}

void Dbg::silent(const QString& err) {
	write(err);
}

void Dbg::silent(const QByteArray& err) {
	silent(QString(err));
}

void Dbg::silent(const char* err) {
	silent(QString(err));
}

void Dbg::silent(const int& err) {
	silent(QString::number(err));
}

void Dbg::crit(const QString& err) {
	emit msgBox(err); //Dialog box
	
	qDebug() << err; //Console output
	
	write(err);
}

void Dbg::html(const QString& page) {
	QFile f(QCoreApplication::applicationDirPath ()+"/data/log.html");
	f.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);
	f.write(page.toLocal8Bit()); //Log file
}

