#include <windows.h>
#include <winuser.h>
#include <commctrl.h>
#include <stdlib.h>
#include "WindowCtrl.h"
#include "Win32Wrap.h"

#ifndef SPI_GETWHEELSCROLLLINES
#define SPI_GETWHEELSCROLLLINES   104
#endif

// �z�C�[���}�E�X�Ή�
#include "zmouse.h"

CGrWinCtrl::CGrWinCtrl() : m_hWnd(NULL), m_popupParent(GetDesktopWindow()), m_bDialog(FALSE), m_lpfnOldWndProc(nullptr)
{
}

CGrWinCtrl::~CGrWinCtrl()
{
	Detach();
}

// ��ɂ��܂��B
//   hWnd       [in]: HWND
//   lprect [in,out]: �N���C�A���g�̃T�C�Y
//                    ��ɂ�����̃N���C�A���g�T�C�Y��Ԃ��܂��B
void CGrWinCtrl::SetWindowPosTop(HWND hWnd, LPRECT lprect)
{
	RECT rect;
	GetWindowRect(hWnd, &rect);
	int nWidth  = RECT_GET_WIDTH(lprect);
	int nHeight = RECT_GET_HEIGHT(&rect);
	// ��ɍŏ㕔�ɕ\�������悤�ɂ��܂��B
	MoveWindow(hWnd, lprect->left, lprect->top, nWidth, nHeight, TRUE);
	// �������N���C�A���g�̈�����炵�܂��B
	lprect->top += nHeight;
}

// ���ɂ��܂��B
//   hWnd       [in]: HWND
//   lprect [in,out]: �N���C�A���g�̃T�C�Y
//                    ���ɂ�����̃N���C�A���g�T�C�Y��Ԃ��܂��B
void CGrWinCtrl::SetWindowPosBottom(HWND hWnd, LPRECT lprect)
{
	RECT rect;
	GetWindowRect(hWnd, &rect);
	int nWidth  = RECT_GET_WIDTH(lprect);
	int nHeight = RECT_GET_HEIGHT(&rect);
	// �X�e�[�^�X�o�[�̍������N���C�A���g�̈�����炵�܂��B
	lprect->bottom -= nHeight;
	// �T�C�Y�ƈʒu�����i��Ɉ�ԉ��ɂ���悤�ɐݒ肵�܂��B�j
	MoveWindow(hWnd, lprect->left, lprect->bottom, nWidth, nHeight, TRUE);
}

// ���ɂ��܂��B
//   hWnd       [in]: HWND
//   lprect [in,out]: �N���C�A���g�̃T�C�Y
//                    ���ɂ�����̃N���C�A���g�T�C�Y��Ԃ��܂��B
void CGrWinCtrl::SetWindowPosLeft(HWND hWnd, LPRECT lprect)
{
	RECT rect;
	GetWindowRect(hWnd, &rect);
	int nWidth  = RECT_GET_WIDTH(&rect);
	int nHeight = RECT_GET_HEIGHT(lprect);
	// ��ɍ��ɕ\�������悤�ɂ��܂��B
	MoveWindow(hWnd, lprect->left, lprect->top, nWidth, nHeight, TRUE);
	// �����N���C�A���g�̈�����炵�܂��B
	lprect->left += nWidth;
}

// �E�ɂ��܂��B
//   hWnd       [in]: HWND
//   lprect [in,out]: �N���C�A���g�̃T�C�Y
//                    �E�ɂ�����̃N���C�A���g�T�C�Y��Ԃ��܂��B
void CGrWinCtrl::SetWindowPosRight(HWND hWnd, LPRECT lprect)
{
	RECT rect;
	GetWindowRect(hWnd, &rect);
	int nWidth  = RECT_GET_WIDTH(&rect);
	int nHeight = RECT_GET_HEIGHT(lprect);
	// �����N���C�A���g�̈�����炵�܂��B
	lprect->left -= nWidth;
	// ��ɉE�ɕ\�������悤�ɂ��܂��B
	MoveWindow(hWnd, lprect->left, lprect->top, nWidth, nHeight, TRUE);
}

