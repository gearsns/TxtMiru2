#ifndef __TXTMAPPER_H__
#define __TXTMAPPER_H__

#include <windows.h>
#include <windowsx.h>
#include "stltchar.h"
#include "Text.h"
#include "TxtDocument.h"
#include "TxtMiruDef.h"
#include <vector>

class CGrTxtMapper
{
public:
	static TxtMiru::TextPoint NextTextPoint(const TxtMiru::TextPoint &tp, const TxtMiru::TextInfoList &text_list);
private:
	TxtMiru::CharPointMap m_cpMap;
	TxtMiru::CharPointMap m_cpLayoutMap;
public:
	CGrTxtMapper(){};
	virtual ~CGrTxtMapper();

	void CreateCharPointMap(const CGrTxtDocument &doc, int offset, const RECT &page_margin, const TxtMiru::TextInfoList &til_layout);
	const TxtMiru::CharPoint &GetCharPoint(const TxtMiru::TextPoint &tp);
	const TxtMiru::CharPoint &GetLayoutCharPoint(const TxtMiru::TextPoint &tp);
	static void CreateIndex(CGrTxtDocument &doc);

	const TxtMiru::CharPointMap &GetCharPointMap() const { return m_cpMap; }
	const TxtMiru::CharPointMap &GetLayoutCharPointMap() const { return m_cpLayoutMap; }
private:
	void setNombre(const CGrTxtDocument &doc, const TxtMiru::TextInfo &ti, int iIndex);
	void setRunningHeads(const CGrTxtDocument &doc, const TxtMiru::TextInfo &ti, int iIndex);
};

#endif // __TXTMAPPER_H__
