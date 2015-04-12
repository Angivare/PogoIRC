#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#include "spellchecker.h"

#include <QTextEdit>
#include <QFont>
#include <QTextCursor>
#include <QTextDocumentFragment>
#include <QTextCharFormat>
#include <QMenu>

#define MIN(A, B) ((A)<(B)?(A):(B))

struct PrevCurr {
	int anchor, position;
	bool hasSelection;
};

#ifndef NO_SPELLCHECK
class TextEdit : public QTextEdit
{
	Q_OBJECT
public:
	explicit TextEdit(QWidget *parent = 0);
	
public slots:
	void createContextMenu(QPoint);
	void correct();
	
private slots:
	void refreshCursor();
	void checkSpell();
	
private:
	void mark(QTextCursor tc, int start, int end, const QTextCharFormat& format);
	void process(int start, int end, int cur_pos);
	
	SpellChecker _sc;
	PrevCurr _pc, _ppc;
	QString _txt;
	bool _marked;
	
	QTextCharFormat _wrong;
	QTextCharFormat _right;
	
	const QRegExp _sep;
};
#else
class TextEdit : public QTextEdit
{ Q_OBJECT
	public: explicit TextEdit(QWidget* parent = 0);	
};
#endif

#endif // TEXTEDIT_H