// �E�C���h�E�S�̂ɔz�u���܂��B
//   hWnd       [in]: HWND
//   lprect [in,out]: �N���C�A���g�̃T�C�Y
void CGrWinCtrl::SetWindowPosAllClient(HWND hWnd, LPRECT lprect)
{
	int nWidth  = RECT_GET_WIDTH(lprect);
	int nHeight = RECT_GET_HEIGHT(lprect);
	MoveWindow(hWnd, lprect->left, lprect->top, nWidth, nHeight, TRUE);
}

// �E�C���h�E��lprect�̉��ɔz�u���܂��B
//   lprect [in]: �N���C�A���g�̃T�C�Y
//   ���E�C���h�E���\��������Ȃ��ꍇ�́A��ɔz�u���܂��B
//       ****** <- lprect |           | <- desktop       +--------+ | <- desktop
//       +--------+       |   ******* |                  |        | |
//       |        |       |   --------+              ->  +--------+ |
//       +--------+       |        �̎�                     ******* |
void CGrWinCtrl::SetWindowPosPopup(LPRECT lprect)
{
	RECT  rc, topRc;
	POINT pos;

	ClientToScreenRect(m_hWnd, lprect);
	pos.x = lprect->left;
	pos.y = lprect->bottom;
	GetWindowRect(m_hWnd, &rc);
	// �w��ʒu�ɃE�C���h�E���ړ����܂��B
	int width  = RECT_GET_WIDTH(&rc);
	int height = RECT_GET_HEIGHT(&rc);
	SetRect(&rc, pos.x, pos.y, pos.x+width, pos.y+height);
	// �E�C���h�E����ʂ���͂ݏo�Ȃ��悤�ɂ��܂��B
	GetWindowRect(m_popupParent, &topRc);
	long style = GetWindowLong(m_popupParent, GWL_STYLE);
	if(style & ~WS_HSCROLL){
		topRc.right -= GetSystemMetrics(SM_CXHSCROLL);
	}
	if(style & ~WS_VSCROLL){
		topRc.bottom -= GetSystemMetrics(SM_CYVSCROLL);
	}
	//   �E�ɂ͂ݏo��
	if(rc.right > topRc.right){
		pos.x = topRc.right - width;
		if(pos.x < 0){
			pos.x = 0;
		}
	}
	//   ���ɂ͂ݏo��
	if(rc.bottom > topRc.bottom){
		pos.y = lprect->top - height;
		if(pos.y < 0){
			pos.y = 0;
		}
	}
	ScreenToClient(m_hWnd, &pos);
	MoveWindow(m_hWnd, pos.x, pos.y, width, height, TRUE);
}

// InitCommonControlsEx
//   dwICC [in]: dwICC
void CGrWinCtrl::InitCommonControlsEx(DWORD dwICC)
{
	// CommonControl�̏��������s��
	InitCommonControls();
	INITCOMMONCONTROLSEX iccex = { sizeof(INITCOMMONCONTROLSEX), dwICC };
	::InitCommonControlsEx(&iccex);
}

void CGrWinCtrl::ClientToScreenRect(HWND hWnd, LPRECT lprc)
{
	::ClientToScreen(hWnd, reinterpret_cast<LPPOINT>(lprc)  );
	::ClientToScreen(hWnd, reinterpret_cast<LPPOINT>(lprc)+1);
}

void CGrWinCtrl::ScreenToClientRect(HWND hWnd, LPRECT lprc)
{
	::ScreenToClient(hWnd, reinterpret_cast<LPPOINT>(lprc)  );
	::ScreenToClient(hWnd, reinterpret_cast<LPPOINT>(lprc)+1);
}

