#include "mainwindow.h"
#include "ui_mainwindow.h"

/* ------------------------
 * Code launched at app startup
 * ----------------------*/
MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	createGui();
	
	//Actions managing
	QAction* closeTopicView = new QAction(this);
	closeTopicView->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_W));
	addAction(closeTopicView);
	QAction* postMsg  = new QAction(this);
	QAction* postMsg2 = new QAction(this);
	postMsg->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Return));
	postMsg2->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Enter));
	addAction(postMsg); addAction(postMsg2);
	QAction* prevHistory = new QAction(this);
	QAction* nextHistory = new QAction(this);
	prevHistory->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Down));
	nextHistory->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Up));
	addAction(prevHistory); addAction(nextHistory);
	connect(closeTopicView, SIGNAL(triggered()), this, SLOT(closeCurrTopicView()));
	connect(postMsg, SIGNAL(triggered()), this, SLOT(on_buttonSend_pressed()));
	connect(postMsg2, SIGNAL(triggered()), this, SLOT(on_buttonSend_pressed()));
	connect(prevHistory, SIGNAL(triggered()), this, SLOT(getPrevHistory()));
	connect(nextHistory, SIGNAL(triggered()), this, SLOT(getNextHistory()));
	connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(close()));
	
	//Jvc handle managing
	m_jv = new Jvc();
	connect(m_jv, SIGNAL(posted()), m_textPost, SLOT(clear()));
	connect(m_textPost, SIGNAL(textChanged()), this, SLOT(on_textPost_textChanged()));
	connect(m_jv, SIGNAL(posted()), this, SLOT(pollTopics()));
	connect(m_jv, SIGNAL(connected()), this, SLOT(pollForms()));
	connect(m_jv, SIGNAL(newMsg()), this, SLOT(newMsg()));
	connect(m_jv, SIGNAL(setStatusRequest(QString)), this, SLOT(setStatus(QString)));
	m_jv->getVersion();
	heartBeat();
	pollBlacklistLoop();
	pollTopicsLoop();
	pollForumsLoop();
	pollFormsLoop();
	
	//Factory plugin server manage
	m_factory = new Factory(this);
	connect(m_factory, SIGNAL(request(QString)), m_dockNick, SLOT(addNick(QString)));
	m_factory->setup();
	
	//Updater plugin server manage
	m_updater = new Updater(this);
	connect(m_updater, SIGNAL(request(QString)), this, SLOT(update(QString)));
	m_updater->setup();
	
	show();
	firstSetup();
}
void MainWindow::createGui() {
	ui->setupUi(this);
	setWindowTitle("PogoIRC");
	QIcon ico(":/data/ico_x128.png");
	ico.addFile(":/data/ico_x64.png");
	ico.addFile(":/data/ico_x32.png");
	ico.addFile(":/data/ico_x16.png");
	setWindowIcon(ico);
	
	/* TEMPORARY - Disable unsupported widgets */
	ui->actionChangeForumViewUrl->setVisible(false);
	ui->actionForumReload->setVisible(false);
	ui->actionNewForumView->setVisible(false);
	ui->menuForums->menuAction()->setVisible(false);
	ui->actionAbout->setVisible(false);
	
	//Log window managing
	m_log_wnd = new QWidget();
	m_log_layout = new QVBoxLayout(m_log_wnd);
	m_log_txt = new QPlainTextEdit(m_log_wnd);
	m_log_wnd->setObjectName("LogWnd");
	m_log_txt->setReadOnly(true);
	m_log_txt->setMaximumBlockCount(Config().get("log/maxLogs", 0).toInt());
	m_log_layout->addWidget(m_log_txt);
	m_log_layout->setMargin(0);
	//Error feedback
	connect(&d, SIGNAL(console(QString)), m_log_txt, SLOT(appendPlainText(QString)));
	connect(&d, SIGNAL(log(QString)), this, SLOT(log(QString)));
	connect(&d, SIGNAL(msgBox(QString)), this, SLOT(msgBox(QString)));
	
	setDockNestingEnabled(true);
	ui->mainWidget->setLayout(ui->layoutMain);
	ui->tags->setLayout(ui->layoutTags);
	
	m_textPost = new TextEdit();
	ui->layoutBot->insertWidget(0, m_textPost);
	m_ver = new QLabel("PogoIRC par Angivare pour l'AAS", ui->status);
	ui->status->addPermanentWidget(m_ver);
	m_dockNick = new DockNick(this);
	this->addDockWidget(Qt::LeftDockWidgetArea, m_dockNick);
	m_topicViewArea = new QTabWidget(this);
	m_topicViewArea->setObjectName("topicViewArea");
	m_topicViewArea->setMovable(true);
	m_topicViewArea->setTabsClosable(true);
	connect(m_topicViewArea, SIGNAL(currentChanged(int)), this, SLOT(refreshTopicFocus(int)));
	connect(m_topicViewArea, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTopicView(int)));
	ui->layoutMain->insertWidget(0, m_topicViewArea, 100);
	ui->layoutMain->setStretchFactor(ui->layoutBot, 10);
	ui->layoutMain->setSpacing(0);
	m_textPost->setProperty("class", "not-editing");
	
	setTabOrder(m_textPost, ui->buttonSend);
	
	//Load state
	Config c;
	c.setTheme(c.get("theme", "base").toString());
	on_actionThemeRefresh_triggered();
	for(int i(0); i < c.get("topics/topicViewCount",0).toInt(); ++i)
		addTopicView();
	QStringList urls(c.get("topics/topicViewUrls").toStringList());
	for(int i(0); i < m_topicViewList.count(); ++i) {
		(*m_topicViewList.at(i))->setUrl(urls.at(i));
		d.silent("Added " + QString::number(i+1) + "th topic's url: " + urls.at(i));
	}
	for(int i(0); i < c.get("forums/forumViewCount",0).toInt(); ++i)
		addForumView();
	urls = c.get("forums/forumViewUrls").toStringList();
	for(int i(0); i < m_forumViewList.count(); ++i) {
		(*m_forumViewList.at(i))->setUrl(urls.at(i));
		d.silent("Added " + QString::number(i+1) + "th forum's url: " + urls.at(i));
	}
	
	m_menuFavSave = new QMenu("Sauvegarder", ui->menuFav);
	m_menuFavLoad = new QMenu("Charger", ui->menuFav);
	ui->menuFav->addMenu(m_menuFavSave);
	ui->menuFav->addMenu(m_menuFavLoad);
	for(int i(0); i < c.get("options/favCount", 9).toInt(); ++i)
	{ //Create save/load actions
		QString name = "Favori n°"+QString::number(i+1);
		if(QString() != c.get("favs/fav"+QString::number(i), QString()).toString()) {
			name = c.get("favs/fav"+QString::number(i)).toString();
			name = name.left(name.lastIndexOf("."));
			int n = 0;
			for(int i(0); i < 7; i++) n = name.indexOf('-', n+1);	
			name = name.right(name.size() - n - 1);
		}
		if(i < 9) {
			m_menuFavSave->addAction(name, this, SLOT(favSave()),
			QKeySequence(Qt::CTRL + (Qt::Key_1 + i)));
			m_menuFavSave->actions().last()->setData(i);
			m_menuFavLoad->addAction(name, this, SLOT(favLoad()),
			QKeySequence(Qt::CTRL + (Qt::Key_F1 + i)));
		}
		m_menuFavLoad->actions().last()->setData(i);
	}
	
	qApp->setStyleSheet(c.getStyle());
	
	QRect geom = c.get("wnd/windowGeom", QRect()).toRect();
	if(geom != QRect()) setGeometry(geom);
	restoreState(c.get("wnd/windowState").toByteArray());
	QRect logGeom = c.get("log/windowGeom", QRect()).toRect();
	if(logGeom != QRect()) m_log_wnd->setGeometry(logGeom);
