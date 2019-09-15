//
// �I�v�V�����ݒ�
//
#ifndef __PROP_PAGE_H__
#define __PROP_PAGE_H__

#include "WindowCtrl.h"

class CGrPropPage : public CGrWinCtrl
{
public:
	CGrPropPage();

	// �y�[�W�̐ݒ�
	//   hWnd [in]: HWND
	//   psp  [in]: PROPSHEETPAGE
	virtual void SetPropSheetPage(HINSTANCE hInst, HWND hWnd, PROPSHEETPAGE *psp);

	virtual bool Apply() { return true; }
	bool IsApply() { return bApply; }
protected:
	// ���b�Z�[�W��U�蕪����E�B���h�E(�_�C�A���O)�v���V�[�W��
	static LRESULT CALLBACK PropWindowMapProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	// �I�[�o�[���C�h�\�ȃE�B���h�E(�_�C�A���O)�v���V�[�W��
	virtual LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT OnNotify(HWND hWnd, int idFrom, NMHDR FAR *lpnmhdr);
protected:
	TCHAR m_title[512];
	bool bApply;
};

#endif
