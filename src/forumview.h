#ifndef FORUMVIEW_H
#define FORUMVIEW_H

#include <QWidget>
#include <QDockWidget>
#include <QListWidget>
#include <QHBoxLayout>

#include "config.h"
#include "dbg.h"

class ForumView : public QDockWidget
{
	Q_OBJECT
public:
	explicit ForumView(QWidget *parent = 0);
	
	void refresh(QMap<QString,QStringList> topics);
	
	void setUrl(QString);
	QString getUrl();
	void setTitle(QString);
	QString getTitle();
	
	int getIndex();
	static int getFocusIndex();
	
signals:
	void topic(QString);
	void closeRequest(ForumView*);
	void refreshRequest(ForumView*);
	
public slots:
	
protected:
	bool eventFilter(QObject*, QEvent*);
	void closeEvent(QCloseEvent*);
	
private slots:
	void openTopic(QListWidgetItem*);
	void setFocus(bool);
	
private:
	//Gui
	QListWidget* m_list;
	
	QString m_url;
	
	int m_index;
	static int s_nextIndex;
	static int s_currIndex;
};
typedef ForumView* ForumViewH;

#endif // FORUMVIEW_H
