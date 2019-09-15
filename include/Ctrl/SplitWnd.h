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

	// 水平垂直を切り替えます。
	//   bHorz [in]: true : 水平 false : 垂直
	void SetMode(bool bHorz);
	bool GetMode(){ return m_bHorz; }
	// 左(上)のウインドウ
	HWND &SetLfWnd(HWND hwnd){ return m_leftWnd = hwnd; }
	HWND &GetLfWnd()         { return m_leftWnd; }
	// 右(下)のウインドウ
	HWND &SetRtWnd(HWND hwnd){ return m_rightWnd = hwnd; }
	HWND &GeRtWnd()          { return m_rightWnd; }

	// ウインドウサイズを調整します。
	//   lprc [in]: 範囲
	void SetWindowSize(LPRECT lprc){ setWindowSize(lprc, m_weightLeft); }
	// スプリットバーの位置設定
	//   pos [in]: 位置
	void SetPosition(int pos){ m_splitPos = pos; }
	int GetPosition(){ return m_splitPos; }

	// スプリットバーの幅を取得します。
	//   戻り値 [out]: 幅
	int GetSplitWidth();

	// 最小幅を設定します。
	//   min [in]: 最小幅
	void SetLeftMinWidth(int min){ m_leftMin = min; }
	int GetLeftMinWidth(){ return m_leftMin; }
	//   min [in]: 最小幅
	void SetRightMinWidth(int min){ m_rightMin = min; }
	int GetRightMinWidth(){ return m_rightMin; }

	// ウインドウの重みを変えます。
	//   bwl [in]: true 左側が重くなります。(デフォルト)
	void SetWeightLeft(bool bwl) { m_weightLeft = bwl; }
	bool GetWeightLeft() { return m_weightLeft; }
	//
	void SetSplitRWnd(CGrSplitWnd *wnd){ m_pRSplit = wnd; }
	void SetSplitLWnd(CGrSplitWnd *wnd){ m_pLSplit = wnd; }
	CGrSplitWnd *GetSplitRWnd(){ return m_pRSplit; }
	CGrSplitWnd *GetSplitLWnd(){ return m_pLSplit; }
protected:
	// ウインドウサイズを調整します。
	//   lprc [in]: 範囲
	void setWindowSize(LPRECT lprc, bool weightLeft);
	// 移動バーを削除します。
	void removeMoveBar();
	// 移動可能範囲を取得します。
	//   lprc [out]: 移動可能範囲を受け取るアドレス
	void getMoveRect(LPRECT lprc);
	// 移動制限
	//   lppos [out]: POINT
	//   lprc  [out]: 範囲
	void getMoveLimit(LPPOINT lppos, LPRECT lprc);
	// 移動バーを描画します。
	void drawMoveBar();
	// 垂直バーを描画
	//   hdc    [in]: HDC
	//   lprc   [in]: 描画範囲
	//   x      [in]: 描画位置
	//   iWidth [in]: バーの幅
	void drawVertBar(HDC hdc, LPRECT lprc, int x, int iWidth);
	// 水平バーを描画
	//   hdc    [in]:  HDC
	//   lprc   [in]:  描画範囲
	//   x      [in]:  描画位置
	//   iWidth [in]:  バーの幅
	void drawHorzBar(HDC hdc, LPRECT lprc, int y, int iHeight);

private:
	// ウィンドウ・クラスの登録
	BOOL initApp(HINSTANCE hInst, WNDPROC fnWndProc, LPCTSTR szClassNm);
protected:
	virtual void resizeLeftWindow (RECT &rect);
	virtual void resizeRightWindow(RECT &rect);
	// オーバーライド可能なウィンドウ(ダイアログ)プロシージャ
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
