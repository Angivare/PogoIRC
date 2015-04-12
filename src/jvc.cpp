#include "jvc.h"

Jvc::Jvc(QObject* parent) : QObject(parent) {
	m_http = new QNetworkAccessManager(this);
	m_req.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/x-www-form-urlencoded"));
	m_connecting = false;
	
	time_t now = time(NULL);
	tm ti = *localtime(&now);
	ti.tm_sec -= 60;
	m_lastPosted = mktime(&ti);
}
void Jvc::getVersion() {
	d << "Récupération de la version sur angiva.re";
	
#ifdef Q_OS_WIN32
	const QString arg("WIN");
#elif defined(Q_OS_LINUX)
	const QString arg("LIN");
#elif defined(Q_OS_MAC)
	const QString arg("MAC");
#endif
	
	m_req.setUrl(QUrl("http://irc.pogo.angiva.re/last_build.php?" + arg + "="));
	m_rep.push_back(new Response(m_http->get(m_req), 0));
	
	QObject::connect(m_rep.last(), SIGNAL(finished(int,Response*)), this, SLOT(getVersion1(int,Response*)));
}
void Jvc::getVersion1(int, Response* reply) {
	QString ver = reply->getRep()->readAll();
	
	m_rep.removeOne(reply);
	delete reply;
	
	QDateTime curr = QFileInfo("pogo_irc.exe").lastModified();
	QDateTime latest = QDateTime::fromTime_t(ver.toInt());
	if(curr < latest) {
		emit setStatusRequest("Une nouvelle version est disponible. Veuillez mettre PogoIRC à jour depuis le menu \"Options\"");
		d.silent("PogoIRC n'est pas à jour; la version la plus récente date de: "
			+ latest.toString() + "; la version actuelle date de: "
			+ curr.toString());
	}
}

void Jvc::update() {
	d << "Téléchargement du patcher...";

#ifdef Q_OS_WIN32
	const QString url("http://irc.pogo.angiva.re/winpatcher");
#elif defined(Q_OS_LINUX)
	const QString url("http://irc.pogo.angiva.re/linpatcher");
#elif defined(Q_OS_MAC)
	const QString url("http://irc.pogo.angiva.re/macpatcher");
#endif
	
	m_req.setUrl(QUrl(url));
	m_rep.push_back(new Response(m_http->get(m_req), 0));
	
	QObject::connect(m_rep.last(), SIGNAL(finished(int,Response*)), this, SLOT(update1(int,Response*)));
}
void Jvc::update1(int, Response* reply) {
	d << "Éxécution du patcher...";
	QFile f(Config().getUpdateScript());
	f.open(QIODevice::WriteOnly);
	while(reply->getRep()->bytesAvailable())
		f.write(reply->getRep()->read(reply->getRep()->bytesAvailable()));
	f.close();

#ifdef Q_OS_WIN
	int res = (int)ShellExecuteA(0, "open", Config().getUpdateScript().toLocal8Bit().constData(), 0, 0, SW_SHOWNORMAL);
	if(res == SE_ERR_ACCESSDENIED)
		ShellExecuteA(0, "runas", Config().getUpdateScript().toLocal8Bit().constData(), 0, 0, SW_SHOWNORMAL);
#else
	if(!QProcess::startDetached(Config().getUpdateScript(), QStringList()))
		d.crit("Impossible de lancer patcher.exe, chemin:\n"
		+ Config().getUpdateScript());
#endif
}
void Jvc::heartBeat() {
	d.silent("Updating stats...");
#ifdef Q_OS_WIN32
	const QString sys("WIN");
#elif defined(Q_OS_LINUX)
	const QString sys("LIN");
#elif defined(Q_OS_MAC)
	const QString sys("MAC");
#endif
	
	uint ts = QFileInfo("pogo_irc.exe").lastModified().toTime_t();
	
	m_req.setUrl(QUrl("http://irc.pogo.angiva.re/stats.php"
		+ QString("?sys=") + sys
		+ "&ts=" + QString::number(ts)
	));
	m_http->get(m_req);
}
void Jvc::blacklist() {
	d.silent("Retrieving blacklist online");
	
	m_req.setUrl(QUrl("http://irc.pogo.angiva.re/blacklist.php"));
	m_rep.push_back(new Response(m_http->get(m_req), 0));	
	QObject::connect(m_rep.last(), SIGNAL(finished(int,Response*)), this, SLOT(blacklist1(int,Response*)));
} void Jvc::blacklist1(int i, Response* reply) {
	QStringList bl;
	while(!reply->getRep()->atEnd())
		bl.append(reply->getRep()->readLine().toLower().trimmed());
	Config().set("blacklist", bl);
}

/*-------------------------------
 * CONNECT CALLCHAIN
 *-----------------------------*/
bool Jvc::connected(QString compare) {
	if(compare!=m_currNick) {
		d << "Déconnexion";
		disconnect();
	}
	return isConnected();
}
bool Jvc::isConnected() {
	QList<QNetworkCookie> list = m_http->cookieJar()->cookiesForUrl(QUrl("http://www.jeuxvideo.com/"));
	for(QList<QNetworkCookie>::iterator it(list.begin()); it != list.end(); ++it)
		if(it->name() == "coniunctio")
			return true; //Login cookie
	m_currNick = QString();
	return false; //No login cookie
}

//void Jvc::disconnect() { m_http->setCookieJar(new QNetworkCookieJar()); } //Clear cookies
void Jvc::disconnect() { m_http->deleteLater(); m_http = new QNetworkAccessManager(this); }

