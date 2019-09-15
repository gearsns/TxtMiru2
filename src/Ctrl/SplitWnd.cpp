#pragma warning( disable : 4786 )

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "SplitWnd.h"

#define SPLITWIDTH  5
#define FRAME_WIDTH 2

CGrSplitWnd::CGrSplitWnd()
: m_bHorz(false), m_weightLeft(true), m_bMove(false), m_leftMin(0), m_rightMin(0), m_splitPos(0), m_cursor(NULL), m_width(FRAME_WIDTH), m_pRSplit(NULL), m_pLSplit(NULL)
{
	m_pos.x  = INT_MIN;
	m_pos.y  = INT_MIN;
	memset(&m_oldRect, 0x00, sizeof(m_oldRect));
}

#define CGRTYPEID _T("CGrSplitWnd")
HWND CGrSplitWnd::Create(HWND hWnd)
{
	auto hInst = GetWindowInstance(hWnd);
	initApp(hInst, CGrWinCtrl::WindowMapProc, CGRTYPEID);

	auto spltWnd = CreateWindowEx(WS_EX_TOPMOST, CGRTYPEID, NULL,
								  WS_VISIBLE | WS_CHILD, 0, 0, SPLITWIDTH, SPLITWIDTH,
								  hWnd, NULL, hInst, this);
	SetMode(m_bHorz);
	return spltWnd;
}

HWND CGrSplitWnd::Create(HWND hWnd, HWND leftWnd, HWND rightWnd)
{
	auto spltWnd = Create(hWnd);
	SetLfWnd(leftWnd);
	SetRtWnd(rightWnd);
	return spltWnd;
}

// �E�B���h�E�E�N���X�̓o�^
BOOL CGrSplitWnd::initApp(HINSTANCE hInst, WNDPROC fnWndProc, LPCTSTR szClassNm)
{
	WNDCLASSEX wc = { sizeof(WNDCLASSEX) };

	wc.hCursor       = LoadCursor(NULL, IDC_SIZEWE);
	wc.lpszClassName = szClassNm;
	wc.hInstance     = hInst;
	wc.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wc.lpfnWndProc   = fnWndProc;

	return RegisterClassEx(&wc);
}

// ���C���E�B���h�E
LRESULT CGrSplitWnd::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg){
		HANDLE_MSG(hWnd, WM_DESTROY    , OnDestroy    );
		HANDLE_MSG(hWnd, WM_LBUTTONUP  , OnLButtonUp  );
		HANDLE_MSG(hWnd, WM_LBUTTONDOWN, OnLButtonDown);
		HANDLE_MSG(hWnd, WM_RBUTTONDOWN, OnRButtonDown);
		HANDLE_MSG(hWnd, WM_MOUSEMOVE  , OnMouseMove  );
		HANDLE_MSG(hWnd, WM_SETCURSOR  , OnSetCursor  );
	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	return 0L;
}

// �X�v���b�g�o�[�̕����擾���܂��B
//   �߂�l [out]: ��
int CGrSplitWnd::GetSplitWidth()
{
	RECT rc;
	::GetWindowRect(m_hWnd, &rc);
	return (m_bHorz) ? (rc.bottom - rc.top) : (rc.right - rc.left);
}

// ����������؂�ւ��܂��B
//   bHorz [in]: true : ���� false : ����
void CGrSplitWnd::SetMode(bool bHorz)
{
	m_bHorz = bHorz;
	m_cursor = static_cast<HCURSOR>(LoadCursor(NULL, (bHorz) ? IDC_SIZENS : IDC_SIZEWE));
}

// �ړ��o�[���폜���܂��B
void CGrSplitWnd::removeMoveBar()
{
	ReleaseCapture(); // �}�E�X���
	//
	m_pos.x = INT_MIN;
	m_pos.y = INT_MIN;
	// �ړ��o�[�������܂��B
	if(!m_bMove){ return ; }

	drawMoveBar();
	m_bMove = false;
	if(m_leftWnd ){ InvalidateRect(m_leftWnd , NULL, TRUE); }
	if(m_rightWnd){ InvalidateRect(m_rightWnd, NULL, TRUE); }
	InvalidateRect(m_hWnd, NULL, TRUE);
}

