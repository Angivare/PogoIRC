#ifndef DOCKNICK_H
#define DOCKNICK_H

#include <QDockWidget>
#include <QListWidget>
#include <QLayout>
#include <QAction>

#include "config.h"
#include "dbg.h"

/*----------------------------------------------
 * DOCKNICK: UI class that represents the widget
 * where user can choose his nick/pass combination
 *--------------------------------------------*/
class DockNick : public QDockWidget
{
	Q_OBJECT
public:
	explicit DockNick(QWidget* parent = 0);
	
	void addNick(QString,QString);
	void addPass(QString,QString);
	
	void saveNick();
	void savePass();
	
	QString getNick();
	QString getPass();
	
	QString getNickAlias();
	QString getPassAlias();
	
	void selectNick(int);
	void selectPass(int);
	
	void showNick(bool);
	void showPass(bool);
	bool getNickHidden();
	bool getPassHidden();
	
public slots:
	void addNick(QString);
	void addPass(QString);
	
private slots:
	void delNick(QListWidgetItem*);
	void delPass(QListWidgetItem*);
	
private:
	QWidget*	 m_wrapper;
	QHBoxLayout* m_layout;
	QListWidget* m_nickList;
	QListWidget* m_passList;
};

#endif // DOCKNICK_H
