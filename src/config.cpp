#include "config.h"

QString Config::s_theme;

Config::Config() : _f(QCoreApplication::applicationDirPath ()+"/config.ini", QSettings::IniFormat)
{}

void Config::set(QString key, QVariant val) {
	_f.setValue(key, val);
}
QVariant Config::get(QString key, QVariant def) {
	return _f.value(key, def);
}

void Config::setVars(QString vars) {
	QFile f(QCoreApplication::applicationDirPath ()+"/data/vars.js");
	f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text);
	f.write(vars.toUtf8().data());
}
QString Config::getVars() {
	QFile f(QCoreApplication::applicationDirPath ()+"/data/vars.js");
	f.open(QIODevice::ReadOnly | QIODevice::Text);
	QString vars;
	while(f.bytesAvailable())
		vars += QString::fromUtf8(f.readLine());
	return vars;
}

void Config::setTheme(QString theme) { s_theme = theme; }
QString Config::getTheme() { return s_theme; }

QString Config::getFocusImage() {
	return QCoreApplication::applicationDirPath ()+"/data/themes/" + s_theme + "/img/focusIn.png";
}
QString Config::getNotifImage() {
	return QCoreApplication::applicationDirPath ()+"/data/themes/" + s_theme + "/img/notif.png";
}

QString Config::getNotifSound() {
	return QCoreApplication::applicationDirPath ()+"/data/themes/" + s_theme + "/notif.wav";
}

QString Config::getStyle() {
	QFile f(QCoreApplication::applicationDirPath ()+"/data/themes/" + s_theme + "/style.css");
	f.open(QIODevice::ReadOnly | QIODevice::Text);
	return QString::fromUtf8(f.readAll());
}

QString Config::getHeader() {
	QFile f(QCoreApplication::applicationDirPath ()+"/data/themes/" + s_theme + "/header.html");
	f.open(QIODevice::ReadOnly | QIODevice::Text);
	return QString::fromUtf8(f.readAll());
}
QString Config::getBody() {
	QFile f(QCoreApplication::applicationDirPath ()+"/data/themes/" + s_theme + "/body.html");
	f.open(QIODevice::ReadOnly | QIODevice::Text);
	return QString::fromUtf8(f.readAll());
}
QString Config::getFooter() {
	QFile f(QCoreApplication::applicationDirPath ()+"/data/themes/" + s_theme + "/footer.html");
	f.open(QIODevice::ReadOnly | QIODevice::Text);
	return QString::fromUtf8(f.readAll());
}

QString Config::getJvCss() {
	QFile f(QCoreApplication::applicationDirPath ()+"/data/jvc.css");
	f.open(QIODevice::ReadOnly | QIODevice::Text);
	return QString::fromUtf8(f.readAll());
}

QString Config::getUpdateScript() {
	return QCoreApplication::applicationDirPath() + "/pogo_irc_1up.exe";
}

QStringList Config::getUserDict() {
	QStringList ret;
	QFile f(get("dict/perso",
		QCoreApplication::applicationDirPath() + "/data/dicts/perso").toString());
	if(f.open(QIODevice::ReadOnly)) {
		QTextStream s(&f);
		for(QString w = s.readLine(); !w.isEmpty(); w = s.readLine())
			ret.append(w);
		f.close();
	} else {
		d.silent("Pas de dictionnaire utilisateur");
	}
	return ret;
}

void Config::addToUserDict(const QString& word) {
	QFile f(get("dict/perso",
		QCoreApplication::applicationDirPath() + "/data/dicts/perso").toString());
	if(f.open(QIODevice::WriteOnly | QIODevice::Append)) {
		f.write(word.toLocal8Bit());
		f.write("\n");
	} else {
		d << "Impossible d'ouvrir le dictionnaire utilisateur";
	}
}