// �ړ��\�͈͂��擾���܂��B
//   lprc [out]: �ړ��\�͈͂��󂯎��A�h���X
void CGrSplitWnd::getMoveRect(LPRECT lprc)
{
	RECT rc;
	auto hParent = GetParent(m_hWnd);

	if(m_leftWnd){
		GetWindowRect(m_leftWnd, &rc);
	} else {
		GetClientRect(hParent, &rc);
		ClientToScreenRect(hParent, &rc);
	}
	lprc->left = rc.left;
	lprc->top  = rc.top;

	if(m_rightWnd){
		GetWindowRect(m_rightWnd, &rc);
	} else {
		GetClientRect(hParent, &rc);
		ClientToScreenRect(hParent, &rc);
	}
	lprc->right  = rc.right;
	lprc->bottom = rc.bottom;

	GetWindowRect(m_hWnd, &rc);
	if(m_bHorz){
		lprc->left   = rc.left;
		lprc->right  = rc.right;
	} else {
		lprc->top    = rc.top;
		lprc->bottom = rc.bottom;
	}
}

static void limitsPos(LONG &pos, LONG minpos, LONG maxpos)
{
	if(pos <= minpos){
		pos = minpos;
	} else if(pos >= maxpos){
		pos = maxpos;
	}
}

// �ړ�����
//   lppos [out]: POINT
//   lprc  [out]: �͈�
void CGrSplitWnd::getMoveLimit(LPPOINT lppos, LPRECT lprc)
{
	getMoveRect(lprc); // �ړ��\�͈͂��擾���܂��B
	// ���̒l��ς��Ȃ��悤�Ɉ�U�R�s�[���܂��B
	auto rc = *lprc;
	rc.right  -= m_width;
	rc.bottom -= m_width;
	if(m_bHorz){
		rc.top += m_leftMin;
		rc.bottom += m_rightMin;
	} else {
		rc.left += m_leftMin;
		rc.right += m_rightMin;
	}
	GetCursorPos(lppos); // ���݂̃}�E�X�ʒu���X�N���[�����W�Ŏ擾
	limitsPos(lppos->x, rc.left, rc.right);
	limitsPos(lppos->y, rc.top, rc.bottom);
}

// �ړ��o�[��`�悵�܂��B
void CGrSplitWnd::drawMoveBar()
{
	RECT  rc;
	RECT  moveRc;
	POINT pos;
	auto  hParent = GetParent(m_hWnd);

	if(!m_bMove){ return; } // �ړ����ł͂Ȃ��̂ŕ`�悵�܂���B

	getMoveLimit(&pos, &moveRc); // �ړ�����

	GetWindowRect(hParent, &rc);

	auto hdc = GetWindowDC(hParent);

	if(m_bHorz){ // ����
		moveRc.left -= rc.left;
		moveRc.right -= rc.left;
		// �ړ������B
		if(m_pos.y != pos.y){
			// �O��̕\���������܂��B
			if(m_pos.y > INT_MIN){ drawHorzBar(hdc, &moveRc, m_pos.y-rc.top, m_width); }
			// �o�[�̕\��
			drawHorzBar(hdc, &moveRc, pos.y-rc.top,  m_width);
		}
	} else {    // ����
		moveRc.top -= rc.top;
		moveRc.bottom -= rc.top;
		// �ړ������B
		if(m_pos.x != pos.x){
			// �O��̕\���������܂��B
			if(m_pos.x > INT_MIN){ drawVertBar(hdc, &moveRc, m_pos.x-rc.left, m_width); }
			// �o�[�̕\��
			drawVertBar(hdc, &moveRc, pos.x-rc.left, m_width);
		}
	}
	ReleaseDC(hParent, hdc);

	m_pos = pos;
}

// �����o�[��`��
//   hdc    [in]: HDC
//   lprc   [in]: �`��͈�
//   x      [in]: �`��ʒu
//   iWidth [in]: �o�[�̕�
void CGrSplitWnd::drawVertBar(HDC hdc, LPRECT lprc, int x, int iWidth)
{
	// �`��͈͊O
	if(x == INT_MIN){ return; }
	// ���̒l��ς��Ȃ��悤�Ɉ�U�R�s�[���܂��B
	auto rc = *lprc;
	rc.left = x - 1;
	rc.right = x + iWidth + 2;
	InvertRect(hdc, &rc);
}

// �����o�[��`��
//   hdc    [in]:  HDC
//   lprc   [in]:  �`��͈�
//   x      [in]:  �`��ʒu
//   iWidth [in]:  �o�[�̕�
void CGrSplitWnd::drawHorzBar(HDC hdc, LPRECT lprc, int y, int iHeight)
{
	// �`��͈͊O
	if(y == INT_MIN){ return; }
	// ���̒l��ς��Ȃ��悤�Ɉ�U�R�s�[���܂��B
	auto rc = *lprc;
	rc.top = y - 1;
	rc.bottom = y + iHeight + 2;
	InvertRect(hdc, &rc);
}