void Jvc::connect(QString nick, QString pass, QString toPost, TopicViewH* topicView) {
	d << "Connexion à JVC.";
	m_connecting = true;
	for(int i(0); i < m_rep.count(); ++i) {
		//abort all connections
		//m_rep[i]->getRep()->abort();
		//m_rep[i]->getRep()->deleteLater();
		//m_rep.removeAt(i);
	}
	
	int index;
	if(m_connect.isEmpty())	index = 0;
	else					index = m_connect.begin().key() - 1;
	m_connect[index].nick = nick;
	m_connect[index].pass = pass;
	m_connect[index].post.msg = toPost;
	m_connect[index].post.url = getFirstPage((*topicView)->getUrl());
	m_connect[index].post.topicView = topicView;
	
	m_req.setUrl(QUrl("http://www.jeuxvideo.com/login"));
	
	m_rep.push_back(new Response(m_http->get(m_req), index));
	
	QObject::connect(m_rep.last(), SIGNAL(finished(int,Response*)), this, SLOT(connect2(int,Response*)));
	connectErr();
}
void Jvc::connect1(int i, Response* reply) {
	d.silent("connect1 " + QString::number(i));
	m_req.setUrl(QUrl("http://www.jeuxvideo.com/login"));
	
	m_rep.push_back(new Response(m_http->get(m_req), i));
	
	QObject::connect(m_rep.last(), SIGNAL(finished(int,Response*)), this, SLOT(connect2(int,Response*)));
	connectErr();
	
	m_rep.removeOne(reply);
	reply->deleteLater();
}
void Jvc::connect2(int i, Response* reply) {
	d.silent("connect2 " + QString::number(i));
	Timer *timer = new Timer(new QTimer(), reply, i);
	QObject::connect(timer, SIGNAL(timeout(int,Response*)), this, SLOT(connect3(int,Response*)));
	QObject::connect(timer, SIGNAL(timeout(int,Response*)), timer->getTimer(), SLOT(deleteLater()));
	timer->getTimer()->start(OFFSET);
}
void Jvc::connect3(int i, Response* reply) {
	d.silent("connect3 " + QString::number(i));
	m_req.setUrl(QUrl("http://www.jeuxvideo.com/login"));
	
	QString tk = getTk(reply);
	
	QByteArray data;
	data += "login_pseudo=" + QUrl::toPercentEncoding(m_connect[i].nick);
	data += "&login_password=" + QUrl::toPercentEncoding(m_connect[i].pass);
	data += "&" + tk;
	
	m_rep.push_back(new Response(m_http->post(m_req, data), i));
	
	QObject::connect(m_rep.last(), SIGNAL(finished(int,Response*)), this, SLOT(connect5(int,Response*)));
	connectErr();
	
	m_rep.removeOne(reply);
	reply->deleteLater();
}
void Jvc::connect4(int i, Response* reply) {
	d.silent("connect4 " + QString::number(i));
	Timer *timer = new Timer(new QTimer(), reply, i);
	QObject::connect(timer, SIGNAL(timeout(int,Response*)), this, SLOT(connect5(int,Response*)));
	QObject::connect(timer, SIGNAL(timeout(int,Response*)), timer->getTimer(), SLOT(deleteLater()));
	timer->getTimer()->start(OFFSET);
}
void Jvc::connect5(int i, Response* reply) {
	d.silent("connect5 " + QString::number(i));
	QString tk, sign, url;
	tk = getTkAndSign(reply, &sign);
	url = "http://www.jeuxvideo.com/captcha/ccode.php?" + sign;
	m_req.setUrl(QUrl(url));
	
	tk.replace("fs_signature="+sign, "fs_signature="+QUrl::toPercentEncoding(sign));
	m_connect[i].data = QByteArray();
	m_connect[i].data.append("login_pseudo=" + QUrl::toPercentEncoding(m_connect[i].nick));
	m_connect[i].data.append("&login_password=" + QUrl::toPercentEncoding(m_connect[i].pass));
	m_connect[i].data += "&" + tk;
	
	m_rep.push_back(new Response(m_http->get(m_req), i));
	QObject::connect(m_rep.last(), SIGNAL(finished(int,Response*)), this, SLOT(connect6(int,Response*)));
	
	m_rep.removeOne(reply);
	reply->deleteLater();
}
void Jvc::connect6(int i, Response* reply) {
	d.silent("connect6 " + QString::number(i));
	m_req.setUrl(QUrl("http://www.jeuxvideo.com/login"));
	
	m_connect[i].data += "&fs_ccode=" + askCaptcha(reply);
	
	m_rep.push_back(new Response(m_http->post(m_req, m_connect[i].data), i));
	QObject::connect(m_rep.last(), SIGNAL(finished(int,Response*)), this, SLOT(connect7(int,Response*)));
	
	m_rep.removeOne(reply);
	reply->deleteLater();
	
}
void Jvc::connect7(int i, Response* reply) {
	d.silent("connect7 " + QString::number(i));
	m_connecting = false;
	QString page = reply->getRep()->readAll();
	if(page.contains("<div class=\"bloc-erreur\">"))
	{	//NOT CONNECTED
		int beg, end;
		beg = page.indexOf("<div class=\"bloc-erreur\">") + strlen("<div class=\"bloc-erreur\">");
		end = page.indexOf("</div>", beg);
		d.crit("Connexion échouée: " + page.left(end).right(end-beg));
	} else if(page == QString()) { //CONNECTED
		d << "Successfully connected.";
		emit connected();
		m_currNick = m_connect[i].nick;
		if(m_connect[i].post.msg != QString())
			//postMsg(m_connect[i].post.msg, m_connect[i].post.topicView);
			prepareForm(m_connect[i].post.topicView, m_connect[i].post.msg);
	} else {
		d.silent(reply->getRep()->rawHeader("Set-Cookie"));
		d.silent(page);
		d.crit("Error during connection.");
	}
	
	m_connect.erase(m_connect.find(i));
	m_rep.removeOne(reply);
	reply->deleteLater();
}

