#ifndef __EDITKEYBIND_H__
#define __EDITKEYBIND_H__

#include "ComonCtrl.h"
#include "KeyState.h"

class CGrEditKeyBind : public CGrComonCtrl
{
public:
	CGrEditKeyBind();
	virtual ~CGrEditKeyBind();

	const CGrKeyboardState &GetKeyboardState() const { return m_ks; }
	void SetKeyboardState(const CGrKeyboardState &ks);
protected:
	// �I�[�o�[���C�h�\�ȃE�B���h�E(�_�C�A���O)�v���V�[�W��
	virtual LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
private:
	CGrKeyboardState m_ks;
};

#endif // __EDITKEYBIND_H__
