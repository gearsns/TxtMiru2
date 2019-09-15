// ListBox.cpp
//
//
//
#define STRICT // �^��錾����юg�p���ɁA��茵���Ȍ^�`�F�b�N���s���܂��B

#include <windows.h>
#include <windowsx.h>
#include "ListBox.h"
#include <commctrl.h>

// ���X�g�{�b�N�X���쐬���܂��B
//   hWnd    [in]: �e�E�C���h�E�̃n���h��
//   �߂�l [out]: �쐬�����X�e�[�^�X�E�C���h�E�̃n���h��
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
