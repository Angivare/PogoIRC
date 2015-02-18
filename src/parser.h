#ifndef PARSER_H
#define PARSER_H

#include <QString>

#include "dbg.h"

class Parser
{
public:
	Parser();
	Parser(const QString&);
	
	void setPos(int& pos);
	int getPos();
	QString get(const char* first, const char* last, bool jump = true);
	bool jumpTo(const char*);
	int find(const char*);
	
private:
	const QString m_str;
	int m_pos;
};

#endif // PARSER_H
