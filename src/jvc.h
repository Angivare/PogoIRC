#ifndef JVC_H
#define JVC_H

#define OFFSET 1000
#define MSG_DELAY 10
#define TIMEOUT 60000

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkCookieJar>
#include <QNetworkCookie>
#include <QTimer>
#include <QSound>
#include <QApplication>
#include <QInputDialog>
#include <QBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QCryptographicHash>
#include <ctime>
#include <QPair>
#include <QTime>
#include <QBuffer>
#include <QProcess>

#include "parser.h"
#include "topicview.h"
#include "forumview.h"
#include "dbg.h"

/*--------------------------------------------
 *	Wrappers, to send signals with additional
 *	defined data (index of current callchain)
 *-------------------------------------------*/
class Response : public QObject {
	Q_OBJECT
public:
	Response(QNetworkReply*, int i = 0);
	~Response();
	QNetworkReply* getRep();
	void setData(void*);
	void* getData();
signals:
	void finished(int, Response*);
private slots:
	void finish();
private:
	QNetworkReply*	m_rep;
	int				m_i;
	void*			m_data;
};
struct msgData {
	QMap<QString,QStringList>*	params;
	int							n;
};
class Timer : public QObject {
	Q_OBJECT
public:
	Timer(QTimer*, Response*, int i = 0);
	~Timer();
	QTimer* getTimer();
	Response* getRep();
signals:
	void timeout(int, Response*);
	void timeout(int);
private slots:
	void finish();
private:
	QTimer*			m_timer;
	Response*		m_resp;
	int				m_i;
};

/*----------------------------------------------
 * JVC CLASS: handles every operations related with
 * jeuxvideo.com
 * May possibly handle other small internet operations
 *--------------------------------------------*/
class Jvc : public QObject
{
	Q_OBJECT
public:
	explicit Jvc(QObject* parent = 0);
	void getVersion(); //GET request to angiva.re
	void update(); //GET request to angiva.re
	void heartBeat(); //GET request to angiva.re
	void blacklist(); //GET request to angiva.re
	
	bool connected(QString);
	bool isConnected();
	void disconnect();
	void connect(QString nick, QString pass,
		QString toPost = QString(), TopicViewH* = NULL); //Random GET req to get cookie
	
	void prepareForm(TopicViewH*, QString msg = QString());
	
	int prePostMsg(QString msg, TopicViewH*);
	void postMsgFromForm(QString msg, TopicViewH*);
	void postMsg(QString msg, TopicViewH*, bool init=true, int fromIndex=0);//GET request to get tk
	
	void editMsg(int id, QString msg, TopicViewH*);//GET request to get tk
	
	void getMsg(QString url, TopicViewH*); //Get topic first page
	
	void getTopic(QString url, ForumViewH*);
	
	static void processMsg(QString&);
	
private slots:
	void getVersion1(int, Response*); //Emit setVersionRequest signal
	
	void update1(int, Response*); //Save to file, shutdown & exec interverting script
	
	void blacklist1(int, Response*); //Parse blacklist & save to file
	
	void connect1(int, Response*); //GET request to get tk
	void connect2(int, Response*); //Wait (to not get busted as bot)
	void connect3(int, Response*); //Parse tk then send connection request
	void connect4(int, Response*); //Parse tk + signature
	void connect5(int, Response*); //Wait (to get the right image)
	void connect6(int, Response*); //Get captcha image & send the whole thing
	void connect7(int, Response*); //Verify server response

	void prepareForm1(int, Response*); //Parse form & store it
	
	void postMsgFromForm1(int);
	
	void postMsg1(int, Response*); //Wait
	void postMsg2(int, Response*); //Parse tk then send msg
	void postMsg3(int, Response*); //Verify server respons
	void postMsgCaptcha(int,Response*);

	void editMsg1(int, Response*); //Wait
	void editMsg2(int, Response*); //Req to get edit tk
	void editMsg3(int, Response*); //Parse tk then send msg
	void editMsg4(int, Response*); //Verify server response
	void editMsgCaptcha(int, Response*);
	
	void getMsg1(int, Response*);  //Parse topic & get link to last page, GET it
	void getMsg2(int, Response*);  //Read Response & call getMsg2(QString)
	void getMsg2(int, QString&);   //Parse last page & update TopicView
	void getMsgTimeout(int);
	
	void getTopic1(int, Response*);
	
	void err(QNetworkReply::NetworkError);
	
	
signals:
	void posted();
	void connected();
	void newMsg();
	void setStatusRequest(QString);
	
private:
	void connectErr();
	
	QString askCaptcha(Response*);
	bool addPost(posts* params, int n, TopicViewH*);
	
	QString getTk(Response*);
	QString getTk(QString); //Part of connect & postmsg callchains
	QString getTkAndSign(Response*, QString* = 0);
	QString getFieldVal(const QString& page, const char* name, int& index);
	QString getAjaxTk(Response*); //Part of editmsg callchain

	void parseHighDensity(const QString& page, posts* params);
	
	QString getTopicId(QString);
	QString getFirstPage(QString); //Gets first page of topic
	QString getForumFirstPage(QString);
	QString getTopicForm(QString);
	QString getTopicLastMsg(QString);
	QString& htmlDecode(QString&);
	
	QNetworkAccessManager*	m_http;
	QNetworkRequest			m_req;
	QList<Response*>		m_rep;

	QString					m_currNick;
	bool					m_connecting;
	time_t					m_lastPosted;
	
	QList<QPair <QString,QPair <QString,QString> > > m_avatarCache;
	
	//Connect callchain
	QMap<int,connect_data>		m_connect;
	//Prepareform callchain
	QMap<int,prepareForm_data>	m_prepare;
	//Postmsg callchain
	QMap<int,post_data>			m_post;
	//Editmsg callchain
	QMap<int,edit_data>			m_edit;
	//Getmsg
	QMap<int,getMsg_data>		m_topicView;
	//Gettopic
	QMap<int,ForumViewH*>		m_forumView;
};



#endif // JVC_H
