#ifndef __HTMLTXTPARSER_H__
#define __HTMLTXTPARSER_H__

#include "TxtParser.h"

class CGrHTMLTxtParser : public CGrTxtParser
{
private:
	std::tstring m_title;
	std::tstring m_author;
	std::tstring m_info;
	std::tstring m_lastWriteTime;
public:
	CGrHTMLTxtParser();
	virtual ~CGrHTMLTxtParser();
	virtual bool ReadFile(LPCTSTR lpFileName, CGrTxtBuffer &buffer);
	virtual bool ReadBuffer(LPCTSTR lpBuffer, CGrTxtBuffer &buffer);
	virtual LPCTSTR Name() const { return _T("CGrHTMLTxtParser"); };
	virtual LPCTSTR GetTitle() const { return m_title.c_str(); };
	virtual LPCTSTR GetAuthor() const { return m_author.c_str(); };
	virtual LPCTSTR GetInfo() const { return m_info.c_str(); }
	virtual LPCTSTR GetLastWriteTime() const { return m_lastWriteTime.c_str(); }
private:
};

#endif // __HTMLTXTPARSER_H__