//	m_topicViewArea->restoreState(c.get("wnd/topicViewAreaState").toByteArray());
	m_log_wnd->setVisible(!c.get("wnd/hideConsole",false).toBool());
	ui->actionShowConsole->setChecked(!m_log_wnd->isHidden());
	m_dockNick->showNick(!c.get("wnd/hideNicks",false).toBool());
	ui->actionShowNick->setChecked(!m_dockNick->getNickHidden());
	m_dockNick->showPass(!c.get("wnd/hidePasses",false).toBool());
	ui->actionShowPass->setChecked(!m_dockNick->getPassHidden());
	on_actionShowSend_toggled(!c.get("wnd/hideSendButton", false).toBool());
	ui->actionShowSend->setChecked(!ui->buttonSend->isHidden());
	on_actionShowTags_toggled(!c.get("wnd/hideTags", false).toBool());
	ui->actionShowTags->setChecked(!ui->tags->isHidden());
	if(c.get("wnd/hideDockNick",false).toBool())
		m_dockNick->hide();
	ui->actionEnableAlert->setChecked(c.get("notif/alert").toBool());
	ui->actionEnableSound->setChecked(c.get("notif/sound").toBool());
}
MainWindow::~MainWindow()
{
	delete m_jv;
	delete ui;
}
void MainWindow::firstSetup()
{
	Config c;
	if(c.get("firstLaunch", true).toBool()) {
		ui->actionShowPass->setChecked(false);
	}
	if(c.get("credentials/defaultNick", QString()).toString() == QString()) {
		on_actionNewNick_triggered();
		m_dockNick->selectNick(0);
	}
	if(c.get("credentials/defaultPass", QString()).toString() == QString()) {
		on_actionNewPass_triggered();
		m_dockNick->selectPass(0);
	}
	if(!c.get("topics/topicViewCount", 0).toInt())
		on_actionNewTopicView_triggered();
	
	c.set("firstLaunch", false);
}