// �E�C���h�E�T�C�Y�𒲐����܂��B
//   lprc [in]: �͈�
void CGrSplitWnd::setWindowSize(LPRECT lprc, bool bwl)
{
	if(!bwl && m_oldRect.right > INT_MIN){
		if(m_bHorz){
			m_splitPos = lprc->bottom - (m_oldRect.bottom - m_splitPos);
		} else {
			m_splitPos = lprc->right - (m_oldRect.right - m_splitPos);
		}
	}
	int splitPos = m_splitPos;
	if(m_bHorz){
		if(splitPos < lprc->top){
			splitPos = lprc->top;
		} else if(splitPos > (lprc->bottom-GetSplitWidth())){
			splitPos = lprc->bottom-GetSplitWidth();
		}
	} else {
		if(splitPos < lprc->left){
			splitPos = lprc->left;
		} else if(splitPos > (lprc->right-GetSplitWidth())){
			splitPos = lprc->right-GetSplitWidth();
		}
	}
	int num = 1;
	if(m_leftWnd ){ ++num; }
	if(m_rightWnd){ ++num; }
	m_oldRect = *lprc;
	auto leftRect = *lprc;
	auto rightRect = *lprc;
	if(m_bHorz){
		lprc->top = splitPos;
		rightRect.top = splitPos + GetSplitWidth();
		leftRect.bottom = splitPos;
		SetWindowPosTop(lprc);
	} else {
		lprc->left = splitPos;
		rightRect.left = splitPos + GetSplitWidth();
		leftRect.right = splitPos;
		SetWindowPosLeft(lprc);
	}
	resizeLeftWindow (leftRect );
	resizeRightWindow(rightRect);
}

////////////////////////////////////////////////
// �C�x���g
// �E�C���h�E�̔j��
void CGrSplitWnd::OnDestroy(HWND hwnd)
{
	ReleaseCapture(); // �}�E�X���
}

// �}�E�X���{�^���̏���
// ���{�^���_�E��
void CGrSplitWnd::OnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
	if(!fDoubleClick){ // �V���O���N���b�N��
		SetCapture(hwnd); // �}�E�X�擾
		m_bMove = true;
		m_pos.x = INT_MIN;
		m_pos.y = INT_MIN;
		drawMoveBar();
	}
}

// �}�E�X���{�^���̏���
// ���{�^���A�b�v
//   �}�E�X�����
void CGrSplitWnd::OnLButtonUp(HWND hwnd, int x, int y, UINT flags)
{
	RECT  moveRc;
	POINT pos;

	removeMoveBar();                              // �ړ��o�[���폜���܂��B
	getMoveLimit(&pos, &moveRc);                  // �ړ�����

	ScreenToClient(GetParent(hwnd), &pos);
	SetPosition((m_bHorz) ? pos.y : pos.x);       // �ʒu����
	ScreenToClientRect(GetParent(hwnd), &moveRc); // ���W�ϊ�
	setWindowSize(&moveRc, true);                 // �E�C���h�E�T�C�Y�𒲐����܂��B
}

// �}�E�X�E�{�^���̏���
// �E�{�^���_�E��
void CGrSplitWnd::OnRButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
	removeMoveBar(); // �ړ��o�[���폜���܂��B
}

// �}�E�X�i�ړ��j
void CGrSplitWnd::OnMouseMove(HWND hwnd, int x, int y, UINT keyFlags)
{
	drawMoveBar(); // �ړ��o�[��`��
}

BOOL CGrSplitWnd::OnSetCursor(HWND hwnd, HWND hwndCursor, UINT codeHitTest, UINT msg)
{
	if(hwnd == hwndCursor){
		SetCursor(m_cursor);
	} else {
		return FALSE;
	}
	return TRUE;
}

void CGrSplitWnd::resizeLeftWindow(RECT &rect)
{
	if(m_leftWnd){
		SetWindowPosAllClient(m_leftWnd, &rect);
	}
	if(m_pLSplit){
		auto tempRect = rect;
		m_pLSplit->SetWindowSize(&tempRect);
	}
}

void CGrSplitWnd::resizeRightWindow(RECT &rect)
{
	if(m_rightWnd){
		SetWindowPosAllClient(m_rightWnd, &rect);
	}
	if(m_pRSplit){
		auto tempRect = rect;
		m_pRSplit->SetWindowSize(&tempRect);
	}
}
