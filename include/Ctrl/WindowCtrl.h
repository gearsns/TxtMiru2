// WindowsCtrl.h
//
// �T�C�Y�ƈʒu���𒲐����܂��B
//   �ǉ�������̃N���C�A���g�E�C���h�E��
//  lprect�Ɋi�[���܂��B
//
#ifndef __WINDOWCTRL_H__
#define __WINDOWCTRL_H__
#pragma warning(disable:4786)

#include "stltchar.h"
#include <windows.h>
#include <string>

class CGrWinCtrl
{
protected:
	HWND m_hWnd;
	HWND m_popupParent;
	BOOL m_bDialog;            // �_�C�A���O���ۂ�
	WNDPROC m_lpfnOldWndProc;  // �E�B���h�E�v���V�[�W���A�h���X
public:
	CGrWinCtrl();
	virtual ~CGrWinCtrl();

	// InitCommonControlsEx
	//   dwICC [in]: dwICC
	void InitCommonControlsEx(DWORD dwICC);
	// HWND���擾���܂��B
	//   �߂�l [out]: HWND
	inline HWND GetWnd() { return m_hWnd; }
	// ��ɂ��܂��B
	//   hWnd       [in]: HWND
	//   lprect [in,out]: �N���C�A���g�̃T�C�Y
	//                    ��ɂ�����̃N���C�A���g�T�C�Y��Ԃ��܂��B
	void SetWindowPosTop(LPRECT lprect) { SetWindowPosTop(m_hWnd, lprect); }
	void SetWindowPosTop(HWND hWnd, LPRECT lprect);
	// ���ɂ��܂��B
	//   hWnd       [in]: HWND
	//   lprect [in,out]: �N���C�A���g�̃T�C�Y
	//                    ���ɂ�����̃N���C�A���g�T�C�Y��Ԃ��܂��B
	void SetWindowPosBottom(LPRECT lprect) { SetWindowPosBottom(m_hWnd, lprect); }
	void SetWindowPosBottom(HWND hWnd, LPRECT lprect);
	// ���ɂ��܂��B
	//   hWnd       [in]: HWND
	//   lprect [in,out]: �N���C�A���g�̃T�C�Y
	//                    ���ɂ�����̃N���C�A���g�T�C�Y��Ԃ��܂��B
	void SetWindowPosLeft(LPRECT lprect) { SetWindowPosLeft(m_hWnd, lprect); }
	void SetWindowPosLeft(HWND hWnd, LPRECT lprect);
	// �E�ɂ��܂��B
	//   hWnd       [in]: HWND
	//   lprect [in,out]: �N���C�A���g�̃T�C�Y
	//                    �E�ɂ�����̃N���C�A���g�T�C�Y��Ԃ��܂��B
	void SetWindowPosRight(LPRECT lprect) { SetWindowPosRight(m_hWnd, lprect); }
	void SetWindowPosRight(HWND hWnd, LPRECT lprect);
	// �E�C���h�E�S�̂ɔz�u���܂��B
	//   hWnd       [in]: HWND
	//   lprect [in,out]: �N���C�A���g�̃T�C�Y
	void SetWindowPosAllClient(LPRECT lprect) { SetWindowPosAllClient(m_hWnd, lprect); }
	void SetWindowPosAllClient(HWND hWnd, LPRECT lprect);
	// �E�C���h�E��lprect�̉��ɔz�u���܂��B
	//   lprect [in]: �N���C�A���g�̃T�C�Y
	//   ���E�C���h�E���\��������Ȃ��ꍇ�́A��ɔz�u���܂��B
	//       ****** <- lprect |           | <- desktop       +--------+ | <- desktop
	//       +--------+       |   ******* |                  |        | |
	//       |        |       |   --------+              ->  +--------+ |
	//       +--------+       |        �̎�                     ******* |
	void SetWindowPosPopup(LPRECT lprect);
	void SetPopupParent(HWND hWnd) { m_popupParent = hWnd; }
	// �E�C���h�E���Z���^�[�ɕ\��
	void SetWindowPosCenter();
	// ���j�^���ŃE�C���h�E���ړ�
	void MoveWindowInMonitor(int left, int top, int width, int height);
	// �X�^�C���̕ύX
	//   dwRemove [in]: �X�^�C���̕ύX���ɍ폜�����E�B���h�E �X�^�C�����w�肵�܂��B
	//   dwAdd    [in]: �X�^�C���̕ύX���ɒǉ������E�B���h�E �X�^�C�����w�肵�܂��B
	void ModifyStyle(DWORD dwRemove, DWORD dwAdd)
	{
		SetWindowLong(m_hWnd, GWL_STYLE, GetWindowLong(m_hWnd, GWL_STYLE) & ~(dwRemove) | (dwAdd));
	}
	void ClientToScreenRect(LPRECT lprc) { ClientToScreenRect(m_hWnd, lprc); }
	void ClientToScreenRect(HWND hWnd, LPRECT lprc);
	void ScreenToClientRect(LPRECT lprc) { ScreenToClientRect(m_hWnd, lprc); }
	void ScreenToClientRect(HWND hWnd, LPRECT lprc);
	// �z�C�[���}�E�X�̃X�N���[���s���擾
	UINT GetMouseScrollLines();

	// �ĕ`��
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	virtual BOOL OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pLResult);
public:
	operator HWND() { return m_hWnd; }
	// �E�B���h�E�n���h����CWndBase�I�u�W�F�N�g�����ѕt����
	BOOL Attach(HWND hWnd);
	// �E�B���h�E�n���h����CWndBase�I�u�W�F�N�g����؂藣��
	BOOL Detach();

protected:
	// ���b�Z�[�W��U�蕪����E�B���h�E(�_�C�A���O)�v���V�[�W��
	static LRESULT CALLBACK WindowMapProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	// �I�[�o�[���C�h�\�ȃE�B���h�E(�_�C�A���O)�v���V�[�W��
	virtual LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
private:
};

#define RECT_GET_WIDTH(__lprc__)        ((__lprc__)->right - (__lprc__)->left)
#define RECT_GET_HEIGHT(__lprc__)       ((__lprc__)->bottom - (__lprc__)->top)

#endif
