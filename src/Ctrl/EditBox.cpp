#pragma warning( disable : 4786 )

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "EditBox.h"

CGrEditBox::CGrEditBox()
{
}

CGrEditBox::~CGrEditBox()
{
}
BOOL              CGrEditBox::Enable(BOOL fEnable)                             { return Edit_Enable(m_hWnd, fEnable)                ;}
int               CGrEditBox::GetText(LPTSTR lpch, int cchMax)                 { return Edit_GetText(m_hWnd, lpch, cchMax)          ;}
int               CGrEditBox::GetTextLength()                                  { return Edit_GetTextLength(m_hWnd)                  ;}
BOOL              CGrEditBox::SetText(LPCTSTR lpsz)                            { return Edit_SetText(m_hWnd, lpsz)                  ;}
void              CGrEditBox::LimitText(int cchMax)                            { /*  */ Edit_LimitText(m_hWnd, cchMax)              ;}
int               CGrEditBox::GetLineCount()                                   { return Edit_GetLineCount(m_hWnd)                   ;}
int               CGrEditBox::GetLine(int line, LPCTSTR lpch, int cchMax)      { return Edit_GetLine(m_hWnd, line, lpch, cchMax)    ;}
void              CGrEditBox::GetRect(LPRECT lprc)                             { /*  */ Edit_GetRect(m_hWnd, lprc)                  ;}
void              CGrEditBox::SetRect(LPRECT lprc)                             { /*  */ Edit_SetRect(m_hWnd, lprc)                  ;}
void              CGrEditBox::SetRectNoPaint(LPRECT lprc)                      { /*  */ Edit_SetRectNoPaint(m_hWnd, lprc)           ;}
DWORD             CGrEditBox::GetSel()                                         { return Edit_GetSel(m_hWnd)                         ;}
void              CGrEditBox::SetSel(int ichStart, int ichEnd)                 { /*  */ Edit_SetSel(m_hWnd, ichStart, ichEnd)       ;}
void              CGrEditBox::ReplaceSel(LPCTSTR lpszReplace)                  { /*  */ Edit_ReplaceSel(m_hWnd, lpszReplace)        ;}
BOOL              CGrEditBox::GetModify()                                      { return Edit_GetModify(m_hWnd)                      ;}
void              CGrEditBox::SetModify(int fModified)                         { /*  */ Edit_SetModify(m_hWnd, fModified)           ;}
BOOL              CGrEditBox::ScrollCaret()                                    { return Edit_ScrollCaret(m_hWnd)                    ;}
int               CGrEditBox::LineFromChar(int ich)                            { return Edit_LineFromChar(m_hWnd, ich)              ;}
int               CGrEditBox::LineIndex(int line)                              { return Edit_LineIndex(m_hWnd, line)                ;}
int               CGrEditBox::LineLength(int line)                             { return Edit_LineLength(m_hWnd, line)               ;}
void              CGrEditBox::Scroll(int dv, int dh)                           { /*  */ Edit_Scroll(m_hWnd, dv, dh)                 ;}
BOOL              CGrEditBox::CanUndo()                                        { return Edit_CanUndo(m_hWnd)                        ;}
BOOL              CGrEditBox::Undo()                                           { return Edit_Undo(m_hWnd)                           ;}
void              CGrEditBox::EmptyUndoBuffer()                                { /*  */ Edit_EmptyUndoBuffer(m_hWnd)                ;}
void              CGrEditBox::SetPasswordChar(int ch)                          { /*  */ Edit_SetPasswordChar(m_hWnd, ch)            ;}
void              CGrEditBox::SetTabStops(int cTabs, const int *lpTabs)        { /*  */ Edit_SetTabStops(m_hWnd, cTabs, lpTabs)     ;}
BOOL              CGrEditBox::FmtLines(int fAddEOL)                            { return Edit_FmtLines(m_hWnd, fAddEOL)              ;}
HLOCAL            CGrEditBox::GetHandle()                                      { return Edit_GetHandle(m_hWnd)                      ;}
void              CGrEditBox::SetHandle(HLOCAL h)                              { /*  */ Edit_SetHandle(m_hWnd, h)                   ;}
int               CGrEditBox::GetFirstVisibleLine()                            { return Edit_GetFirstVisibleLine(m_hWnd)            ;}
BOOL              CGrEditBox::SetReadOnly(int fReadOnly)                       { return Edit_SetReadOnly(m_hWnd, fReadOnly)         ;}
TCHAR             CGrEditBox::GetPasswordChar()                                { return Edit_GetPasswordChar(m_hWnd)                ;}
void              CGrEditBox::SetWordBreakProc(EDITWORDBREAKPROC lpfnWordBreak){ /*  */ Edit_SetWordBreakProc(m_hWnd, lpfnWordBreak);}
EDITWORDBREAKPROC CGrEditBox::GetWordBreakProc()                               { return Edit_GetWordBreakProc(m_hWnd)               ;}

void CGrEditBox::GetText(std::tstring &out_str)
{
	int cchMax = GetTextLength()+1;
	auto *str = new TCHAR[cchMax];
	if(str){
		GetText(str, cchMax);
		out_str = str;
		delete [] str;
	}
}

HWND CGrEditBox::Create(HINSTANCE hInst, HWND hWnd, UINT style)
{
	m_hWnd = CreateWindow(_T("edit"),
						  NULL,
						  style,
						  0, 0, 0, 0,
						  hWnd,
						  NULL,
						  hInst,
						  0);
	SetWindowFont(m_hWnd, GetWindowFont(hWnd), TRUE);

	return m_hWnd;
}
