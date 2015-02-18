#ifndef CONFIG_H
#define CONFIG_H

#include "dbg.h"

#include <QSettings>
#include <QFile>
#include <QCoreApplication>
#include <QStringList>
#include <QTextStream>

/*--------------------------------
 * CONFIG CLASS: utility class that
 * handles I/O operations with user
 * defined data
 *-------------------------------*/
class Config
{
public:
	Config();
	
	void setVars(QString);
	QString getVars();
	static void setTheme(QString);
	static QString getTheme();
	QString getFocusImage();
	QString getNotifImage();
	QString getNotifSound();
	QString getHeader();
	QString getBody();
	QString getFooter();
	QString getJvCss();
	QString getUpdateScript();
	
	QString getStyle();
	
	QStringList getUserDict();
	void addToUserDict(const QString& word);
	
	virtual QVariant get(QString key, QVariant def = QVariant());
	virtual void set(QString key, QVariant value);
	
private:
	QSettings _f;
	
	static QString s_theme;
};

#endif // CONFIG_H
