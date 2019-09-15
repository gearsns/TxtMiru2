#ifndef __COVERINFO_H__
#define __COVERINFO_H__

#include "WindowCtrl.h"
#include "TxtFunc.h"
#include "TxtFuncBookmarkDef.h"
#include "EditBox.h"

class CGrCoverInfo : public CGrWinCtrl
{
public:
	CGrCoverInfo();
	virtual ~CGrCoverInfo();
	HWND Create(HWND hWnd);
	void Attach(HWND hWnd);
	void Set(const TxtFuncBookmark::Book &book);
protected:
	virtual LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void setWindowSize(int cx, int cy);
private:
	TxtFuncBookmark::Book m_book;
	CGrEditBox m_EditTitle ;
	CGrEditBox m_EditURL   ;
	CGrEditBox m_EditAuthor;
};

#endif // __COVERINFO_H__
