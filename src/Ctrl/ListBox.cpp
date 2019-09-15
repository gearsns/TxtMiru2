// ListBox.cpp
//
//
//
#define STRICT // 型を宣言および使用時に、より厳密な型チェックが行われます。

#include <windows.h>
#include <windowsx.h>
#include "ListBox.h"
#include <commctrl.h>

// リストボックスを作成します。
//   hWnd    [in]: 親ウインドウのハンドル
//   戻り値 [out]: 作成したステータスウインドウのハンドル
HWND CGrListBox::Create(HINSTANCE hInst, HWND hWnd)
{
	m_hWnd = CreateWindow(_T("LISTBOX"),
						  NULL,
						  WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | WS_BORDER | WS_VSCROLL |
						  LBS_DISABLENOSCROLL | LBS_NOINTEGRALHEIGHT | LBS_WANTKEYBOARDINPUT | LBS_NOTIFY,
						  0, 0, 0, 0,
						  hWnd,
						  NULL,
						  hInst,
						  0);
	SetFocus(m_hWnd);
	SetWindowFont(m_hWnd, GetWindowFont(hWnd), TRUE);

	return m_hWnd;
}

int CGrListBox::SetText(int idx, LPCTSTR lpszBuffer)
{
	InsertString(idx, lpszBuffer);
	return DeleteString(idx+1);
}