// pgm form Visual C TechTips
// �E�B���h�E�n���h����CGrWinCtrl�I�u�W�F�N�g�����ѕt����
BOOL CGrWinCtrl::Attach(HWND hWnd)
{
	// �E�B���h�E�n���h�� hWnd �� m_hWnd �ɕۑ�
	if(!hWnd){
		return FALSE;
	}
	m_hWnd = hWnd;
	// �_�C�A���O���E�B���h�E���𔻒肷��
	m_bDialog = (GetWindowLong(hWnd, DWL_DLGPROC) != 0);
	int nIndex = m_bDialog ? DWL_DLGPROC : GWL_WNDPROC;

	// �E�B���h�E�n���h����CGrWinCtrl�I�u�W�F�N�g�����т���
	SetProp(m_hWnd, _T("CPP_CLASS"), this);

	// �����̃E�B���h�E���T�u�N���X������ꍇ�́A�E�B���h�E(�_�C�A���O)
	// �v���V�[�W����WindowMapProc�ɒu��������
	if(GetWindowLong(m_hWnd, nIndex) != reinterpret_cast<LONG>(WindowMapProc)){
		m_lpfnOldWndProc = reinterpret_cast<WNDPROC>(SetWindowLong(m_hWnd, nIndex, reinterpret_cast<LONG>(WindowMapProc)));
	}

	return TRUE;
}

// �E�B���h�E�n���h����CWndBase�I�u�W�F�N�g����؂藣��
BOOL CGrWinCtrl::Detach()
{
	if(!m_hWnd){
		return FALSE;
	}

	// �E�B���h�E���T�u�N���X������Ă���ꍇ�́A�E�B���h�E(�_�C�A���O)
	// �v���V�[�W�������ɖ߂��B
	if(m_lpfnOldWndProc){
		SetWindowLong(m_hWnd, (m_bDialog ? DWL_DLGPROC : GWL_WNDPROC), reinterpret_cast<DWORD>(m_lpfnOldWndProc));
	}
	// �E�B���h�E�n���h����CWndBase�I�u�W�F�N�g����؂藣��
	RemoveProp(m_hWnd, _T("CPP_CLASS"));

	m_hWnd = NULL;

	return TRUE;
}

bool isSendChildNotify(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(hWnd){
		auto *pChildWnd = static_cast<CGrWinCtrl*>(GetProp(hWnd, _T("CPP_CLASS")));
		return (pChildWnd && pChildWnd->OnChildNotify(uMsg, wParam, lParam, nullptr)) == TRUE;
	}
	return false;
}