/*-------------------------------
 * FORM PREPARING CALLCHAIN
 *------------------------------*/
void Jvc::prepareForm(TopicViewH* topic, QString msg) {
	d.silent("Preparing form for " + (*topic)->getUrl());
	if(m_connecting) {
		d << "Préparation de formulaire annulé";
		return;
	}
	if(!isConnected()) {
		d.silent("Form preparation aborted: not connected");
		return;
	}
	int index;
	if(m_prepare.isEmpty())	index = 0;
	else					index = m_prepare.begin().key() - 1;
	
	m_prepare[index].topicView = topic;
	m_prepare[index].msg = msg;
	
	m_req.setUrl(QUrl((*topic)->getUrl()));
	m_rep.push_back(new Response(m_http->get(m_req), index));
	
	QObject::connect(m_rep.last(), SIGNAL(finished(int,Response*)), this, SLOT(prepareForm1(int,Response*)));
	connectErr();
}
void Jvc::prepareForm1(int i, Response* reply)
{
	QString page; 
	while(reply->getRep()->bytesAvailable())
		page += QString::fromUtf8(reply->getRep()->readLine());//Read page
	
	m_prepare[i].form.tk = getTk(page);
	if(m_prepare[i].form.tk == QString()) {
		//Error, try later
		d.silent("Retrying form refresh");
		prepareForm(m_prepare[i].topicView, m_prepare[i].msg);
		
		m_prepare.erase(m_prepare.find(i));
		m_rep.removeOne(reply);
		reply->deleteLater();
		return;
	}
	
	if(-1 != page.indexOf(" name=\"fs_signature\" ")) {
		//Captcha
		int n(0);
		m_prepare[i].form.sign = getFieldVal(page, "fs_signature", n);
		m_prepare[i].form.tk += "&fs_signature=" + m_prepare[i].form.sign;
	} else m_prepare[i].form.sign = "";
	
	if(*m_prepare[i].topicView)
		(*m_prepare[i].topicView)->setForm(m_prepare[i].form);
	else {
		delete m_prepare[i].topicView;
		m_prepare[i].topicView = NULL;
	}
	
	if(m_prepare[i].msg != QString() && m_prepare[i].topicView)
	{	//Post new msg
		postMsgFromForm(m_prepare[i].msg, m_prepare[i].topicView);
	}
	
	m_prepare.erase(m_prepare.find(i));
	m_rep.removeOne(reply);
	reply->deleteLater();
}

/*-------------------------------
 * MESSAGE POSTING CALLCHAIN
 *-----------------------------*/
int Jvc::prePostMsg(QString msg, TopicViewH* toPost) {
	int index;
	if(m_post.isEmpty())	index = 0;
	else					index = m_post.begin().key() -  1;
	m_post[index].msg = msg;
	m_post[index].topicView = toPost;
	m_post[index].url = (*toPost)->getUrl();
	m_post[index].tk = QByteArray();
	
	return index;
}

void Jvc::postMsgFromForm(QString msg, TopicViewH* toPost) {
	d.silent("postMsgFromForm()");
	if(m_connecting) {
		d << "Envoi de message annulé";
		return;
	}
	int index = prePostMsg(msg, toPost);
	
	Timer *timer = new Timer(new QTimer(), 0, index);
	QObject::connect(timer, SIGNAL(timeout(int)), this, SLOT(postMsgFromForm1(int)));
	QObject::connect(timer, SIGNAL(timeout(int)), timer, SLOT(deleteLater()));
	timer->getTimer()->start(OFFSET);
}
void Jvc::postMsgFromForm1(int i) {
	d.silent("postMsgFromForm1()");
	postMsg("", NULL, false, i);
}

void Jvc::postMsg(QString msg, TopicViewH* toPost, bool init, int fromIndex) {
	d.silent("postMsg()");
	if(m_connecting) {
		d << "Envoi de message annulé";
		return;
	}
	int index;
	if(init)
		index = prePostMsg(msg, toPost);
	else
		index = fromIndex;

	if(!(*m_post[index].topicView)->isFormReady()) {
		d << "Formulaire non chargé, réessayez dans quelques secondes.";
		return;
	} else (*m_post[index].topicView)->expireForm();
	
	
	formCache fc = (*m_post[index].topicView)->getForm();
	QByteArray data; data += fc.tk;
	data += "&message_topic=" + QUrl::toPercentEncoding(m_post[index].msg);
	data += "&form_alias_rang=" + QString("1");
	m_post[index].tk = data;
	
	if(difftime(time(NULL), m_lastPosted) < MSG_DELAY) { //has signature
		m_post[index].sign = fc.sign;
		m_req.setUrl(QUrl("http://www.jeuxvideo.com/captcha/ccode.php?" + fc.sign));
		m_rep.push_back(new Response(m_http->get(m_req), index));
		QObject::connect(m_rep.last(), SIGNAL(finished(int,Response*)), this, SLOT(postMsgCaptcha(int,Response*)));
		connectErr();
		return;
	}
	
	data += "&fs_ccode=";
	d.silent(data);
	
	m_req.setUrl(QUrl(m_post[index].url));
	m_rep.push_back(new Response(m_http->post(m_req, data), index));
	QObject::connect(m_rep.last(), SIGNAL(finished(int,Response*)), this, SLOT(postMsg3(int,Response*)));
	connectErr();
}
void Jvc::postMsg1(int,Response*){} //unused
void Jvc::postMsg2(int,Response*){} //unused
void Jvc::postMsg3(int i, Response* reply) {
	d.silent("postMsg3 " + QString::number(i));
	QString page; 
	while(reply->getRep()->bytesAvailable())
		page += QString::fromUtf8(reply->getRep()->readLine());//Read page
	
	if(page != QString()) {
		int beg, end;
		beg = page.indexOf("<div class=\"alert-row\">") + strlen("<div class=\"alert-row\">");
		end = page.indexOf("</div>", beg);
		d.crit("Connexion échouée: " + page.left(end).right(end-beg));
	} else { //POSTED
		time(&m_lastPosted);
		d << "Message successfully posted.";
		emit posted();
	}
	
	prepareForm(m_post[i].topicView);
	m_post.erase(m_post.find(i));
	m_rep.removeOne(reply);
	reply->deleteLater();
}
void Jvc::postMsgCaptcha(int i, Response* reply) {
	d.silent("PostMsgCaptcha()");
	if(m_connecting) {
		d << "Envoi de message annulé";
		m_post.erase(m_post.find(i));
		m_rep.removeOne(reply);
		reply->deleteLater();
		return;
	}
	m_post[i].tk += "&fs_ccode=" + askCaptcha(reply);
	
	m_req.setUrl(QUrl(m_post[i].url));
	m_rep.push_back(new Response(m_http->post(m_req, m_post[i].tk), i));
	
	QObject::connect(m_rep.last(), SIGNAL(finished(int,Response*)), this, SLOT(postMsg3(int,Response*)));
	connectErr();
	
	m_rep.removeOne(reply);
	reply->deleteLater();
}