/* -------------------------------
 * Stuff that makes GUI work
 * -----------------------------*/
void MainWindow::on_buttonSend_pressed() {
	TopicViewH* topic = currView(); if(!topic) return;
	if(!(*topic)->getHistory().id)
	{ //Simple post message
		if(!m_jv->connected(m_dockNick->getNick()))
		{
//			d.silent(m_dockNick->getNick() + ':' + m_dockNick->getPass());
			m_jv->connect(m_dockNick->getNick(), m_dockNick->getPass(), 
	m_textPost->toPlainText(), topic);
		} else if(m_textPost->toPlainText() != QString()){
			m_jv->postMsg(m_textPost->toPlainText(), topic);
		}
	} else { //Edit message
		m_jv->editMsg((*topic)->getHistory().id, m_textPost->toPlainText(), topic);
	}		
}
void MainWindow::on_actionUpdate_triggered() {
	m_jv->update();
}
void MainWindow::on_actionAbout_triggered() {
	QMessageBox::about(this, "À propos",
	"Créé par Angivare");
}
void MainWindow::on_actionChangeTopicViewUrl_triggered() {
	TopicViewH* topic = currView(); if(!topic) return;
	bool ok;
	QString url = QInputDialog::getText(this, "Choix du topic", "Entrez l'URL du topic:", QLineEdit::Normal, "http://", &ok);
	if(ok)
	{
		(*topic)->setUrl(url);
		pollTopics();
		pollForm(topic);
	}
}
void MainWindow::on_actionNewTopicView_triggered() { 
	addTopicView();
	on_actionChangeTopicViewUrl_triggered();
}
void MainWindow::on_actionChangeForumViewUrl_triggered() {
	ForumViewH* forum = currForum(); if(!forum) return;
	bool ok;
	QString url = QInputDialog::getText(this, "Choix du forum", "Entrez l'URL du forum:", QLineEdit::Normal, "http://", &ok);
	if(ok)
	{
		(*forum)->setUrl(url);
		pollForums();
	}
}
void MainWindow::on_actionNewForumView_triggered() {
	qDebug() << "dp";
	addForumView();
	on_actionChangeForumViewUrl_triggered();
}
void MainWindow::on_actionNewNick_triggered() {
	bool ok;
	QString data = QInputDialog::getText(this, "Nouveau pseudo", "Entrez votre pseudo: (ou pseudo:mdp)", QLineEdit::Normal, QString(), &ok);
	if(data != QString()) m_dockNick->addNick(data);
}
void MainWindow::on_actionNewPass_triggered() {
	bool ok;
	QString data = QInputDialog::getText(this, "Nouveau mdp", "Entrez votre mot de passe: (ou alias:mdp)", QLineEdit::Password, QString(), &ok);
	if(data != QString()) m_dockNick->addPass(data);
}
void MainWindow::on_actionReload_triggered() {
	(*currView())->setUrl((*currView())->getUrl());
}
void MainWindow::on_actionForumReload_triggered() {
	pollForums();
}
void MainWindow::on_actionThemeRefresh_triggered() {
	QDir dir(QCoreApplication::applicationDirPath () + "/data/themes");
	QStringList themes = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
	ui->menuLoad->clear();
	for(int i(0); i < themes.count(); ++i)
	{	//Fill theme load menu
		ui->menuLoad->addAction(themes.at(i), this, SLOT(openTheme()));
		ui->menuLoad->actions().last()->setCheckable(true);
		if(themes.at(i) == Config::getTheme())
			ui->menuLoad->actions().last()->setChecked(true);
	}
}
void MainWindow::on_actionShowConsole_triggered() {
	m_log_wnd->setVisible(!m_log_wnd->isVisible());
}
void MainWindow::on_actionShowDockNick_triggered() {
	m_dockNick->setVisible(!m_dockNick->isVisible());
}
void MainWindow::on_actionShowNick_toggled(bool show) { m_dockNick->showNick(show); m_dockNick->show(); }
void MainWindow::on_actionShowPass_toggled(bool show) { m_dockNick->showPass(show); m_dockNick->show(); }
void MainWindow::on_actionShowTags_toggled(bool show) { 
	if(show) ui->tags->show();
	else ui->tags->hide();
}
void MainWindow::on_actionShowSend_toggled(bool show) { 
	if(show) ui->buttonSend->show();
	else ui->buttonSend->hide();
}
void MainWindow::on_actionEnableSound_toggled(bool enable) {
	Config().set("notif/sound", enable);
}
void MainWindow::on_actionEnableAlert_toggled(bool enable) {
	Config().set("notif/alert", enable);
}

