//
// �I�v�V�����ݒ�
//
#define STRICT // �^��錾����юg�p���ɁA��茵���Ȍ^�`�F�b�N���s���܂��B

#include <windows.h>
#include <windowsx.h>
#include "PropPage.h"

CGrPropPage::CGrPropPage()
{
	m_title[0] = _T('\0');
	bApply = false;
}

static UINT CALLBACK PropSheetPageProc(HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp)
{
	if(uMsg != PSPCB_CREATE && uMsg != PSPCB_RELEASE){
		auto *dlg = reinterpret_cast<CGrWinCtrl *>(ppsp->lParam);
		dlg->Attach(hwnd);
		SendMessage(hwnd, WM_INITDIALOG, reinterpret_cast<WPARAM>(ppsp), reinterpret_cast<LPARAM>(hwnd));
	}
	return TRUE;
}

// ���b�Z�[�W��U�蕪����E�B���h�E(�_�C�A���O)�v���V�[�W��
LRESULT CALLBACK CGrPropPage::PropWindowMapProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	// �E�B���h�E�̃v���p�e�B���X�g����CGrWinCtrl�ւ̃|�C���^�̎擾�����݂�
	auto* pTargetWnd = static_cast<CGrPropPage*>(GetProp(hWnd, _T("CPP_CLASS")));

	// CGrWinCtrl�I�u�W�F�N�g�ƃE�B���h�E�����ѕt�����Ă��Ȃ��ꍇ��
	// CGrWinCtrl�I�u�W�F�N�g�ƃE�B���h�E�����ѕt����
	if(!pTargetWnd){
		// CGrWinCtrl�ւ̃|�C���^���擾
		if(uMsg == WM_INITDIALOG){
			LPPROPSHEETPAGE ppsp = (LPPROPSHEETPAGE)lParam;
			pTargetWnd = reinterpret_cast<CGrPropPage*>(ppsp->lParam);
		}
		// �E�B���h�E�n���h����CGrWinCtrl�I�u�W�F�N�g�����ѕt����
		if(pTargetWnd){
			pTargetWnd->Attach(hWnd);
		}
	}

	// pTargetWnd �̎擾�ɐ��������ꍇ�́ACGrWinCtrl��WndProc���Ăяo��
	if(pTargetWnd){
		// CGrWinCtrl��WndProc���Ăяo��
		auto lResult = pTargetWnd->WndProc(hWnd, uMsg, wParam, lParam);
		// WM_DESTROY���b�Z�[�W�ŃE�B���h�E��CGrWinCtrl�I�u�W�F�N�g��؂藣��
		// ����WindowMapProc�ōs�����Ƃɂ��A�h���N���X����E�B���h�E��
		// �N���X�̌��т����ӎ������ɂ���
		if(uMsg == WM_DESTROY){
			pTargetWnd->Detach();
		}
		return lResult;
	}

	// �_�C�A���O�̏ꍇ�AFALSE��Ԃ�
	if(GetWindowLong(hWnd, DWL_DLGPROC)){
		return FALSE;
	}

	// �f�t�H���g�E�B���h�E�v���V�[�W�����Ăяo��
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

// �y�[�W�̐ݒ�
//   hWnd [in]: HWND
//   psp  [in]: PROPSHEETPAGE
void CGrPropPage::SetPropSheetPage(HINSTANCE hInst, HWND hWnd, PROPSHEETPAGE *psp)
{
	// �v���p�e�B�V�[�g 1�y�[�W�ڂ̐ݒ�
	psp->dwSize      = sizeof(PROPSHEETPAGE);
	psp->dwFlags     = PSP_USETITLE | PSP_HASHELP;
	psp->hInstance   = hInst;
	psp->pszTemplate = nullptr;
	psp->pszIcon     = nullptr;
	psp->pfnDlgProc  = reinterpret_cast<DLGPROC>(PropWindowMapProc);
	psp->pszTitle    = m_title;                           // �y�[�W��
	psp->lParam      = reinterpret_cast<LPARAM>(this);
	psp->pfnCallback = nullptr;
}

// ���C���E�B���h�E
LRESULT CGrPropPage::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HWND hcwnd = NULL;
	switch(uMsg){
	case WM_NOTIFY:
		{
			auto pnmh = reinterpret_cast<LPNMHDR>(lParam);
			if(pnmh && pnmh->hwndFrom != hWnd){
				hcwnd = pnmh->hwndFrom;
			}
		}
	case WM_DRAWITEM:
		{
			auto pdis = reinterpret_cast<LPDRAWITEMSTRUCT>(lParam);
			if(pdis && pdis->hwndItem != hWnd){
				hcwnd = pdis->hwndItem;
			}
		}
		break;
	}
	return FALSE;
}

LRESULT CGrPropPage::OnNotify(HWND hWnd, int idFrom, NMHDR FAR *lpnmhdr)
{
	switch(lpnmhdr->code){
	case PSN_APPLY:
		// �K�p
		if(Apply()){
			bApply = true;
			::SetWindowLong(hWnd, DWL_MSGRESULT, PSNRET_NOERROR);
		} else {
			::SetWindowLong(hWnd, DWL_MSGRESULT, PSNRET_INVALID);
		}
		break;
	case PSN_RESET:
		break;
	}
	return TRUE;
}
