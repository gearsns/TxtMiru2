#ifndef __KEYBIND_H__
#define __KEYBIND_H__

#include "ComonCtrl.h"
#include "KeyState.h"

class CGrKeyBind : public CGrComonCtrl
{
public:
	CGrKeyBind();
	virtual ~CGrKeyBind();

	const CGrKeyboardState &GetKeyboardState() const { return m_ks; }
protected:
	// �I�[�o�[���C�h�\�ȃE�B���h�E(�_�C�A���O)�v���V�[�W��
	virtual LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
private:
	CGrKeyboardState m_ks;
};

#endif // __KEYBIND_H__