/* ---------------------------
 * Tags buttons management
 * -------------------------*/
void MainWindow::on_tagBold_pressed() { addText("''' '''"); }
void MainWindow::on_tagItalic_pressed() { addText("'' ''"); }
void MainWindow::on_tagUnderline_pressed() { addText("<u> </u>"); }
void MainWindow::on_tagCrossed_pressed() { addText("<s> </s>"); }
void MainWindow::on_tagList_pressed() { addText("*"); }
void MainWindow::on_tagNumberedList_pressed() { addText("#"); }
void MainWindow::on_tagQuote_pressed() { addText(">"); }
void MainWindow::on_tagCode_pressed() { addText("<code> </code>"); }
void MainWindow::on_tagSpoil_pressed() { addText("<spoil> </spoil>"); }
void MainWindow::addText(QString tag) {
	d.silent(tag);
	int beg = m_textPost->textCursor().position();
	int end = m_textPost->textCursor().anchor();
	if(end < beg) { //then reorder it
		int temp = end;
		end = beg;
		beg = temp;
	}
	QString str = m_textPost->toPlainText();
	if(tag.contains(' ')) { //Add half tag to one end and other half to other end
		QString first = tag.left(tag.indexOf(' '));
		QString last = tag.right(tag.size() - tag.indexOf(' ') - 1);
		QString toShow;
		toShow = str.left(beg); toShow += first;
		toShow += str.left(end).right(end-beg);
		toShow += last; toShow += str.right(str.size() - end);
		QTextCursor curs = m_textPost->textCursor();
		curs.select(QTextCursor::Document);
		curs.insertText(toShow);
		curs.setPosition(beg + first.size());
		m_textPost->setTextCursor(curs);
	} else { //Add tag to each selected line
		str = str.left(end+1);
		QTextStream s(&str);
		int lEnd = 0;
		while(!s.atEnd()) { s.readLine(); lEnd++; }
		str = str.left(beg+1);
		s.seek(0); int lBeg = 0;
		while(!s.atEnd()) { s.readLine(); lBeg++; }
		str = m_textPost->toPlainText();
		s.seek(0);
		QString toShow;
		for(int i(0); i < lBeg-1; ++i) toShow += s.readLine() + '\n';
		for(; lBeg <= lEnd; ++lBeg) {
			toShow += tag + ' ';
			toShow += s.readLine() + '\n';
		} toShow += s.readAll();
		QTextCursor curs = m_textPost->textCursor();
		curs.select(QTextCursor::Document);
		curs.insertText(toShow);
		curs.setPosition(beg + tag.size() + 1);
		m_textPost->setTextCursor(curs);
	}
}

