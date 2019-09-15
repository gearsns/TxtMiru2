// SplitWnd.h
//
//
//
#ifndef __SPLITWND_H__
#define __SPLITWND_H__

#include "WindowCtrl.h"
#include "Bitmap.h"

class CGrSplitWnd : public CGrWinCtrl
{
public:
	CGrSplitWnd();
	virtual HWND Create(HWND hWnd);
	HWND Create(HWND hWnd, HWND leftWnd, HWND rightWnd);

	// ����������؂�ւ��܂��B
	//   bHorz [in]: true : ���� false : ����
	void SetMode(bool bHorz);
	bool GetMode(){ return m_bHorz; }
	// ��(��)�̃E�C���h�E
	HWND &SetLfWnd(HWND hwnd){ return m_leftWnd = hwnd; }
	HWND &GetLfWnd()         { return m_leftWnd; }
	// �E(��)�̃E�C���h�E
	HWND &SetRtWnd(HWND hwnd){ return m_rightWnd = hwnd; }
	HWND &GeRtWnd()          { return m_rightWnd; }

	// �E�C���h�E�T�C�Y�𒲐����܂��B
	//   lprc [in]: �͈�
	void SetWindowSize(LPRECT lprc){ setWindowSize(lprc, m_weightLeft); }
	// �X�v���b�g�o�[�̈ʒu�ݒ�
	//   pos [in]: �ʒu
	void SetPosition(int pos){ m_splitPos = pos; }
	int GetPosition(){ return m_splitPos; }

	// �X�v���b�g�o�[�̕����擾���܂��B
	//   �߂�l [out]: ��
	int GetSplitWidth();

	// �ŏ�����ݒ肵�܂��B
	//   min [in]: �ŏ���
	void SetLeftMinWidth(int min){ m_leftMin = min; }
	int GetLeftMinWidth(){ return m_leftMin; }
	//   min [in]: �ŏ���
	void SetRightMinWidth(int min){ m_rightMin = min; }
	int GetRightMinWidth(){ return m_rightMin; }

	// �E�C���h�E�̏d�݂�ς��܂��B
	//   bwl [in]: true �������d���Ȃ�܂��B(�f�t�H���g)
	void SetWeightLeft(bool bwl) { m_weightLeft = bwl; }
	bool GetWeightLeft() { return m_weightLeft; }
	//
	void SetSplitRWnd(CGrSplitWnd *wnd){ m_pRSplit = wnd; }
	void SetSplitLWnd(CGrSplitWnd *wnd){ m_pLSplit = wnd; }
	CGrSplitWnd *GetSplitRWnd(){ return m_pRSplit; }
	CGrSplitWnd *GetSplitLWnd(){ return m_pLSplit; }
protected:
	// �E�C���h�E�T�C�Y�𒲐����܂��B
	//   lprc [in]: �͈�
	void setWindowSize(LPRECT lprc, bool weightLeft);
	// �ړ��o�[���폜���܂��B
	void removeMoveBar();
	// �ړ��\�͈͂��擾���܂��B
	//   lprc [out]: �ړ��\�͈͂��󂯎��A�h���X
	void getMoveRect(LPRECT lprc);
	// �ړ�����
	//   lppos [out]: POINT
	//   lprc  [out]: �͈�
	void getMoveLimit(LPPOINT lppos, LPRECT lprc);
	// �ړ��o�[��`�悵�܂��B
	void drawMoveBar();
	// �����o�[��`��
	//   hdc    [in]: HDC
	//   lprc   [in]: �`��͈�
	//   x      [in]: �`��ʒu
	//   iWidth [in]: �o�[�̕�
	void drawVertBar(HDC hdc, LPRECT lprc, int x, int iWidth);
	// �����o�[��`��
	//   hdc    [in]:  HDC
	//   lprc   [in]:  �`��͈�
	//   x      [in]:  �`��ʒu
	//   iWidth [in]:  �o�[�̕�
	void drawHorzBar(HDC hdc, LPRECT lprc, int y, int iHeight);

private:
	// �E�B���h�E�E�N���X�̓o�^
	BOOL initApp(HINSTANCE hInst, WNDPROC fnWndProc, LPCTSTR szClassNm);
protected:
	virtual void resizeLeftWindow (RECT &rect);
	virtual void resizeRightWindow(RECT &rect);
	// �I�[�o�[���C�h�\�ȃE�B���h�E(�_�C�A���O)�v���V�[�W��
	virtual LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void OnDestroy(HWND hwnd);
	void OnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
	void OnLButtonUp(HWND hwnd, int x, int y, UINT flags);
	void OnRButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
	void OnMouseMove(HWND hwnd, int x, int y, UINT keyFlags);
	BOOL OnSetCursor(HWND hwnd, HWND hwndCursor, UINT codeHitTest, UINT msg);
protected:
	RECT    m_oldRect;
	long    m_splitPos;
	HWND    m_leftWnd;
	HWND    m_rightWnd;
	bool    m_bHorz;
	POINT   m_pos;
	bool    m_bMove;
	int     m_width;
	HCURSOR m_cursor;
	long    m_leftMin;
	long    m_rightMin;
	bool    m_weightLeft;
	CGrSplitWnd *m_pRSplit;
	CGrSplitWnd *m_pLSplit;
	CGrBitmap  m_bkbuffer;
};

#endif