/* -----------------------
 * POST EDITING CALLCHAIN
 * ---------------------*/
void Jvc::editMsg(int id, QString msg, TopicViewH* topicView) {
	d.silent("editMsg");
	if(m_connecting) {
		d << "Édition de message annulé";
		return;
	}
	int index;
	if(m_edit.isEmpty())	index = 0;
	else					index = m_edit.begin().key() -  1;
	m_edit[index].id = id;
	m_edit[index].msg = msg;
	m_edit[index].topicView = topicView;
	
	m_req.setUrl(QUrl("http://www.jeuxvideo.com/fortytwo/forums/message/" + QString::number(id)));
	m_rep.push_back(new Response(m_http->get(m_req), index));
	
	QObject::connect(m_rep.last(), SIGNAL(finished(int,Response*)), this, SLOT(editMsg1(int,Response*)));
	connectErr();
}
void Jvc::editMsg1(int i, Response* reply) {
	d.silent("editMsg1 " + QString::number(i));
	if(m_connecting) {
		d << "Édition de message annulé";
		m_edit.erase(m_edit.find(i));
		m_rep.removeOne(reply);
		reply->deleteLater();
		return;
	}
	const int statusCode = reply->getRep()->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
	if(statusCode != 301)
		editMsg2(i, reply);
	QString url = reply->getRep()->rawHeader("Location");
	url.prepend("http://www.jeuxvideo.com");
	d.silent(url);
	
	m_req.setUrl(QUrl(url));
	m_rep.push_back(new Response(m_http->get(m_req), i));
	QObject::connect(m_rep.last(), SIGNAL(finished(int,Response*)), this, SLOT(editMsg2(int,Response*)));
	connectErr();
	
	m_rep.removeOne(reply);
	reply->deleteLater();
}
void Jvc::editMsg2(int i, Response* reply) {
	d.silent("editMsg2 ");
	if(m_connecting) {
		d << "Édition de message annulé";
		m_edit.erase(m_edit.find(i));
		m_rep.removeOne(reply);
		reply->deleteLater();
		return;
	}
	
	QString tk = getAjaxTk(reply);
	m_edit[i].tk = tk;
	tk += "&id_message=" + QString::number(m_edit[i].id);
	tk += "&action=get";
	
	m_req.setUrl(QUrl("http://www.jeuxvideo.com/forums/ajax_edit_message.php?" + tk));
	
	m_rep.push_back(new Response(m_http->get(m_req), i));
	QObject::connect(m_rep.last(), SIGNAL(finished(int,Response*)), this, SLOT(editMsg3(int,Response*)));
	connectErr();
	
	m_rep.removeOne(reply);
	reply->deleteLater();
}
void Jvc::editMsg3(int i, Response* reply) {
	d.silent("editMsg3 " + QString::number(i));
	if(m_connecting) {
		d << "Édition de message annulé";
		m_edit.erase(m_edit.find(i));
		m_rep.removeOne(reply);
		reply->deleteLater();
		return;
	}
	
	QString page;
	while(reply->getRep()->bytesAvailable())
		page += QString::fromUtf8(reply->getRep()->readLine());//Read page
	page.replace("\\", "");
	QString tk = getTk(page);
	int n(0);
	QString sign = getFieldVal(page, "fs_signature", n);
	d.silent("Sign:" + sign);
	
	QByteArray data; data += tk;
	data += "&" + m_edit[i].tk;
	data += "&id_message=" + QString::number(m_edit[i].id);
	data += "&message_topic=" + QUrl::toPercentEncoding(m_edit[i].msg);
	data += "&action=post";
	d.silent("Data: " + data);
	
	if(!sign.isEmpty())
	{ //Handle captcha
		data += "&fs_signature=" + QUrl::toPercentEncoding(sign);
		m_edit[i].tk = data;
		m_req.setUrl(QUrl("http://www.jeuxvideo.com/captcha/ccode.php?" + sign));
		m_rep.push_back(new Response(m_http->get(m_req), i));
		QObject::connect(m_rep.last(), SIGNAL(finished(int,Response*)), this, SLOT(editMsgCaptcha(int,Response*)));
		connectErr();
		
		m_rep.removeOne(reply);
		reply->deleteLater();
		return;
	}
	
	m_req.setUrl(QUrl("http://www.jeuxvideo.com/forums/ajax_edit_message.php"));
	
	m_rep.push_back(new Response(m_http->post(m_req, data), i));
	
	QObject::connect(m_rep.last(), SIGNAL(finished(int,Response*)), this, SLOT(editMsg4(int,Response*)));
	connectErr();
	
	m_rep.removeOne(reply);
	reply->deleteLater();
}
void Jvc::editMsg4(int i, Response* reply) {
	d.silent("editMsg4 " + QString::number(i));
	QString page; 
	while(reply->getRep()->bytesAvailable())
		page += QString::fromUtf8(reply->getRep()->readLine());//Read page
	
	int beg, end;
	beg = page.indexOf("\"erreur\":[") + strlen("\"erreur\":[");
	end = page.indexOf("]", beg);
	QString err = page.left(end).right(end-beg);
	if(err != QString()) {
		//Error
		d.crit("Erreur pendant l'édition: " + err);
		d.silent(page);
	} else {
		d << "Édition réussie";
		emit posted();
	}
	
	m_edit.erase(m_edit.find(i));
	m_rep.removeOne(reply);
	reply->deleteLater();
}
void Jvc::editMsgCaptcha(int i, Response* reply) {
	d.silent("editMsgCaptcha " + QString::number(i));
	if(m_connecting) {
		d << "Édition de message annulé";
		m_edit.erase(m_edit.find(i));
		m_rep.removeOne(reply);
		reply->deleteLater();
		return;
	}
	m_edit[i].tk += "&fs_ccode=" + askCaptcha(reply);
	QByteArray data; data+= m_edit[i].tk;
	
	d.silent("Data:" + data);
	
	m_req.setUrl(QUrl("http://www.jeuxvideo.com/forums/ajax_edit_message.php"));
	m_rep.push_back(new Response(m_http->post(m_req, data), i));
	QObject::connect(m_rep.last(), SIGNAL(finished(int,Response*)), this, SLOT(editMsg4(int,Response*)));
	connectErr();
	
	m_rep.removeOne(reply);
	reply->deleteLater();
}