//---------------------------------------------------
void MainWindow::openTheme() {
	Config c;
	QAction* act = (QAction*) sender();
	c.setTheme(act->text());
	c.set("theme", act->text());
	
	for(int i(0); i < ui->menuLoad->actions().count(); ++i)
		if(ui->menuLoad->actions().at(i)->text() == act->text())
			ui->menuLoad->actions()[i]->setChecked(true);
		else
			ui->menuLoad->actions()[i]->setChecked(false);
	
	qApp->setStyleSheet(c.getStyle());
	for(int i(0); i < m_topicViewList.count(); ++i)
		(*m_topicViewList[i])->setUrl((*m_topicViewList.at(i))->getUrl());
}

void MainWindow::favSave() {
	TopicViewH* topic = currView(); if(!topic) return;
	QAction* act = (QAction*) sender();
	QString str = act->data().toString();
	QString url = (*topic)->getUrl();
	Config().set("favs/fav"+str, url);
	url = url.left(url.lastIndexOf("."));
	int n = 0;
	for(int i(0); i < 7; i++) n = url.indexOf('-', n+1);	
	url = url.right(url.size() - n - 1);
	act->setText(url);
	for(int i(0); i < m_menuFavLoad->actions().count(); ++i)
		if(m_menuFavLoad->actions().at(i)->data() == act->data())
			m_menuFavLoad->actions()[i]->setText(url);
}

void MainWindow::favLoad() {
	QAction* act = (QAction*) sender();
	QString str = act->data().toString();
	str = Config().get("favs/fav"+str, "http://").toString();
	addTopicView(str);
}

void MainWindow::on_textPost_textChanged() {
	if(m_textPost->toPlainText().isEmpty()) {
		for(int i(0); i < m_topicViewList.count(); ++i)
			(*m_topicViewList[i])->putFirstHistory();
		m_textPost->setProperty("class", "not-editing");
		style()->unpolish(m_textPost);
		style()->polish(m_textPost);
	}
}

void MainWindow::getPrevHistory() {
	post p = (*currView())->getPrevHistory();
	if(-1 == (*currView())->getHistoryIndex())
		m_textPost->setPlainText(m_lastTxt);
	else
		m_textPost->setPlainText(p.msg);
	m_textPost->moveCursor(QTextCursor::End);
	if(!p.id)
		m_textPost->setProperty("class", "not-editing");
	else
		m_textPost->setProperty("class", "editing");
	style()->unpolish(m_textPost);
	style()->polish(m_textPost);
}
void MainWindow::getNextHistory() {
	post p = (*currView())->getNextHistory();
	if(!p.id) return;
	if(!(*currView())->getHistoryIndex())
		m_lastTxt = m_textPost->toPlainText();
	m_textPost->setPlainText(p.msg);
	m_textPost->moveCursor(QTextCursor::End);
	if(!p.id)
		m_textPost->setProperty("class", "not-editing");
	else
		m_textPost->setProperty("class", "editing");
	style()->unpolish(m_textPost);
	style()->polish(m_textPost);
}


void MainWindow::closeEvent(QCloseEvent* ev) {
	//Save application state
	Config c;
	c.set("wnd/windowGeom", geometry());
	c.set("log/windowGeom", m_log_wnd->geometry());
	c.set("wnd/windowState", saveState());
	c.set("topics/topicViewCount", m_topicViewList.count());
	c.set("forums/forumViewCount", m_forumViewList.count());
	QStringList urls;
	for(int i(0); i < m_topicViewArea->count(); ++i)
		urls += ((TopicView*)m_topicViewArea->widget(i))->getUrl();
	c.set("topics/topicViewUrls", urls);
	urls.clear();
	for(int i(0); i < m_forumViewList.count(); ++i)
		urls += (*m_forumViewList.at(i))->getUrl();
	c.set("forums/forumViewUrls", urls);
	c.set("credentials/defaultNick",m_dockNick->getNickAlias());
	c.set("credentials/defaultPass",m_dockNick->getPassAlias());
	m_dockNick->saveNick(); m_dockNick->savePass();
	c.set("wnd/hideConsole", m_log_wnd->isHidden());
	c.set("wnd/hideNicks", m_dockNick->getNickHidden());
	c.set("wnd/hidePasses", m_dockNick->getPassHidden());
	c.set("wnd/hideSendButton", ui->buttonSend->isHidden());
	c.set("wnd/hideTags", ui->tags->isHidden());
	c.set("wnd/hideDockNick", m_dockNick->isHidden());
	QMainWindow::closeEvent(ev);
	qApp->quit();
}

