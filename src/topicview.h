#ifndef TOPICVIEW_H
#define TOPICVIEW_H

#include <QApplication>
#include <QTabWidget>
#include <QWebView>
#include <QWebFrame>
#include <QWebElement>
#include <QEvent>
#include <QTimer>
#include <QDir>
#include <QDesktopServices>
#include <QAction>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QRegExp>

#ifdef Q_OS_WIN32
#include "windows.h"
#endif

#include "parser.h"
#include "data_structs.h"
#include "config.h"
#include "dbg.h"


//Stores javascript's state, JSON-encoded
class JsState : public QObject {
	Q_OBJECT
	Q_PROPERTY(QString state READ get WRITE set)
public:
	explicit JsState(QWidget* parent = 0);
	
	QString get() const;
	void set(const QString&);
	
public slots:
	void quote(QString);
	
signals:
	void quoteReq(QString&);
	
private:
	QString m_state;
};

/*-----------------------------------------
 * CLASS TOPICVIEW: represents a single UI 
 * displaying a single topic.
 *----------------------------------------*/
class TopicView : public QWidget
{
	Q_OBJECT
public:
	explicit TopicView(QWidget* parent = 0);
	~TopicView();
	bool refresh(posts*);
	bool addPost(posts*, post);
	void addHistory(post);
	
	void setForm(formCache);
	formCache getForm();
	void expireForm();
	bool isFormReady();
	
	void setUrl(QString);
	QString getUrl();
	
	void setData(QString title);
	QString getTitle();
	
	int getIndex();
	static int getFocusIndex();
	QWebView* getView();
	
	void htmlToMarkDown(QString& str);
	
	static bool isTopic(const QString&);
	static bool isForum(const QString&);
	
public slots:
	post getPrevHistory(); //starting from the top: when no history it will give the last
	post getNextHistory();
	post getHistory();
	void putFirstHistory();
	int getHistoryIndex();
	
	void setFocus(bool);
	
	void openLink(const QUrl&);
	void quote(QString&);
	
signals:
	void closeRequest(int);
	void repaintRequest(int);
	void resizeHackReq();
	void titleRequest(QWidget*,QString&);
	void messRequest(QWidget*);
	void quoteReq(QString&);
	
protected:
	bool eventFilter(QObject*, QEvent*);
	
private:
	static void jsEncode(QString&);
	static void htmlDecode(QString&);
	static void htmlEncode(QString&);
	static void wrap(QWebElement&, QString selector, QString, QString, bool newline = false);
	static void prefix(QWebElement&, QString selector, QString);
	static void processMsg(QString&);
	
	QWebView* m_htmlView;
	QString m_body, m_footer;
	QString m_html; //heavy data
	QString m_url;
	
	JsState m_state;
	
	bool m_unreadMessage;
	
	QList<int> m_idList;
	QList<QString> m_timeList;
	
	QList<post> m_history; //Posted messages list, id & msg
	int m_historyIndex;
	
	int m_index;
	static int s_nextIndex;
	static int s_currIndex;
	
	formCache m_form;
	bool m_hasForm;
};

class WebPage : public QWebPage {
	Q_OBJECT
public:
	explicit WebPage(QWidget* parent = 0);
protected:
	void javaScriptConsoleMessage (const QString& message, int line, const QString& id);
};

/*---------------------------------
 * Wrapper to hold the TopicView as
 * it can become invalid because of
 * asynchronous flow in program
 * The first pointer is a safe-guard
 * so you can check if the second 
 * pointer is valid.
 *-------------------------------*/
typedef TopicView* TopicViewH;

#endif // TOPICVIEW_H