/* ------------------------------------
 * RETRIEVING POSTED MESSAGES CALLCHAIN
 * ----------------------------------*/
void Jvc::getMsg(QString url, TopicViewH* topicView) {
	if(m_connecting) { d.silent("Connecting..."); return; }
	for(QMap<int,getMsg_data>::iterator it(m_topicView.begin()); it != m_topicView.end(); ++it)
		if(it->topicView == topicView && !it->processed)
			return;
	
	int index;
	if(m_topicView.isEmpty())	index = 0;
	else						index = m_topicView.begin().key() - 1;
//	d.silent(QTime::currentTime().toString("hh:mm:ss") + " getMsg() " + QString::number(index));
	m_topicView[index].topicView = topicView;
	m_topicView[index].processed = false;
	m_topicView[index].timedOut = false;
	
	//Callchain timeout
	Timer *timer = new Timer(new QTimer(), NULL, index);
	QObject::connect(timer, SIGNAL(timeout(int)), this, SLOT(getMsgTimeout(int)));
	QObject::connect(timer, SIGNAL(timeout(int)), timer->getTimer(), SLOT(deleteLater()));
	timer->getTimer()->start(TIMEOUT);
	
	m_topicView[index].timer = timer;
	
	m_req.setUrl(QUrl(getTopicLastMsg(url)));
	
	m_rep.push_back(new Response(m_http->get(m_req), index));
	
	QObject::connect(m_rep.last(), SIGNAL(finished(int,Response*)), this, SLOT(getMsg1(int,Response*)));
	connectErr();
	
}
void Jvc::getMsg1(int i, Response* reply) {
	if(m_topicView[i].timedOut) {
		d << "GetMsg() timed out";
		m_rep.removeOne(reply);
		reply->deleteLater();
		m_topicView.erase(m_topicView.find(i));
		return;
	}
	QString page; 
	while(reply->getRep()->bytesAvailable())
		page += QString::fromUtf8(reply->getRep()->readLine());//Read page
	
	int n = page.indexOf("<div class=\"pagi-after\"><span>");
	if(n != -1) {
		int beg, end(0);
		QList<int> pages;
		while( -1 != (beg = page.indexOf(" class=\"lien-jv\">", end)) ) {
			beg += strlen(" class=\"lien-jv\">");
			end = page.indexOf("</a>", beg);
			pages.append(page.left(end).right(end-beg).toInt());
		}
		qSort(pages);
		QString k = QString::number(pages.last());
		beg = page.indexOf(" class=\"lien-jv\">" + k + "</a>");
		beg = page.lastIndexOf("/forums/", beg);
		end = page.indexOf("\" ", beg);
		QString url = "http://www.jeuxvideo.com" + page.left(end).right(end-beg);
		
		m_req.setUrl(QUrl(url));
		m_rep.push_back(new Response(m_http->get(m_req), i));
		QObject::connect(m_rep.last(), SIGNAL(finished(int,Response*)), this, SLOT(getMsg2(int,Response*)));
//		d.silent("Last page: " + url);
	} else {
		m_rep.removeOne(reply);
		reply->deleteLater();
		d.silent("No last page found");
		return getMsg2(i, page);
	}
	
	m_rep.removeOne(reply);
	reply->deleteLater();
}
void Jvc::getMsg2(int i, Response* reply) {
	if(m_topicView[i].timedOut) {
		d << "GetMsg() timed out";
		m_rep.removeOne(reply);
		reply->deleteLater();
		m_topicView.erase(m_topicView.find(i));
		return;
	}
	QString page;
	while(reply->getRep()->bytesAvailable())
		page += QString::fromUtf8(reply->getRep()->readLine());//Read page
	m_rep.removeOne(reply);
	reply->deleteLater();
	return getMsg2(i, page);
}

