#include "topicview.h"

int TopicView::s_nextIndex(0), TopicView::s_currIndex(-1);

TopicView::TopicView(QWidget* parent) : QWidget(parent), 
	m_body(Config().getBody()), m_footer(Config().getFooter()), 
	m_html(Config().getHeader() + m_footer), m_url(QString()),
	m_state(this), m_unreadMessage(false), 
	m_historyIndex(-1), m_index(s_nextIndex),
	m_hasForm(false)
{	
	connect(this, SIGNAL(visibilityChanged(bool)), this, SLOT(setFocus(bool)));
	connect(&m_state, SIGNAL(quoteReq(QString&)), this, SLOT(quote(QString&)));
	
	s_nextIndex++;
	setObjectName("TopicView"+QString::number(m_index));
	m_htmlView = new QWebView(this);
	QVBoxLayout* layout = new QVBoxLayout;
	layout->addWidget(m_htmlView,0);
	layout->setContentsMargins(0,0,0,0);
	this->setLayout(layout);
	
	m_htmlView->setPage(new WebPage(m_htmlView));
	m_htmlView->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
	connect(m_htmlView, SIGNAL(linkClicked(QUrl)), this, SLOT(openLink(QUrl)));
	m_htmlView->installEventFilter(this);
	m_htmlView->settings()->setUserStyleSheetUrl(QUrl("data:text/css;charset=utf-8;base64,"
	+ Config().getJvCss().toLocal8Bit().toBase64()));
	m_htmlView->settings()->setAttribute(QWebSettings::LocalStorageEnabled, true);
	m_htmlView->settings()->setAttribute(QWebSettings::OfflineStorageDatabaseEnabled, true);
	m_htmlView->settings()->setLocalStoragePath(QCoreApplication::applicationDirPath () + "/data/temp");
	m_htmlView->settings()->setOfflineStoragePath(QCoreApplication::applicationDirPath () + "/data/temp");
	m_htmlView->settings()->setOfflineStorageDefaultQuota(9999);
	m_htmlView->settings()->setAttribute(QWebSettings::PluginsEnabled, true);
	m_htmlView->settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
	m_htmlView->settings()->setAttribute(QWebSettings::JavascriptCanAccessClipboard, true);
	m_htmlView->settings()->setAttribute(QWebSettings::PrivateBrowsingEnabled, false);
	
	m_html.replace("$DIR$", QCoreApplication::applicationDirPath());
	m_htmlView->setHtml(m_html);
	m_htmlView->page()->mainFrame()->addToJavaScriptWindowObject("g_state", &m_state);
	
	m_htmlView->setFocus(Qt::OtherFocusReason);
	s_currIndex = m_index;
	emit repaintRequest(m_index);
}

TopicView::~TopicView() {
}

bool TopicView::refresh(posts* batch) {
	QString js  = QString("refresh(\"%1\", \"%2\", \"%3\", \"%4\", \"%5\", \"%6\"); null;")
		.arg(QString::number(batch->forum), QString::number(batch->topic), QString::number(batch->page),
		QString::number(batch->type), QString::number(batch->connected),
		QString::number(batch->mp));
	m_htmlView->page()->mainFrame()->evaluateJavaScript(js);
	return true;
}

bool TopicView::addPost(posts* batch, post p) {
	QStringList bl = Config().get("blacklist").toStringList();
	for(int i(0); i < bl.count(); ++i)
		if(p.nick.toLower() == bl.at(i))
			return false;
	
	bool newPost(true);
	for(int i(0); i < m_idList.count(); ++i)
	{
		if(p.id == m_idList[i]) {
			if(p.editTime != QString())
			{ //Message may have been edited
				if(m_timeList[i] != p.editTime)
				{ //Message has been edited
					m_timeList[i] = p.editTime;
					newPost = false;
					break;
				} else return false;
			} else return false;
		}
	}
	if(newPost) {
		//New id
		m_idList.push_back(p.id);
		if(p.editTime != QString()) m_timeList.push_back(p.editTime);
		else m_timeList.push_back(p.time);
	}
	processMsg(p.msg);
	if(p.rank == "self" && newPost)
		addHistory(p);
	else if(p.rank == "self")
		for(int i(0); i < m_history.count(); i++)
			if(m_history[i].id == p.id)
			{
				post history = p;
				htmlToMarkDown(history.msg);
				m_history[i] = history;
			}
	QString post = m_body;
	post.replace("$TYPE$", QString::number(batch->type));
	post.replace("$FORUM$", QString::number(batch->forum));
	post.replace("$TOPIC$", QString::number(batch->topic));
	post.replace("$PAGE$", QString::number(batch->page));
	
	post.replace("$ID$", QString::number(p.id));
//	post.replace("$FORUM$", forum);
//	post.replace("$TOPIC$", topic);
//	post.replace("$PAGE$", page);
	post.replace("$TIME$", p.time);
	post.replace("$DATE$", p.date);
	post.replace("$RANK$", p.rank);
	post.replace("$AVATAR$", p.avatar);
	post.replace("$AVATAR_SMALL$", p.avatar);
	post.replace("$AVATAR_LARGE$", p.avatar);
	post.replace("$NICK$", p.nick);
	post.replace("$LWNICK$", p.lw_nick);
	post.replace("$EDITED$", p.edited);
	post.replace("$EDITTIME$", p.editTime);
	post.replace("$EDITDATE$", p.editDate);
	post.replace("$EDITNICK$", p.editNick);
	post.replace("$SIGN$", p.sign);
	post.replace("$MSG$", p.msg);
	
	//PROCESSING m_html DATA TO RESPECT MAXIMUM POSTS NUMBER
	const int maxPosts = Config().get("options/maxPosts", 200).toInt();
	if(maxPosts) {
		m_htmlView->page()->mainFrame()->evaluateJavaScript("clearPosts(" + QString::number(maxPosts) + "); null;");
	}
	
	jsEncode(post); jsEncode(p.nick);
	QString js  = QString("addPost(\"%1\", \"%2\", \"%3\"); null;").arg(QString::number(p.id), p.nick, post);
	m_htmlView->page()->mainFrame()->evaluateJavaScript(js);
	emit messRequest(this);
	return p.rank != "self";
}

