#ifndef __EDITBOX_H__
#define __EDITBOX_H__
#include "stltchar.h"

class CGrEditBox
{
public:
	CGrEditBox();
	virtual ~CGrEditBox();
	BOOL              Enable(BOOL fEnable)                             ;
	int               GetText(LPTSTR lpch, int cchMax)                 ;
	int               GetTextLength()                                  ;
	BOOL              SetText(LPCTSTR lpsz)                            ;
	void              LimitText(int cchMax)                            ;
	int               GetLineCount()                                   ;
	int               GetLine(int line, LPCTSTR lpch, int cchMax)      ;
	void              GetRect(LPRECT lprc)                             ;
	void              SetRect(LPRECT lprc)                             ;
	void              SetRectNoPaint(LPRECT lprc)                      ;
	DWORD             GetSel()                                         ;
	void              SetSel(int ichStart, int ichEnd)                 ;
	void              ReplaceSel(LPCTSTR lpszReplace)                  ;
	BOOL              GetModify()                                      ;
	void              SetModify(int fModified)                         ;
	BOOL              ScrollCaret()                                    ;
	int               LineFromChar(int ich)                            ;
	int               LineIndex(int line)                              ;
	int               LineLength(int line)                             ;
	void              Scroll(int dv, int dh)                           ;
	BOOL              CanUndo()                                        ;
	BOOL              Undo()                                           ;
	void              EmptyUndoBuffer()                                ;
	void              SetPasswordChar(int ch)                          ;
	void              SetTabStops(int cTabs, const int *lpTabs)        ;
	BOOL              FmtLines(int fAddEOL)                            ;
	HLOCAL            GetHandle()                                      ;
	void              SetHandle(HLOCAL h)                              ;
	int               GetFirstVisibleLine()                            ;
	BOOL              SetReadOnly(int fReadOnly)                       ;
	TCHAR             GetPasswordChar()                                ;
	void              SetWordBreakProc(EDITWORDBREAKPROC lpfnWordBreak);
	EDITWORDBREAKPROC GetWordBreakProc()                               ;

	HWND Create(HINSTANCE hInst, HWND hWnd, UINT style=WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT);
	void SetWnd(HWND hWnd){ m_hWnd = hWnd; }
	HWND GetWnd(){ return m_hWnd; }
	void GetText(std::tstring &out_str);
	operator HWND() { return m_hWnd; }

private:
	HWND m_hWnd = NULL;
};

#endif // __EDITBOX_H__