void Jvc::getMsg2(int i, QString& page) {
	if(m_topicView[i].timedOut) {
		d << "GetMsg() timed out";
		m_topicView.erase(m_topicView.find(i));
		return;
	}
	posts params;
	params.newPost = false;
	QString title;
	
	int beg(0), end;
	
	beg = page.indexOf("<title>") + strlen("<title>");
	end = page.indexOf(" sur le forum ",beg);
	title = page.left(end).right(end-beg);
	htmlDecode(title);
	
	if(page.contains("class=\"bloc-message-forum \""))
		parseHighDensity(page, &params);
	else if(page.contains("<div id=\"forum-right-col\"")) {
		Parser parser(page);
		parser.jumpTo("<div id=\"forum-right-col\"");
		parser.jumpTo("&jv_page_url=");
		QString url = parser.get("%2F", "&").replace("%2F", "/");
		url.prepend("http://www.jeuxvideo.com/");
		const int beg = url.indexOf('-', url.indexOf('-', url.indexOf('-', url.indexOf('-'+1)+1)+1)+1)+1;
		const int end = url.indexOf('-', beg);
		const int pageNum = url.left(end).right(end-beg).toInt() - 1;
		url.replace(beg, end-beg, QString::number(pageNum));
		
		m_req.setUrl(QUrl(url));
		m_rep.append(new Response(m_http->get(m_req), i));
		QObject::connect(m_rep.last(), SIGNAL(finished(int,Response*)), this, SLOT(getMsg2(int,Response*)));
		connectErr();
		return;
	} else { //Error: no single message
		d << "Aucn message trouvé: page mal formée.";
		m_topicView[i].processed = true;
		m_topicView[i].timer->disconnect();
		m_topicView.erase(m_topicView.find(i));
		return;
	}
	
	if(*m_topicView[i].topicView) { //Pointer is valid
		(*m_topicView[i].topicView)->setData(title);
		(*m_topicView[i].topicView)->refresh(&params);
		for(int n(0); n < params.p.count(); ++n) {
			addPost(&params, n, m_topicView[i].topicView);
		}
	} else delete m_topicView[i].topicView; //Its TopicView doesn't exist anymore
	
	m_topicView[i].processed = true;
	m_topicView[i].timer->disconnect();
	m_topicView.erase(m_topicView.find(i));
//	d.silent("processed: " + QString::number(i));
}
void Jvc::getMsgTimeout(int i) {
	d.silent("GetMsg() " + QString::number(i) + " timeout");
	if(m_topicView[i].processed)
		m_topicView.erase(m_topicView.find(i));
	m_topicView[i].processed = true;
	m_topicView[i].timedOut = true;
}

/* --------------------
   GET TOPIC CALLCHAIN (UNUSED)
  -----------------*/
void Jvc::getTopic(QString url, ForumViewH* forumView) {
	if(m_connecting) return;
	for(QMap<int,ForumViewH*>::iterator it(m_forumView.begin()); it != m_forumView.end(); ++it)
		if(*it == forumView) return;
	
	int index;
	if(m_forumView.isEmpty())	index = 0;
	else						index = m_forumView.begin().key() - 1;
	m_forumView[index] = forumView;
	
	m_req.setUrl(QUrl(getForumFirstPage(url)));
	
	m_rep.push_back(new Response(m_http->get(m_req), index));
	
	QObject::connect(m_rep.last(), SIGNAL(finished(int,Response*)), this, SLOT(getTopic1(int,Response*)));
	connectErr();
}
void Jvc::getTopic1(int i, Response* reply) {
	QString page; 
	while(reply->getRep()->bytesAvailable())
		page += QString::fromUtf8(reply->getRep()->readLine());//Read page
	
	QMap<QString,QStringList> topics;
	int beg, end;
	beg = page.indexOf("<nom_forum>") + strlen("<nom_forum>");
	end = page.indexOf("</nom_forum>", beg);
	QString title = page.left(end).right(end-beg);
	while( -1 != (beg = page.indexOf("<icone>", end)) )
	{
		//icon
		beg += strlen("<icone>");
		end = page.indexOf("</icone>", beg);
		topics["icon"].push_back(page.left(end).right(end-beg));
		//author
		beg = page.indexOf("<auteur>", end) + strlen("<auteur>");
		end = page.indexOf("</auteur>", beg);
		topics["author"].push_back(page.left(end).right(end-beg));
		//title
		beg = page.indexOf("<titre>", end) + strlen("<titre>");
		end = page.indexOf("</titre>", beg);
		topics["title"].push_back(page.left(end).right(end-beg));
		//response count
		beg = page.indexOf("<nb_reponses>", end) + strlen("<nb_reponses>");
		end = page.indexOf("</nb_reponses", beg);
		topics["response_count"].push_back(page.left(end).right(end-beg));
		//url
		beg = page.indexOf("jv://", end) + strlen("jv://");
		end = page.indexOf(".xml", beg);
		topics["url"].push_back("http://www.jeuxvideo.com/" + page.left(end).right(end-beg));
	}
	
	if(*m_forumView[i]) { //Pointer is valid
		(*m_forumView[i])->refresh(topics);
		(*m_forumView[i])->setTitle(title);
	} else delete m_forumView[i]; //Its ForumView doesn't exist anymore
	
	m_rep.removeOne(reply);
	reply->deleteLater();
	m_forumView.erase(m_forumView.find(i));
}

/*----------------------------------------------
 * PRIVATE UTILITY METHODS 
 * USED BY CALL CHAINS
 *--------------------------------------------*/
void Jvc::processMsg(QString& msg) {
}