void TopicView::addHistory(post p) {
	htmlToMarkDown(p.msg);
	m_history.push_front(p);
	
	if(m_history.count() > Config().get("options/maxHistory", 30).toInt())
	{
		if(m_historyIndex > m_history.count() - 2)
			m_historyIndex = m_history.count() - 2;
		m_history.pop_back();
	}
}

void TopicView::setForm(formCache fc) { 
	m_form = fc; m_hasForm = true;
	d.silent("Form prepared");
}
formCache TopicView::getForm() { return m_form; }
void TopicView::expireForm() { m_hasForm = false; }
bool TopicView::isFormReady() { return m_hasForm; }

//Various data handling
void TopicView::setData(QString title) {
	emit titleRequest(this,title);
}
QString TopicView::getTitle() { return windowTitle(); }

//Index manager
int TopicView::getIndex()		{ return m_index; }
int TopicView::getFocusIndex()	{ return s_currIndex; }

//Url GET/SET
void TopicView::setUrl(QString url) {
	if(isForum(url)) {
		m_url = url;
		m_url.replace("https://", "http://");
		return m_htmlView->load(QUrl(url));
	}
		
	m_url = url;
	m_url.replace("https://", "http://");
	m_body = Config().getBody();
	m_footer = Config().getFooter();
	m_html = Config().getHeader() + m_footer;
	m_html.replace("$DIR$", QCoreApplication::applicationDirPath());
	m_htmlView->setHtml(m_html);
	emit resizeHackReq();
	m_htmlView->page()->mainFrame()->addToJavaScriptWindowObject("g_state", &m_state);
	
	m_idList.clear();
	m_timeList.clear();
	m_history.clear();
	m_historyIndex = -1;
	
	s_currIndex = m_index;
	m_unreadMessage = false;
	setWindowTitle(QString());
	emit repaintRequest(m_index);
}
QString TopicView::getUrl() { return m_url; }
QWebView* TopicView::getView() { return m_htmlView; }

/* ---------------------------------
 * History management slots
 * -------------------------------*/
post TopicView::getPrevHistory() {
	if(m_historyIndex > -1)
		m_historyIndex--;
	return getHistory();
}
post TopicView::getNextHistory() {
	if(m_historyIndex < m_history.count() - 1)
		m_historyIndex++;
	return getHistory();
} post TopicView::getHistory() {
	d.silent("history n°" + QString::number(m_historyIndex));
	if(m_historyIndex != -1) {
		return m_history[m_historyIndex];
	} else {
		post ret; ret.msg = ""; ret.id=0;
		return ret;
	}
}

void TopicView::putFirstHistory() {
	m_historyIndex = -1;
}
int TopicView::getHistoryIndex() { return m_historyIndex; }

void TopicView::openLink(const QUrl& url) {
	QString str = url.toString();
	QString curr = m_htmlView->url().toString();
	if(str.indexOf("pogo://") == 0) {
		m_url = str.replace("pogo://", "http://");
		return m_htmlView->load(QUrl(m_url));
	}
	if(isTopic(str) && curr != "about:blank")
		return setUrl(str);
	if(curr == "about:blank")
		if(!QDesktopServices::openUrl(url))
		{
			QString str = " \"" + url.toString() + '"';
#ifdef Q_OS_WIN32
			QString cmd = "explorer";
#elif defined(Q_OS_LINUX)
			QString cmd = "xdg-open";
#elif defined(Q_OS_MAC)
			QString cmd = "open";
#endif
			system((cmd + str).toLocal8Bit().data());
		}
}
void TopicView::quote(QString& str) {
	emit quoteReq(str);
	htmlToMarkDown(str);
	d.silent(str);
}

void TopicView::setFocus(bool set) {
	if(set) {
		m_htmlView->setFocus(Qt::OtherFocusReason);
		m_unreadMessage = false;
		setWindowTitle(windowTitle().replace("/!\\ ", ""));
		s_currIndex = m_index;
		emit repaintRequest(m_index);
	}
}