// ���b�Z�[�W��U�蕪����E�B���h�E(�_�C�A���O)�v���V�[�W��
LRESULT CALLBACK CGrWinCtrl::WindowMapProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// �E�B���h�E�̃v���p�e�B���X�g����CGrWinCtrl�ւ̃|�C���^�̎擾�����݂�
	auto* pTargetWnd = static_cast<CGrWinCtrl*>(GetProp(hWnd, _T("CPP_CLASS")));

	// CGrWinCtrl�I�u�W�F�N�g�ƃE�B���h�E�����ѕt�����Ă��Ȃ��ꍇ��
	// CGrWinCtrl�I�u�W�F�N�g�ƃE�B���h�E�����ѕt����
	if(!pTargetWnd){
		// CGrWinCtrl�ւ̃|�C���^���擾
		if((uMsg == WM_CREATE) || (uMsg == WM_NCCREATE)){
			pTargetWnd = static_cast<CGrWinCtrl*>(reinterpret_cast<LPCREATESTRUCT>(lParam)->lpCreateParams);
		} else if(uMsg == WM_INITDIALOG){
			pTargetWnd = reinterpret_cast<CGrWinCtrl*>(lParam);
		}
		// �E�B���h�E�n���h����CGrWinCtrl�I�u�W�F�N�g�����ѕt����
		if(pTargetWnd){
			pTargetWnd->Attach(hWnd);
		}
	}

	// pTargetWnd �̎擾�ɐ��������ꍇ�́ACGrWinCtrl��WndProc���Ăяo��
	if(pTargetWnd){
		if(uMsg == WM_DRAWITEM){
			auto dw = reinterpret_cast<LPDRAWITEMSTRUCT>(lParam);
			if(dw && dw->hwndItem != hWnd && isSendChildNotify(dw->hwndItem, uMsg, wParam, lParam)){
				SendMessage(dw->hwndItem, uMsg, wParam, lParam);
				return TRUE;
			}
		} else if(uMsg == WM_CTLCOLORSTATIC ||
				  uMsg == WM_CTLCOLOREDIT   ){
			auto cdlg = reinterpret_cast<HWND>(lParam);
			if(cdlg != hWnd && isSendChildNotify(cdlg, uMsg, wParam, lParam)){
				return SendMessage(cdlg, uMsg, wParam, lParam);
			}
		}
		// CGrWinCtrl��WndProc���Ăяo��
		auto lResult = pTargetWnd->WndProc(hWnd, uMsg, wParam, lParam);
		// WM_DESTROY���b�Z�[�W�ŃE�B���h�E��CGrWinCtrl�I�u�W�F�N�g��؂藣��
		// ����WindowMapProc�ōs�����Ƃɂ��A�h���N���X����E�B���h�E��
		// �N���X�̌��т����ӎ������ɂ���
		if(uMsg == WM_DESTROY){
			pTargetWnd->Detach();
		} else if(uMsg == WM_NOTIFY){
			auto pnmh = reinterpret_cast<LPNMHDR>(lParam);
			if(pnmh && pnmh->hwndFrom != hWnd && isSendChildNotify(pnmh->hwndFrom, uMsg, wParam, lParam)){
				SendNotifyMessage(pnmh->hwndFrom, uMsg, wParam, lParam);
			}
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

// �I�[�o�[���C�h�\�ȃE�B���h�E(�_�C�A���O)�v���V�[�W��
LRESULT CGrWinCtrl::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// �T�u�N���X�����Ă���ꍇ�́A�Â��E�B���h�E�v���V�[�W���Ɍ�̏�����C����
	if(m_lpfnOldWndProc){
		return CallWindowProc(m_lpfnOldWndProc, hWnd, uMsg, wParam, lParam);
	}
	// �_�C�A���O�̏ꍇ�AFALSE��Ԃ�
	if(m_bDialog){
		return FALSE;
	}
	// �T�u�N���X���Ŗ����ꍇ�́A�f�t�H���g�E�B���h�E�v���V�[�W�����Ăяo��
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
// �z�C�[���}�E�X�̃X�N���[���s���擾
UINT CGrWinCtrl::GetMouseScrollLines()
{
	static UINT uCachedScrollLines = 0;
	if(uCachedScrollLines != 0){
		return uCachedScrollLines;
	}
	::SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &uCachedScrollLines, 0);
	return uCachedScrollLines;
}

BOOL CGrWinCtrl::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	return FALSE;
}

BOOL CGrWinCtrl::OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pLResult)
{
	return FALSE;
}

void CGrWinCtrl::SetWindowPosCenter()
{
	RECT rc_parent ={};
	RECT rc_dialog ={};
	::GetWindowRect(GetParent(m_hWnd), &rc_parent);
	::GetWindowRect(m_hWnd           , &rc_dialog);
	int x = rc_parent.left + (rc_parent.right  - rc_parent.left) / 2 - (rc_dialog.right  - rc_dialog.left) / 2;
	int y = rc_parent.top  + (rc_parent.bottom - rc_parent.top ) / 2 - (rc_dialog.bottom - rc_dialog.top ) / 2;
	::SetWindowPos(m_hWnd, 0, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

void CGrWinCtrl::MoveWindowInMonitor(int left, int top, int width, int height)
{
	if(left >= 0 && top >= 0 && width >= 0 && height >= 0){
		// ���j�^���Ɏ��܂��Ă��邩�`�F�b�N
		POINT point = {left, top};
		auto hMonitor = ::MonitorFromPoint(point, MONITOR_DEFAULTTONEAREST);
		if(hMonitor){
			MONITORINFO mi = {sizeof(MONITORINFO)};
			if(::GetMonitorInfo(hMonitor, &mi) && PtInRect(&mi.rcMonitor, point)){
				::MoveWindow(m_hWnd, left, top, width, height, TRUE);
			}
		}
	}
}
