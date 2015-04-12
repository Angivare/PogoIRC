#include "textedit.h"
#ifndef NO_SPELLCHECK

TextEdit::TextEdit(QWidget *parent) :
QTextEdit(parent), _marked(false),
_sep(Config().get("dicts/sep", "[(\\s+),:;\\?!/\\\\\\(\\)\\[\\]\\{\\}\\\"]").toString())
{
	_wrong.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);
	_wrong.setUnderlineColor(QColor(255,0,0));
	
	_right.setUnderlineStyle(QTextCharFormat::NoUnderline);
	
	refreshCursor();
	
	this->setTabChangesFocus(true);
	this->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, SIGNAL(customContextMenuRequested(QPoint)), 
		this, SLOT(createContextMenu(QPoint)));
	
	connect(this, SIGNAL(textChanged()), this, SLOT(checkSpell()));
	connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(refreshCursor()));
}

void TextEdit::createContextMenu(QPoint pt) {
	d.silent("Create menu");
	QMenu* menu = this->createStandardContextMenu();
	
	QTextCursor tc = this->cursorForPosition(pt);
	tc.select(QTextCursor::WordUnderCursor);
	QString w = tc.selectedText();
	QStringList suggestions = _sc.suggest(w);
	
	QAction* before = menu->actions()[0];
	for(int i = 0; i < suggestions.size(); i++) {
		QAction* act = new QAction(suggestions[i], menu);
		QFont f = act->font();
		f.setBold(true);
		act->setFont(f);
		act->setData(pt);
		menu->insertAction(before, act);
		connect(act, SIGNAL(triggered()), this, SLOT(correct()));
	}
	menu->insertSeparator(before);
		
	menu->popup(this->mapToGlobal(pt));
	connect(menu, SIGNAL(aboutToHide()), menu, SLOT(deleteLater()));
}

void TextEdit::correct() {
	QAction* act = (QAction*) this->sender();
	if(act)
	{
		QTextCursor tc = this->cursorForPosition(act->data().toPoint());
		tc.select(QTextCursor::WordUnderCursor);
		
		mark(tc, tc.selectionStart(), tc.selectionEnd(), _right);
		_marked = false;
		tc.insertText(act->text());
	}
}

void TextEdit::refreshCursor() {
	QTextCursor tc = this->textCursor();
	
	PrevCurr pc;
	pc.anchor = tc.anchor();
	pc.position = tc.position();
	pc.hasSelection = tc.hasSelection();
	
	_ppc = _pc;
	_pc = pc;
}

void TextEdit::checkSpell() {
	if(_marked || toPlainText().isEmpty()) return;
	QTextCursor tc = this->textCursor();
	
	const int select_end = _ppc.hasSelection ? tc.position() : _ppc.position;
	const int select_start = _ppc.hasSelection ? MIN(_ppc.anchor, _ppc.position) : _ppc.position;
	
	int start = toPlainText().lastIndexOf(_sep, select_start);
	if(start == -1) start = 0;
	else {
		start = toPlainText().lastIndexOf(_sep, start-1);
		if(start == -1) start = 0;
	}
	
	int end = toPlainText().indexOf(_sep, select_end);
	if(end == -1) end = toPlainText().size();
	else {
		end = toPlainText().indexOf(_sep, end+1);
		if(end == -1) end = toPlainText().size();
	}
	
	process(start, end, select_end+1);
	
	_txt = this->toPlainText();
	_marked = false;
}

void TextEdit::mark(QTextCursor tc, int start, int end, const QTextCharFormat& format) {
//	if(format == _wrong)
//		d.silent("[" + QString::number(start) + ";" + QString::number(end) + "] is wrong");
	_marked = true;
	tc.setPosition(start);
	tc.setPosition(end, QTextCursor::KeepAnchor);
	tc.setCharFormat(format);
}

void TextEdit::process(int start, int end, int cur_pos) {
//	d.silent("Processing [" + QString::number(start) + ";" + QString::number(end) + "]");
	//Set as right; we'll recheck all that later
	mark(this->textCursor(), start, end, _right);
	
	const QString sub = this->toPlainText().mid(start, end - start);
	QStringList l = sub.split(_sep);
//	d.silent("cur_pos:" + QString::number(cur_pos));
	for(int n = 0, i = start; n < l.size(); n++)
	{
	//	d.silent(l[n]);
		i = this->toPlainText().indexOf(l[n], i);
		const int wordEnd  = i + l[n].size();
		
		if(l[n].isEmpty()
		|| (i <= cur_pos && wordEnd >= cur_pos)
		) {
			i = wordEnd;
			continue;
		}
		
		if(!_sc.spell(l[n]))
		{
			mark(this->textCursor(), i, wordEnd, _wrong);
}//		} else d.silent(l[n] + " is right");
		
		i = wordEnd;
	}
}
#else
TextEdit::TextEdit(QWidget *parent) : QTextEdit(parent)
{
	this->setTabChangesFocus(true);
	this->setAcceptRichText(false);
}
#endif
