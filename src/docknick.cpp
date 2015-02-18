#include "docknick.h"

DockNick::DockNick(QWidget* parent) : QDockWidget(parent)
{
	setObjectName("DockNick");
	setWindowTitle("Pseudos");
	setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable);
	//Initialize & create GUI
	m_wrapper = new QWidget(this);
	m_layout = new  QHBoxLayout(m_wrapper);
	m_nickList = new QListWidget(m_wrapper);
	m_passList = new QListWidget(m_wrapper);
	setWidget(m_wrapper);
	m_layout->addWidget(m_nickList);
	m_layout->addWidget(m_passList);
	m_layout->setContentsMargins(0,0,0,0);
	
	m_nickList->setDragDropMode(QAbstractItemView::InternalMove);
	m_passList->setDragDropMode(QAbstractItemView::InternalMove);
	
	//Add credential lists
	m_nickList->addItems(Config().get("credentials/nickAlias").toStringList());
	m_passList->addItems(Config().get("credentials/passAlias").toStringList());
	
	//Set credential data
	QStringList nickData = Config().get("credentials/nickData").toStringList();
	for(int i(0); i < m_nickList->count(); i++)
		m_nickList->item(i)->setData(32, nickData.at(i));
	QStringList passData = Config().get("credentials/passData").toStringList();
	for(int i(0); i < m_passList->count(); i++)
		m_passList->item(i)->setData(32, passData.at(i));
	
	//Set default credentials
	QString nick = Config().get("credentials/defaultNick",QString()).toString();
	if(nick != QString()) {
		if(m_nickList->findItems(nick, Qt::MatchExactly).isEmpty())
			m_nickList->addItem(nick);
		m_nickList->setCurrentItem(m_nickList->findItems(nick,Qt::MatchExactly).first());
	}
	QString pass = Config().get("credentials/defaultPass",QString()).toString();
	if(pass != QString()) {
		if(m_passList->findItems(pass, Qt::MatchExactly).isEmpty())
			m_passList->addItem(pass);
		m_passList->setCurrentItem(m_passList->findItems(pass,Qt::MatchExactly).first());
	}
	
	//Connect deleting signals
	connect(m_nickList, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(delNick(QListWidgetItem*)));
	connect(m_passList, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(delPass(QListWidgetItem*)));
}


void DockNick::addNick(QString alias, QString data) {
	m_nickList->addItem(alias);
	m_nickList->item(m_nickList->count()-1)->setData(32, data);
	saveNick();
}

void DockNick::addNick(QString data) {
	QString alias;
	int i = data.indexOf(':');
	if(i == -1) alias = data;
	else alias = data.left(i);
	
	addNick(alias, data);
}

void DockNick::addPass(QString alias, QString data) {
	m_passList->addItem(alias);
	m_passList->item(m_passList->count()-1)->setData(32, data);
	savePass();
}

void DockNick::saveNick() {
	QStringList aliasList, dataList;
	for(int i(0); i < m_nickList->count(); ++i) {
		aliasList += m_nickList->item(i)->text();
		dataList += m_nickList->item(i)->data(32).toString();	
	}
	Config().set("credentials/nickAlias",aliasList);
	Config().set("credentials/nickData",dataList);
}

void DockNick::savePass() {
	QStringList aliasList, dataList;
	for(int i(0); i < m_passList->count(); ++i) {
		aliasList += m_passList->item(i)->text();
		dataList += m_passList->item(i)->data(32).toString();
	}
	Config().set("credentials/passAlias",aliasList);
	Config().set("credentials/passData",dataList);
}

void DockNick::addPass(QString data) {
	QString alias;
	int i = data.indexOf(':');
	if(i == -1) alias = data;
	else {
		alias = data.left(i);
		data = data.right(data.size() - i - 1);
	}
	
	addPass(alias, data);
}

QString DockNick::getNick() {
	QString str = m_nickList->currentItem()->data(32).toString();
	if(str.contains(':')) 
	{
		int i = str.indexOf(':');
		return str.left(i);
	} else return str;
}
QString DockNick::getPass() { 
	QString str = m_nickList->currentItem()->data(32).toString();
	if(str.contains(':'))
	{ //use special password for this nick
		int i = str.indexOf(':') + 1;
		return str.right(str.size() - i);
	} else return m_passList->currentItem()->data(32).toString();
}

QString DockNick::getNickAlias() { return m_nickList->currentItem()->text(); }
QString DockNick::getPassAlias() { return m_passList->currentItem()->text(); }

void DockNick::selectNick(int i) { m_nickList->setCurrentItem(m_nickList->item(i)); }
void DockNick::selectPass(int i) { m_passList->setCurrentItem(m_passList->item(i)); }

void DockNick::showNick(bool show) {
	if(show)
		m_nickList->show();
	else
		m_nickList->hide();
}

void DockNick::showPass(bool show) {
	if(show)
		m_passList->show();
	else
		m_passList->hide();
}

bool DockNick::getNickHidden() { return m_nickList->isHidden(); }
bool DockNick::getPassHidden() { return m_passList->isHidden(); }

void DockNick::delNick(QListWidgetItem* it) { 
	delete m_nickList->takeItem(m_nickList->row(it));
	
	QStringList alias, data;
	for(int i(0); i < m_nickList->count(); ++i) {
		alias += m_nickList->item(i)->text();
		data += m_nickList->item(i)->data(32).toString();	
	}
	Config().set("credentials/nickAlias",alias);
	Config().set("credentials/nickData",data);
}

void DockNick::delPass(QListWidgetItem* it) {
	delete m_passList->takeItem(m_passList->row(it)); 
	
	QStringList alias, data;
	for(int i(0); i < m_passList->count(); ++i) {
		alias += m_passList->item(i)->text();
		data += m_passList->item(i)->data(32).toString();
	}
	Config().set("credentials/passAlias",alias);
	Config().set("credentials/passData",data);
}
