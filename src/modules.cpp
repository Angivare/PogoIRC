#include "modules.h"

/*----------------------
 - base abstract class - 
 ---------------------*/
AngiServer::AngiServer(QObject* parent) : QLocalServer(parent), m_s(NULL)
{
}

void AngiServer::setup()
{
	listen(getServerName());
	connect(this, SIGNAL(newConnection()), this, SLOT(new_connection()));
}

void AngiServer::new_connection()
{
	d.silent("Incoming connection from server " + QString(getServerName()));
	if(m_s) m_s->abort();
	m_s = nextPendingConnection();
	
	connect(m_s, SIGNAL(disconnected()), this, SLOT(finish()));
	connect(m_s, SIGNAL(disconnected()), m_s, SLOT(deleteLater()));
	connect(m_s, SIGNAL(readyRead()), this, SLOT(ready_read()));
}

void AngiServer::ready_read()
{
	if(m_s->bytesAvailable() < (int)sizeof(quint16)) return;
	m_tmp += m_s->readAll();
}

void AngiServer::finish()
{
	emit request(m_tmp);
	onRequest(m_tmp);
	m_tmp = QString();
}

void AngiServer::onRequest(const QString&) {}

/*--------------
  - subclasses -
  ------------*/
Updater::Updater(QObject* parent) : AngiServer(parent) {}
const char* Updater::getServerName() { return "POGOIRC_UPDATER"; }

Factory::Factory(QObject* parent) : AngiServer(parent) {}
const char* Factory::getServerName() { return "POGOIRC_CODOFACTORY"; }
