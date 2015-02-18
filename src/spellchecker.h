#ifndef SPELLCHECKER_H
#define SPELLCHECKER_H

#include "dbg.h"

#include "hunspell/hunspell.hxx"

#include <QString>
#include <QFile>
#include <QTextStream>
#include <QTextCodec>
#include <QCoreApplication>

class SpellChecker
{
public:
	SpellChecker();
	~SpellChecker();
	
	bool spell(const QString& word);
	QStringList suggest(const QString& word);
	void ignoreWord(const QString& word);
	void addToUserList(const QString& word);
	
private:
	const char* c_string(const QString& word);
	void put_word(const QString& word);
	Hunspell* m_hunspell;
	QString m_encoding;
	QTextCodec* m_codec;
};

#endif // SPELLCHECKER_H