void Jvc::parseHighDensity(const QString& page, posts *params) {
	Parser parser(page);
	parser.jumpTo("<div class=\"bloc-nb-mp\"");
	if(parser.find("<span class=\"sup\">") < parser.find("</a>"))
	{	//New MPs found
		params->mp = parser.get("<span class=\"sup\">", "</span>").toInt();
	} else params->mp = 0;
	while(parser.jumpTo("<div class=\"bloc-message-forum \""))
	{
		const int nextHidden = parser.find("style=\"display: none;\"");
		if(nextHidden < parser.find(">")
		&& nextHidden != -1)
			continue;
		
		post p; int i;
		int next(parser.find("<div class=\"bloc-message-forum \""));
		p.id = parser.get("data-id=\"", "\"").toInt();
		if(parser.jumpTo("<div class=\"bloc-avatar-msg\""))
			p.avatar = parser.get("<img src=\"", "\" ");
		else
			p.avatar = QString();
		parser.jumpTo("\"bloc-header\"");
		p.rank = parser.get("text-", "\" target=\"_blank\">");
		p.nick = parser.get("\">", "</span>");
		p.lw_nick = p.nick.toLower();
		parser.jumpTo("class=\"bloc-date-msg\"");
		p.date = parser.get("target=\"_blank\">", " à");
		p.time = parser.get("à", "</span>");
		if(!parser.jumpTo("<div class=\"txt-msg  text-enrichi-forum \">"))
			parser.jumpTo("<div class=\"txt-msg  text-desenrichi-forum \">");
		p.msg = parser.get("<", "</div>", false);
		i = parser.find("<div class=\"signature-msg\"");
		if((i != -1) && (i < next || -1 == next))
		{
			parser.jumpTo("<div class=\"signature-msg\"");
			p.sign = parser.get("<", "</div>");
		} else p.sign = QString();
		i = parser.find("<div class=\"info-edition-msg\">");
		if((i != -1) && (i < next || -1 == next))
		{
			p.edited = "edited";
			parser.jumpTo("<div class=\"info-edition-msg\">");
			p.editDate = parser.get(" le ", " à");
			p.editTime = parser.get("à", "par");
			p.editNick = parser.get(" target=\"_blank\">", "</a>");
		} else p.edited = "not-edited";
		params->p.push_back(p);
	}
	parser.jumpTo("<div id=\"forum-right-col\"");
	params->connected = parser.get("<span class=\"nb-connect-fofo\">", " connecté(s)</span>").toInt();
	parser.jumpTo("&jv_page_url=%2Fforums%2F");
	params->type = parser.get("", "-").toInt();
	params->forum = parser.get("-", "-").toInt();
	params->topic = parser.get("-", "-").toInt();
	params->page = parser.get("-", "-").toInt();
	
	if(!params->p.size()) {
		d.silent(page);
	}
}

QString Jvc::getFieldVal(const QString& page, const char* name, int& index) {
	const int rest = strlen("\" value=\"");
	int beg, end;
	QString prefix = "name=\"";
	
	beg = page.indexOf(prefix+name, index);
	if(beg == -1) return QString();
	beg += (prefix+name).size() + rest;
	end = page.indexOf("\"/>", beg);
	if(end == -1) return QString();
	
	index = end;
	return page.left(end).right(end-beg);
}
QString Jvc::getTk(Response* reply) {
	QString page;
	while(reply->getRep()->bytesAvailable())
		page += QString::fromUtf8(reply->getRep()->readLine());//Read page
	return getTk(page);
}
QString Jvc::getTk(QString page) {
	QString tk;
	if(!page.contains("fs_session")) {
		//Basic error verification
		d << "No tokens in page: couldn't parse it.";
		return QString();
	}
	
	int i(0), i2;
	//Get hidden fields' values
	tk += "fs_session="		+ getFieldVal(page, "fs_session", i);
	tk += "&fs_timestamp="	+ getFieldVal(page, "fs_timestamp", i);
	tk += "&fs_mdr_int="	+ getFieldVal(page, "fs_mdr_int", i);
	tk += "&fs_timeout="	+ getFieldVal(page, "fs_timeout", i);
	tk += "&fs_version="	+ getFieldVal(page, "fs_version", i);
	
	//Get values of fields with random names
	for(int n(0); n < 2; ++n)
	{	//There are two such fields
		i = page.indexOf("name=\"", i) + strlen("name=\"");
		i2= page.indexOf("\"", i);
		tk += '&' + page.left(i2).right(i2-i); //Field name
		i = page.indexOf("value=\"", i2) + strlen("value=\"");
		i2= page.indexOf("\"/>", i);
		tk += '=' + page.left(i2).right(i2-i); //Field value
	}
	return tk;
}
QString Jvc::getTkAndSign(Response* reply, QString* sign) {
	QString page, data;
	int i(0);
	while(reply->getRep()->bytesAvailable())
		page += QString::fromUtf8(reply->getRep()->readLine());//Read page
	data = getTk(page);
	if(sign) {
		*sign = getFieldVal(page, "fs_signature", i);
		data += "&fs_signature=" + *sign;
	} else
		data += "&fs_signature=" + getFieldVal(page, "fs_signature", i);
	return data;
}
QString Jvc::getAjaxTk(Response* reply) {
	QString page, tk;
	while(reply->getRep()->bytesAvailable())
		page += QString::fromUtf8(reply->getRep()->readLine());//Read page
	
	if(!page.contains("ajax_timestamp_liste_messages")) {
		//Basic error verification
		d << "No ajax token in page: couldn't parse it.";
		return QString();
	}
	
	int beg, end;
	beg = page.indexOf("id=\"ajax_timestamp_liste_messages") + strlen("id=\"ajax_timestamp_liste_messages");
	beg+= strlen("\" value=\"");
	end = page.indexOf("\" />", beg);
	tk += "ajax_timestamp=" + page.left(end).right(end-beg);
	beg = page.indexOf("id=\"ajax_hash_liste_messages", end) + strlen("id=\"ajax_hash_liste_messages");
	beg+= strlen("\" value=\"");
	end = page.indexOf("\" />", beg);
	tk += "&ajax_hash=" + page.left(end).right(end-beg);
	
	return tk;
}

