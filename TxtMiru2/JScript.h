#ifndef __JSCRIPT_H__
#define __JSCRIPT_H__

#include "stlutil.h"
class CGrJScript
{
public:
	CGrJScript();
	virtual ~CGrJScript();
	bool Run(std::tstring &outstr, LPCTSTR funcName, LPCTSTR args[], int num);
	bool Load(LPCTSTR pszFileName);
protected:
	void *m_pScriptCtrl = nullptr;
};

#endif // __JSCRIPT_H__
