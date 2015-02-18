#include "parser.h"

Parser::Parser() : m_str(QString()), m_pos(0)
{
	
}

Parser::Parser(const QString& str) :  m_str(str), m_pos(0)
{
	
}

void Parser::setPos(int& pos)
{
	m_pos = pos;
}
int Parser::getPos()
{
	return m_pos;
}

QString Parser::get(const char* first, const char* last, bool jump)
{
	int beg(m_str.indexOf(first, m_pos));
	if(jump) beg += strlen(first);
	const int end(m_str.indexOf(last, beg));
	m_pos = end;
	
	return m_str.left(end).right(end-beg).trimmed();
}

bool Parser::jumpTo(const char* to)
{
	const int tmp(m_str.indexOf(to, m_pos));
	if(tmp == -1) return false;
	m_pos = tmp + strlen(to);
	return true;
}

int Parser::find(const char* str)
{
	return m_str.indexOf(str, m_pos);
}
