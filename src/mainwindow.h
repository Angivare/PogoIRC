#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QMessageBox>
#include <QInputDialog>
#include <QTimer>
#include <QDesktopServices>
#include <QKeyEvent>
#include <QMenu>
#include <QLabel>
#include <QDir>
#include <QFont>
#include <QIcon>
#include <QVBoxLayout>
#include <QRegExp>
#include <QTextDocument>

#include "data_structs.h"
#include "config.h"
#include "docknick.h"
#include "topicview.h"
#include "jvc.h"
#include "dbg.h"
#include "modules.h"
#include "textedit.h"

namespace Ui {
class MainWindow;
}

/*----------------------------------------------------
 * CLASS MAINWINDOW: main UI class, the whole data of
 * the program is stocked directly or  indirectly in 
 * one of its members.
 *---------------------------------------------------*/
class MainWindow : public QMainWindow
{
	Q_OBJECT
	
public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();
	
public slots:
	void log(const QString&);
	void msgBox(const QString&);
	
	void update(QString);
	
	void on_buttonSend_pressed();
	
	void on_actionUpdate_triggered();
	void on_actionAbout_triggered();
	void on_actionChangeTopicViewUrl_triggered();
	void on_actionNewTopicView_triggered();
	void on_actionChangeForumViewUrl_triggered();
	void on_actionNewForumView_triggered();
	void on_actionNewNick_triggered();
	void on_actionNewPass_triggered();
	void on_actionReload_triggered();
	void on_actionForumReload_triggered();
	void on_actionThemeRefresh_triggered();
	void on_actionShowConsole_triggered();
	void on_actionShowDockNick_triggered();
	void on_actionShowNick_toggled(bool);
	void on_actionShowPass_toggled(bool);
	void on_actionShowTags_toggled(bool);
	void on_actionShowSend_toggled(bool);
	void on_actionEnableSound_toggled(bool);
	void on_actionEnableAlert_toggled(bool);
	
	//Tags
	void on_tagBold_pressed();
	void on_tagItalic_pressed();
	void on_tagUnderline_pressed();
	void on_tagCrossed_pressed();
	void on_tagList_pressed();
	void on_tagNumberedList_pressed();
	void on_tagQuote_pressed();
	void on_tagCode_pressed();
	void on_tagSpoil_pressed();
	
	void openTheme();
	
	void favSave();
	void favLoad();
	
	void on_posted();
	
	void getPrevHistory();
	void getNextHistory();
	
	void setStatus(QString);
	void pollForum(ForumView*);
	void pollForums();
	void pollTopics();
	void pollForm(TopicViewH*);
	void pollForms();
	void newMsg();
	void closeCurrTopicView();
	void closeTopicView(int);
	void closeForumView(ForumView*);
	void repaintTopicView(int);
	void resizeHack();
	void resizeHackFinish();
	
protected:
	void closeEvent(QCloseEvent*);
	
private slots:
	void heartBeat();
	void pollForumsLoop();
	void pollTopicsLoop();
	void pollFormsLoop();
	void pollBlacklistLoop();
	void createGui();
	void firstSetup();
	void addForumView();
	void addTopicView();
	void addTopicView(QString);
	void refreshTopicTitle(QWidget*,QString&);
	void refreshMess(QWidget*);
	void refreshTopicFocus(int);
	void raiseDock();
	void quote(QString&);
	
private:
	ForumViewH* currForum();
	TopicViewH* currView();
	void addText(QString);
	
	//Gui
	Ui::MainWindow* ui;
	TextEdit* m_textPost;
	QLabel* m_ver;
	QTabWidget* m_topicViewArea;
	QList<TopicViewH*> m_topicViewList;
	QList<ForumViewH*> m_forumViewList;
	DockNick* m_dockNick;
	QMenu* m_menuFavSave;
	QMenu* m_menuFavLoad;
	
	QString m_lastTxt;
	
	//Console GUI
	QWidget* m_log_wnd;
	QPlainTextEdit* m_log_txt;
	QVBoxLayout* m_log_layout;
	
	Jvc* m_jv;
	Factory* m_factory;
	Updater* m_updater;
};

#endif // MAINWINDOW_H
