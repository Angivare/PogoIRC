#include "spellchecker.h"
#ifndef NO_SPELLCHECK

SpellChecker::SpellChecker()
{
	Config c;
	
	const QByteArray fPath(c.get("dict/base",
		QCoreApplication::applicationDirPath() + "/data/dicts/fr_FR").toByteArray());
	m_hunspell = new Hunspell(
		(fPath + ".aff").constData(),
		(fPath + ".dic").constData()
	);
	
	m_codec = QTextCodec::codecForName(m_hunspell->get_dic_encoding());
	
	QStringList userDict = c.getUserDict();
	for(int i = 0; i < userDict.size(); i++)
		put_word(userDict.at(i));
	
	put_word(":");
	put_word("!");
	put_word("?");
	put_word(",");
	put_word(";");
}

SpellChecker::~SpellChecker() {
	delete m_hunspell;
}

bool SpellChecker::spell(const QString& word) {
	return 0 != m_hunspell->spell(c_string(word));
}

QStringList SpellChecker::suggest(const QString& word) {
	char** wordList;
	
	QStringList ret;
	int n = m_hunspell->suggest(&wordList, c_string(word));
	for(int i = 0; i < n; i++)
		ret.append(m_codec->toUnicode(wordList[i]));
	m_hunspell->free_list(&wordList, n);
	
	return ret;
}

void SpellChecker::ignoreWord(const QString& word) {
	put_word(word);
}

void SpellChecker::addToUserList(const QString& word)  {
	put_word(word);
	Config().addToUserDict(word);
}

const char* SpellChecker::c_string(const QString& word) {
	return m_codec->fromUnicode(word).constData();
}

void SpellChecker::put_word(const QString& word) {
	m_hunspell->add(c_string(word));
}

#endif
