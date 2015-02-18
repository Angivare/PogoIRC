#ifndef DATA_STRUCTS_H
#define DATA_STRUCTS_H

#include <QString>

class TopicView;
class ForumView;
class Timer;

struct post {
	int id;
	QString avatar, rank, nick, lw_nick, date, time;
	QString msg, sign;
	QString edited, editDate, editTime, editNick;
};

struct posts {
	int forum, topic, page, type;
	int mp;
	int connected;
	bool newPost;
	QList<post> p;
};

struct formCache {
	QString tk;
	QString sign;
};

struct prepareForm_data {
	formCache form;
	TopicView** topicView;
	QString msg;
};

struct post_data {
	QByteArray tk;
	QString sign;
	QString msg, url;
	TopicView** topicView;
};

struct connect_data {
	QString nick, pass;
	QByteArray data;
	QString tk;
	
	post_data post;
};

struct edit_data {
	int id, topicId;
	QString msg, tk;
	TopicView** topicView;
};

struct getMsg_data {
	TopicView** topicView;
	bool processed;
	bool timedOut;
	Timer* timer;
};

struct getTopic_data {
	ForumView** forumView;
};

#endif // DATA_STRUCTS_H
