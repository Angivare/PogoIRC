#include "forumview.h"

int ForumView::s_nextIndex(0), ForumView::s_currIndex(-1);

ForumView::ForumView(QWidget *parent) :
QDockWidget(parent), m_index(s_nextIndex)
{
	s_nextIndex++;
	s_currIndex = m_index;
	setObjectName("ForumView"+QString::number(m_index));
	
	m_list = new QListWidget(this);
	this->setWidget(m_list);
	m_list->installEventFilter(this);
	
	connect(this, SIGNAL(visibilityChanged(bool)), this, SLOT(setFocus(bool)));
	connect(m_list, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(openTopic(QListWidgetItem*)));
}

void ForumView::refresh(QMap<QString, QStringList> topics) {
	m_list->clear();
//	m_list->addItems(topics["title"]);
	QString title;
	for(int i(0); i < topics["url"].count(); i++)  {
		if(topics["icon"].at(i) == "topic_marque_on")
			title = "_ ";
		else
			title = "";
		if(Config().get("forums/author", false).toBool())
			title += topics["author"].at(i) + " ";
		if(Config().get("forums/response_count", false).toBool())
			title += topics["response_count"].at(i) + " ";
		title += topics["title"].at(i);
		m_list->addItem(title);
		m_list->item(i)->setData(32, topics["url"].at(i));
	}
}
void ForumView::openTopic(QListWidgetItem* it) {
	d.silent(it->data(32).toString());
	emit topic(it->data(32).toString());
}

bool ForumView::eventFilter(QObject* obj, QEvent* ev) {
	Q_UNUSED(obj);
	//Check CTRL + W & CTRL + R
	if(ev->type() == QEvent::FocusIn)
	{
		s_currIndex = m_index;
	}
	else if(ev->type() == QEvent::KeyPress)
	{
		QKeyEvent *kEv = static_cast<QKeyEvent*>(ev);
		d.silent(kEv->text());
//		if(kEv->text() == QKeySequence("Ctrl+R").toString())
		if(kEv->matches(QKeySequence::Refresh))
		{ //reload
			emit refreshRequest(this);
		}
//		else if(kEv->text() == QKeySequence("Ctrl+W").toString())
		else if(kEv->matches(QKeySequence::Close))
		{ //close
			emit closeRequest(this);
		}
	}
	return false;
}

void ForumView::closeEvent(QCloseEvent* ev) {
	emit closeRequest(this);
	return QDockWidget::closeEvent(ev);
}

void ForumView::setFocus(bool set) {
	if(set) {
		m_list->setFocus(Qt::OtherFocusReason);
		s_currIndex = m_index;
	}
}

void ForumView::setUrl(QString url) {
	m_url = url;
}
QString ForumView::getUrl() { return m_url; }

void ForumView::setTitle(QString title) { setWindowTitle(title); }
QString ForumView::getTitle() { return windowTitle(); }

int ForumView::getIndex()		{ return m_index; }
int ForumView::getFocusIndex()	{ return s_currIndex; }