/* ---------------------------
 * Other stuff
 * --------------------------*/
void MainWindow::setStatus(QString status) {
	m_ver->setText(status);
}
void MainWindow::newMsg() {
	Config c;
	if(c.get("notif/sound", true).toBool())
		QSound::play(Config().getNotifSound());
	if(c.get("notif/alert", true).toBool())
		qApp->alert(this);	
}

void MainWindow::heartBeat() {
	m_jv->heartBeat();
	QTimer::singleShot(1000*60*30, this, SLOT(heartBeat()));
}

void MainWindow::pollTopics() {
	for(int i(0); i < m_topicViewList.count(); ++i)
		if(TopicView::isTopic((*m_topicViewList.at(i))->getUrl()))
			m_jv->getMsg((*m_topicViewList.at(i))->getUrl(), m_topicViewList[i]);
}
void MainWindow::pollTopicsLoop() {
	pollTopics();
	QTimer::singleShot(Config().get("options/pollRate",3000).toInt(), this, SLOT(pollTopicsLoop()));
}
void MainWindow::pollForum(ForumView* forum) {
	d.silent("Forum refresh");
	for(int i(0); i < m_forumViewList.count(); ++i)
		if(*m_forumViewList.at(i) == forum)
			m_jv->getTopic(forum->getUrl(), m_forumViewList[i]);
}
void MainWindow::pollForums() {
	for(int i(0); i < m_forumViewList.count(); ++i)
		m_jv->getTopic((*m_forumViewList.at(i))->getUrl(), m_forumViewList[i]);
}
void MainWindow::pollForumsLoop() {
	pollForums();
	if(!Config().get("options/forumsPollRate",0).toInt()) return;
	QTimer::singleShot(Config().get("options/forumsPollRate").toInt(), this, SLOT(pollForumsLoop()));
}
void MainWindow::pollForm(TopicViewH* topic) { m_jv->prepareForm(topic); }
void MainWindow::pollForms() {
	for(int i(0); i < m_topicViewList.count(); ++i)
		pollForm(m_topicViewList[i]);
}
void MainWindow::pollFormsLoop() {
	pollForms();
	QTimer::singleShot(1200000, this, SLOT(pollFormsLoop()));
}
void MainWindow::pollBlacklistLoop() { m_jv->blacklist(); }

void MainWindow::closeCurrTopicView() { closeTopicView(TopicView::getFocusIndex()); }
void MainWindow::closeTopicView(int i) {
	for(int n(0); n <  m_topicViewList.count(); ++n)
		if((*m_topicViewList.at(n)) == m_topicViewArea->widget(i)) {
			(*m_topicViewList[n])->deleteLater();
			*m_topicViewList[n] = NULL;
			m_topicViewList.removeAt(n);
			m_topicViewArea->widget(i)->close();
			d.silent("Removed " + QString::number(n) + "th TopicView");
		}
}
void MainWindow::closeForumView(ForumView* it) {
	for(int n(0); n <  m_forumViewList.count(); ++n)
		if(*m_forumViewList.at(n) == it) {
			(*m_forumViewList[n])->deleteLater();
			*m_forumViewList[n] = NULL;
			m_forumViewList.removeAt(n);
			d.silent("Removed ForumView");
		}
}
void MainWindow::addForumView() {
	d.silent("addForumView()");
	QMainWindow* parent = this;
	ForumViewH* item = new ForumViewH;
	*item = new ForumView();
	(*item)->setParent(parent);
	m_forumViewList.push_back(item);
	parent->addDockWidget(Qt::TopDockWidgetArea, *m_forumViewList.last());
	connect(*m_forumViewList.last(), SIGNAL(closeRequest(ForumView*)), this, SLOT(closeForumView(ForumView*)));
	connect(*m_forumViewList.last(), SIGNAL(topic(QString)),  this, SLOT(addTopicView(QString)));
	connect(*m_forumViewList.last(), SIGNAL(refreshRequest(ForumView*)), this, SLOT(pollForum(ForumView*)));
	d.silent("Added " + QString::number(m_forumViewList.size()) + "th ForumView");
	(*m_forumViewList.last())->raise();
}
void MainWindow::addTopicView() {
	d.silent("addTopicView()");
	TopicViewH* item = new TopicViewH;
	*item = new TopicView(); 
	(*item)->setParent(m_topicViewArea);
	m_topicViewList.push_back(item);
	m_topicViewArea->addTab(*m_topicViewList.last(), "Chargement...");
	m_topicViewArea->setCurrentWidget(*m_topicViewList.last());
	connect(*m_topicViewList.last(), SIGNAL(closeRequest(int)), this, SLOT(closeTopicView(int)));
	connect(*m_topicViewList.last(), SIGNAL(repaintRequest(int)), this, SLOT(repaintTopicView(int)));
	connect(*m_topicViewList.last(), SIGNAL(resizeHackReq()), this, SLOT(resizeHack()));
	connect(*m_topicViewList.last(), SIGNAL(titleRequest(QWidget*,QString&)), this, SLOT(refreshTopicTitle(QWidget*,QString&)));
	connect(*m_topicViewList.last(), SIGNAL(messRequest(QWidget*)), this, SLOT(refreshMess(QWidget*)));
	connect(*m_topicViewList.last(), SIGNAL(quoteReq(QString&)), this, SLOT(quote(QString&)));
	d.silent("Added " + QString::number(m_topicViewList.size()) + "th TopicView");
}
void MainWindow::addTopicView(QString url) {
	addTopicView();
	(*m_topicViewList.last())->setUrl(url);
}
void MainWindow::refreshTopicTitle(QWidget* w, QString& t) {
	m_topicViewArea->setTabText(m_topicViewArea->indexOf(w), t);
}
void MainWindow::refreshMess(QWidget* w) {
	int i = m_topicViewArea->indexOf(w);
	if(m_topicViewArea->currentIndex() != i)
		m_topicViewArea->setTabIcon(i, QIcon(Config().getNotifImage()));
}
void MainWindow::refreshTopicFocus(int i) {
	m_topicViewArea->setTabIcon(i, QIcon());
}