QString Jvc::getTopicId(QString url) {
	url = url.left(url.lastIndexOf("-"));
	return url.right(url.size() - url.lastIndexOf("/") - 1);
}

QString Jvc::getFirstPage(QString url) {
	int i = url.indexOf("-");
	i = url.indexOf("-", i+1);
	i = url.indexOf("-", i+1);
	return url.left(i) + "-1-0-1-0-0.htm";
}
QString Jvc::getForumFirstPage(QString url) {
	int n = strlen("http://www");
	int i = url.indexOf('-');
	i = url.indexOf('-',i+1);
	return "http://ws" + url.left(i).right(i-n)
	+ "-0-1-0-1-0-0.xml";
}
QString Jvc::getTopicForm(QString url) {
	int n = strlen("http://www");
	int i = url.lastIndexOf('/');
	int i2 = url.indexOf('-',i+1);
	int i3 = url.indexOf('-',i2+1);
	i3 = url.indexOf('-',i3+1);
	i3 = url.indexOf('-',i3+1);
	i3 = url.indexOf('-',i3+1);
	i3 = url.indexOf('-',i3+1);
	i3 = url.indexOf('-',i3+1);
	return "http://ws" + url.left(i).right(i-n) 
		+ "/5" + url.right(url.length() - i2).left(i3-i2) + "-0.xml";	
}

QString Jvc::getTopicLastMsg(QString url) {
	return url; //TEMPORARY
	int n = strlen("http://www");
	int i = url.lastIndexOf('/');
	int i2 = url.indexOf('-',i+1);
	int i3 = url.indexOf('-',i2+1);
	i3 = url.indexOf('-',i3+1);
	i3 = url.indexOf('-',i3+1);
	i3 = url.indexOf('-',i3+1);
	i3 = url.indexOf('-',i3+1);
	i3 = url.indexOf('-',i3+1);
	return "http://ws" + url.left(i).right(i-n) 
		+ "/3" + url.right(url.length() - i2).left(i3-i2) + "-0.xml";
}
QString& Jvc::htmlDecode(QString& html) {
	html.replace("&amp;", "&");
	html.replace("&quot;", "\"");
	html.replace("&#039;", "'");
	html.replace("&lt;", "<");
	html.replace("&gt;", ">");
	return html;
}
QString Jvc::askCaptcha(Response* reply) {
	QImage img; img.load(reply->getRep(), 0);
	QPixmap pixmap; pixmap.convertFromImage(img);
	
	QDialog d(NULL);
	QVBoxLayout* l = new QVBoxLayout(&d);
	QLabel *lbl = new QLabel("Entrez le captcha:", &d);
	l->addWidget(lbl);
	QHBoxLayout* h = new QHBoxLayout(&d);
	l->addLayout(h);
	QLabel *image = new QLabel(&d);
	image->setPixmap(pixmap);
	h->addWidget(image);
	QLineEdit* txt = new QLineEdit(&d);
	h->addWidget(txt);
	QPushButton* b = new QPushButton("Valider", &d);
	l->addWidget(b);
	QObject::connect(b, SIGNAL(pressed()), &d, SLOT(close()));
	d.exec();
	
	return txt->text();
}
bool Jvc::addPost(posts* params, int n, TopicViewH *topic) {
	if(params->p.at(n).nick.toLower() == m_currNick.toLower())
		params->p[n].rank = "self";
	if(!*topic) { d.silent("Topic doesn't exist anymore"); return false; }
//	d.silent("Adding post n° " + QString::number(n) + " by " + params->p.at(n).nick);
	if((*topic)->addPost(params, params->p[n]))
		params->newPost = true;
	if(params->newPost && n+1 >= params->p.count()) {
		emit newMsg();
	}
	return true;
}

void Jvc::connectErr() {
	QObject::connect(m_rep.last()->getRep(), SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(err(QNetworkReply::NetworkError)));
}

void Jvc::err(QNetworkReply::NetworkError err) {
	d << "Network error: " + QString::number(err);
}


/* -------------------------------------------
 * Response implementation: basic wrapper for
 * a QNetworkResponse that also stores the 
 * current operation's index.
 * ------------------------------------------*/
Response::Response(QNetworkReply* rep, int i)
	: m_rep(rep), m_i(i), m_data(NULL){ 
	connect(m_rep, SIGNAL(finished()), this, SLOT(finish()));
}
Response::~Response() {
	if(m_rep) m_rep->deleteLater();
}
QNetworkReply* Response::getRep() { return m_rep; }
void Response::finish() { emit finished(m_i, this); }
void Response::setData(void* d) { m_data = d; }
void*Response::getData() { return m_data; }
/* --------------------------------------------
 * Timer implementation: QTimer holding an index 
 * -------------------------------------------*/
Timer::Timer(QTimer* timer, Response* resp, int i) 
	: m_timer(timer), m_resp(resp), m_i(i) {
	timer->setSingleShot(true);
	connect(m_timer, SIGNAL(timeout()), this, SLOT(finish()));
}
Timer::~Timer(){
	if(m_timer) m_timer->deleteLater();
}
QTimer* Timer::getTimer() { return m_timer; }
Response* Timer::getRep() { return m_resp; }
void Timer::finish() { !m_resp ? emit timeout(m_i) : emit timeout(m_i, m_resp); }
