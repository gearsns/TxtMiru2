#ifndef __LIST_BOX_H__
#define __LIST_BOX_H__

#include "ComonCtrl.h"
#include <windowsx.h>

class CGrListBox : public CGrComonCtrl
{
public:
	// ���X�g�{�b�N�X���쐬���܂��B
	//   hWnd    [in]: �e�E�C���h�E�̃n���h��
	//   �߂�l [out]: �쐬�����X�e�[�^�X�E�C���h�E�̃n���h��
	virtual HWND Create(HINSTANCE hInst, HWND hWnd);
	int AddString(LPCTSTR str)                    { return ListBox_AddString(m_hWnd, str);                }
	int GetCurSel()                               { return ListBox_GetCurSel(m_hWnd);                     }
	int GetTextLen(int idx)                       { return ListBox_GetTextLen(m_hWnd, idx);               }
	int GetText(int idx, LPCTSTR lpszBuffer)      { return ListBox_GetText(m_hWnd, idx, lpszBuffer);      }
	int GetCaretIndex()                           { return ListBox_GetCaretIndex(m_hWnd);                 }
	int GetCount()                                { return ListBox_GetCount(m_hWnd);                      }
	int DeleteString(int idx)                     { return ListBox_DeleteString(m_hWnd, idx);             }
	int InsertString(int idx, LPCTSTR lpszBuffer) { return ListBox_InsertString(m_hWnd, idx, lpszBuffer); }
	int SetCaretIndex(int idx)                    { return ListBox_SetCaretIndex(m_hWnd, idx);            }
	int SetCurSel(int idx)                        { return ListBox_SetCurSel(m_hWnd, idx);                }
	int SetText(int idx, LPCTSTR lpszBuffer);
	LRESULT GetItemData(int index)                { return ListBox_GetItemData(m_hWnd, index);            }
	int SetItemData(int index, LPARAM data)       { return ListBox_SetItemData(m_hWnd, index, data);      }
	BOOL ResetContent()                           { return ListBox_ResetContent(m_hWnd);                  }
};

#endif
