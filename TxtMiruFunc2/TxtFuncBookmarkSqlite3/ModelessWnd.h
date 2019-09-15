#ifndef __MODELESSWND_H__
#define __MODELESSWND_H__

#include "WindowCtrl.h"
class CGrModelessWnd : public CGrWinCtrl
{
protected:
	HWND m_parentWnd = NULL;
public:
	CGrModelessWnd(HWND hWnd) : m_parentWnd(hWnd), CGrWinCtrl(){};
	virtual ~CGrModelessWnd(){};
	virtual void Show(HWND hWnd) = 0;
	virtual BOOL IsDialogMessage(HWND hwnd, MSG &msg){ return FALSE; }
	HWND GetParentWnd(){ return m_parentWnd; }
};

#endif // __MODELESSWND_H__
