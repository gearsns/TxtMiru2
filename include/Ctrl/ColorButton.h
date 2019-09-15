#ifndef __COLOR_BUTTON_H__
#define __COLOR_BUTTON_H__

#include "ComonCtrl.h"

class CGrColorButton : public CGrComonCtrl
{
public:
	CGrColorButton();

	// �J���[�I���{�^�����쐬���܂��B
	//   hWnd    [in]: �e�E�C���h�E�̃n���h��
	//   �߂�l [out]: �쐬�����X�e�[�^�X�E�C���h�E�̃n���h��
	virtual HWND Create(HINSTANCE hInst, HWND hWnd);
	virtual void DrawItem(HWND hwnd, const DRAWITEMSTRUCT *lpDrawItemStruct);
	virtual BOOL OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pLResult);
	void OnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);

	// �F�ݒ�
	//   color [in]: COLORREF
	void SetColor(COLORREF color);
	// �F�擾
	//   �߂�l [out]: COLORREF
	COLORREF GetColor() { return m_Color; }

	BOOL Attach(HWND hWnd);
private:
	COLORREF m_Color;
	COLORREF m_DefaultColor;
protected:
	// �I�[�o�[���C�h�\�ȃE�B���h�E(�_�C�A���O)�v���V�[�W��
	virtual LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void DrawArrow(HDC hdc,
				   RECT* pRect,
				   int iDirection,
				   COLORREF clrArrow = RGB(0,0,0));
	// �F��I�����܂��B
	//   �߂�l : �I�������ꍇ�� TRUE
	BOOL chooseColor();
};

#endif
