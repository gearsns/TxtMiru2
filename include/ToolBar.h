// ToolBar.h
//
// �c�[���o�[
//
#ifndef TOOLBAR_H
#define TOOLBAR_H
#pragma warning(disable:4786)

#include "WindowCtrl.h"

#include <vector>
#include <string>
#include "stltchar.h"

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>

typedef std::vector<TBBUTTON> TBB_VEC;

class CGrToolBar : public CGrWinCtrl
{
public:
	// �c�[���o�[���쐬���܂��B
	//   hWnd   : HWND
	//   �߂�l : �c�[���o�[HWND
	HWND Create(HWND hWnd, HINSTANCE hInst, UINT id);
	// �{�^���̏����擾���܂��B
	//   �߂�l : �{�^���̏�
	std::tstring &GetButtons();
	// �{�^����ݒ肵�܂��B
	//   buttons : �{�^���̏�
	void SetButtons(LPCTSTR buttons);
	void Reset();

	void AddButtons(TBBUTTON *tbb, int max_num, TBADDBITMAP &tb);
	void AddButton(TBBUTTON &tbb, TBADDBITMAP &tb);
	bool GetTBBUTTON(int iItem, TBBUTTON &tb);

	LRESULT NotifyProc(LPARAM lParam);
private:
	// �{�^���̏����擾���܂��B
	//   �߂�l : �{�^���̏�
	char *getButtons();

	// �{�^���̃r�b�g�}�b�v�C���f�b�N�X��o�^���܂��B
	//   id          : �C�x���gID
	//   bitmapIndex : �J�n�ʒu
	void registBtn(int id, int bitmapIndex);
protected:
	// �I�[�o�[���C�h�\�ȃE�B���h�E(�_�C�A���O)�v���V�[�W��
	virtual LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
private:
	TBB_VEC       m_tbb;
	std::tstring  m_buttons;
};

class CGrCoolBar : public CGrWinCtrl
{
public:
	// �c�[���o�[���쐬���܂��B
	//   hWnd   : HWND
	//   �߂�l : �c�[���o�[HWND
	HWND Create(HWND hWnd, HINSTANCE hInst, UINT id, UINT toobar_id);

	CGrToolBar &GetToolBar(){ return m_toolBar; }
private:
	CGrToolBar m_toolBar;
};

#endif