void MainWindow::repaintTopicView(int i) {
	for(int n(0); n < m_topicViewList.count(); ++n)
		(*m_topicViewList.at(n))->repaint();
}
void MainWindow::resizeHack() {
	QRect geom = geometry();
	geom.setHeight(geom.height()+1);
	setGeometry(geom);
	QTimer::singleShot(100, this, SLOT(resizeHackFinish()));
}
void MainWindow::resizeHackFinish() {
	QRect geom = geometry();
	geom.setHeight(geom.height()-1);
	setGeometry(geom);
}
void MainWindow::quote(QString& str) {
//	TopicView::htmlToMarkDown(str);
	
	str.prepend(">");
	str.replace("\n", "\n>");
	str.append("\n\n");
	
	int beg = m_textPost->textCursor().position();
	int end = m_textPost->textCursor().anchor();
	if(end < beg) { //then reorder it
		int temp = end;
		end = beg;
		beg = temp;
	}
	
	QString txt = m_textPost->toPlainText();
	QString left = txt.left(beg);
	QString right = txt.right(txt.length() - end);
	txt = left + str + right;
	m_textPost->setFocus();
	m_textPost->setPlainText(txt);
	QTextCursor curs = m_textPost->textCursor();
	curs.setPosition(txt.length());
	m_textPost->setTextCursor(curs);
}

/* --------------------------
 * Slots that gives graphical 
 * feedback to the user
 * -------------------------*/
void MainWindow::log(const QString& data) {
	ui->status->showMessage(data, 5000);
}
void MainWindow::msgBox(const QString& data) {
	//Show dialog box with data as text
	QMessageBox* msgBox = new QMessageBox(this);
	msgBox->setText(data);
	msgBox->exec();
}

/* ----------------
 * Utility
 * ---------------*/
void MainWindow::raiseDock() { (*currView())->raise(); }

ForumViewH* MainWindow::currForum() {
	int i = ForumView::getFocusIndex();
	for(int n(0); n < m_forumViewList.count(); ++n)
		if((*m_forumViewList.at(n))->getIndex() == i)
			return m_forumViewList.at(n);
	return NULL;
}
TopicViewH* MainWindow::currView() {
	for(int n(0); n < m_topicViewList.count(); ++n)
		if((*m_topicViewList.at(n)) == m_topicViewArea->currentWidget())
			return m_topicViewList.at(n);
	return NULL;
}

void MainWindow::update(QString) {
	qApp->quit();
}