/* -------------------------------
 * Virtual reimplementations
 * -------------------------------*/
bool TopicView::eventFilter(QObject* obj, QEvent* ev) {
	Q_UNUSED(obj);
	if(ev->type() == QEvent::FocusIn)
	{
		m_unreadMessage = false;
		setWindowTitle(windowTitle().replace("/!\\ ", ""));
		s_currIndex = m_index;
		emit repaintRequest(m_index);
	}
	return false;
}

WebPage::WebPage(QWidget* parent) : QWebPage(parent) {}
void WebPage::javaScriptConsoleMessage(const QString& message, int line, const QString& id) {
	::d.silent("error: " + message 
	+ "; line: " + QString::number(line) 
	+ "; id: " + id + "+;");
}

/* ------------------
 * JS Management
 * http://qt-project.org/doc/qt-4.8/qtwebkit-bridge.html
 * -----------------*/
JsState::JsState(QWidget* parent) : QObject(parent) {
	m_state = Config().get("js/state", "{\"ignored\":[]}").toString();
} QString JsState::get() const {
	return m_state;
} void JsState::set(const QString& state) {
	m_state = state;
	Config().set("js/state", m_state);
} void JsState::quote(QString str) {
	emit quoteReq(str);
}

/* ------------------------------
 * Utility functions
 * ----------------------------*/
void TopicView::jsEncode(QString& str) {
	str.replace("src=\"//", "src=\"http://");
	str.replace('\\', "\\\\");
	str.replace('\"', "\\\"");
	str.replace('\n', "\\n");
	str.replace('\r', "\\r");
}
void TopicView::htmlDecode(QString& str) {
	str.replace("&amp;", "&");
	str.replace("&lt;", "<");
	str.replace("&gt;", ">");
	str.replace("&nbsp;", " ");
	str.replace("&copy;", "©");
	str.replace("&quot;", "\"");
	str.replace("&reg;", "®");
	str.replace("&apos;", "'");
}
void TopicView::htmlEncode(QString& str) {
	str.replace("&", "&amp;");
    str.replace("<", "&lt;");
    str.replace(">", "&gt;");
    str.replace(" ", "&nbsp;");
    str.replace("©", "&copy;");
    str.replace("\"", "&quot;");
    str.replace("®", "&reg;");
    str.replace("'", "&apos;");
}

void TopicView::htmlToMarkDown(QString& str) {
	jsEncode(str);
	d.silent(str);
	str = m_htmlView->page()->mainFrame()->evaluateJavaScript(QString("JVCode.toJVCode(\"%1\")").arg(str)).toString();
}

void TopicView::wrap(QWebElement& doc, QString s, QString a, QString b, bool nl) {
	htmlEncode(a); htmlEncode(b);
	QWebElementCollection els = doc.findAll(s);
	if(nl) b += "<br>";
	for(int i(0); i < els.count(); ++i)
		els[i].setInnerXml(a + els.at(i).toInnerXml() + b);
}

void TopicView::prefix(QWebElement& doc, QString s, QString pre) {
	htmlEncode(pre);
	QWebElementCollection q = doc.findAll(s), els;
	for(int i(0); i < q.count(); ++i) {
		els = q.at(i).findAll("br");
		for(int j(0); j < els.count(); ++j)
			els[j].setOuterXml("<br>" + pre);
		
		els = q.at(i).findAll("p");
		for(int j(0); j < els.count(); ++j) {
			els[j].prependInside(pre);
			els[j].setAttribute(
				"depth",
				QString::number(els.at(j).attribute("depth", "0").toInt()+1)
			);
		}
	}
}

void TopicView::processMsg(QString& msg) {
	QWebPage p; p.mainFrame()->setHtml(msg);
	QWebElement doc = p.mainFrame()->documentElement();
	QWebElementCollection jvcare = doc.findAll(".JvCare");
	
	const QString b("0A12B34C56D78E9F");
	
	for(int i(0); i < jvcare.count(); ++i) {
		if(jvcare.at(i).classes().count() < 1) {
			d << "Erreur jvcare";
			continue;
		}
		const QString s(jvcare.at(i).classes().at(1));
		QString u;
		for(int j(0); j < s.count(); j+=2) {
			const int ch(b.indexOf(s.at(j)));
			const int cl(b.indexOf(s.at(j+1)));
			u += char(ch*16+cl);
		}
		jvcare[i].setOuterXml(
			"<a href=\"" + u + "\">"
			+ u
			+ "</a>"
		);
	}
	
	msg = doc.findFirst("body").toInnerXml();
}

bool TopicView::isTopic(const QString& str) {
	return str.indexOf("http://www.jeuxvideo.com/forums/42") == 0
	|| str.indexOf("http://www.jeuxvideo.com/forums/1") == 0;
}
bool TopicView::isForum(const QString& str) {
	return str.indexOf("http://www.jeuxvideo.com/forums/0") == 0;
}
