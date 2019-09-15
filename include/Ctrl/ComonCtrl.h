//
#ifndef __COMONCTRL_H__
#define __COMONCTRL_H__

#include "WindowCtrl.h"

class CGrComonCtrl : public CGrWinCtrl
{
public:
	CGrComonCtrl();
	virtual ~CGrComonCtrl();

	SHORT GetExitKeyCode(){ return m_exitKeyCode; }
	BOOL  GetExitShiftKey(){ return m_bShift; }
	void SetExitKeyCode(SHORT code){ m_exitKeyCode = code; }
protected:
	// �I�[�o�[���C�h�\�ȃE�B���h�E(�_�C�A���O)�v���V�[�W��
	virtual LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
protected:
	SHORT   m_exitKeyCode;
	BOOL    m_bShift;
	BOOL    m_bPopup;
};

#endif
