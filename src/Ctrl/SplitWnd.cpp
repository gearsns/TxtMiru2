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

// ウィンドウ・クラスの登録
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

// メインウィンドウ
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

// スプリットバーの幅を取得します。
//   戻り値 [out]: 幅
int CGrSplitWnd::GetSplitWidth()
{
	RECT rc;
	::GetWindowRect(m_hWnd, &rc);
	return (m_bHorz) ? (rc.bottom - rc.top) : (rc.right - rc.left);
}

// 水平垂直を切り替えます。
//   bHorz [in]: true : 水平 false : 垂直
void CGrSplitWnd::SetMode(bool bHorz)
{
	m_bHorz = bHorz;
	m_cursor = static_cast<HCURSOR>(LoadCursor(NULL, (bHorz) ? IDC_SIZENS : IDC_SIZEWE));
}

// 移動バーを削除します。
void CGrSplitWnd::removeMoveBar()
{
	ReleaseCapture(); // マウス解放
	//
	m_pos.x = INT_MIN;
	m_pos.y = INT_MIN;
	// 移動バーを消します。
	if(!m_bMove){ return ; }

	drawMoveBar();
	m_bMove = false;
	if(m_leftWnd ){ InvalidateRect(m_leftWnd , NULL, TRUE); }
	if(m_rightWnd){ InvalidateRect(m_rightWnd, NULL, TRUE); }
	InvalidateRect(m_hWnd, NULL, TRUE);
}

// 移動可能範囲を取得します。
//   lprc [out]: 移動可能範囲を受け取るアドレス
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

// 移動制限
//   lppos [out]: POINT
//   lprc  [out]: 範囲
void CGrSplitWnd::getMoveLimit(LPPOINT lppos, LPRECT lprc)
{
	getMoveRect(lprc); // 移動可能範囲を取得します。
	// 元の値を変えないように一旦コピーします。
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
	GetCursorPos(lppos); // 現在のマウス位置をスクリーン座標で取得
	limitsPos(lppos->x, rc.left, rc.right);
	limitsPos(lppos->y, rc.top, rc.bottom);
}

// 移動バーを描画します。
void CGrSplitWnd::drawMoveBar()
{
	RECT  rc;
	RECT  moveRc;
	POINT pos;
	auto  hParent = GetParent(m_hWnd);

	if(!m_bMove){ return; } // 移動中ではないので描画しません。

	getMoveLimit(&pos, &moveRc); // 移動制限

	GetWindowRect(hParent, &rc);

	auto hdc = GetWindowDC(hParent);

	if(m_bHorz){ // 水平
		moveRc.left -= rc.left;
		moveRc.right -= rc.left;
		// 移動した。
		if(m_pos.y != pos.y){
			// 前回の表示を消します。
			if(m_pos.y > INT_MIN){ drawHorzBar(hdc, &moveRc, m_pos.y-rc.top, m_width); }
			// バーの表示
			drawHorzBar(hdc, &moveRc, pos.y-rc.top,  m_width);
		}
	} else {    // 垂直
		moveRc.top -= rc.top;
		moveRc.bottom -= rc.top;
		// 移動した。
		if(m_pos.x != pos.x){
			// 前回の表示を消します。
			if(m_pos.x > INT_MIN){ drawVertBar(hdc, &moveRc, m_pos.x-rc.left, m_width); }
			// バーの表示
			drawVertBar(hdc, &moveRc, pos.x-rc.left, m_width);
		}
	}
	ReleaseDC(hParent, hdc);

	m_pos = pos;
}

// 垂直バーを描画
//   hdc    [in]: HDC
//   lprc   [in]: 描画範囲
//   x      [in]: 描画位置
//   iWidth [in]: バーの幅
void CGrSplitWnd::drawVertBar(HDC hdc, LPRECT lprc, int x, int iWidth)
{
	// 描画範囲外
	if(x == INT_MIN){ return; }
	// 元の値を変えないように一旦コピーします。
	auto rc = *lprc;
	rc.left = x - 1;
	rc.right = x + iWidth + 2;
	InvertRect(hdc, &rc);
}

// 水平バーを描画
//   hdc    [in]:  HDC
//   lprc   [in]:  描画範囲
//   x      [in]:  描画位置
//   iWidth [in]:  バーの幅
void CGrSplitWnd::drawHorzBar(HDC hdc, LPRECT lprc, int y, int iHeight)
{
	// 描画範囲外
	if(y == INT_MIN){ return; }
	// 元の値を変えないように一旦コピーします。
	auto rc = *lprc;
	rc.top = y - 1;
	rc.bottom = y + iHeight + 2;
	InvertRect(hdc, &rc);
}

// ウインドウサイズを調整します。
//   lprc [in]: 範囲
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
// イベント
// ウインドウの破棄
void CGrSplitWnd::OnDestroy(HWND hwnd)
{
	ReleaseCapture(); // マウス解放
}

// マウス左ボタンの処理
// 左ボタンダウン
void CGrSplitWnd::OnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
	if(!fDoubleClick){ // シングルクリック時
		SetCapture(hwnd); // マウス取得
		m_bMove = true;
		m_pos.x = INT_MIN;
		m_pos.y = INT_MIN;
		drawMoveBar();
	}
}

// マウス左ボタンの処理
// 左ボタンアップ
//   マウスを解放
void CGrSplitWnd::OnLButtonUp(HWND hwnd, int x, int y, UINT flags)
{
	RECT  moveRc;
	POINT pos;

	removeMoveBar();                              // 移動バーを削除します。
	getMoveLimit(&pos, &moveRc);                  // 移動制限

	ScreenToClient(GetParent(hwnd), &pos);
	SetPosition((m_bHorz) ? pos.y : pos.x);       // 位置調整
	ScreenToClientRect(GetParent(hwnd), &moveRc); // 座標変換
	setWindowSize(&moveRc, true);                 // ウインドウサイズを調整します。
}

// マウス右ボタンの処理
// 右ボタンダウン
void CGrSplitWnd::OnRButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
	removeMoveBar(); // 移動バーを削除します。
}

// マウス（移動）
void CGrSplitWnd::OnMouseMove(HWND hwnd, int x, int y, UINT keyFlags)
{
	drawMoveBar(); // 移動バーを描画
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
