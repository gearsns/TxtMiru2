#ifndef __LUPEWND_H__
#define __LUPEWND_H__

#include "WindowCtrl.h"
#include "Font.h"
#include "TxtCanvas.h"

class CGrLupeWnd : public CGrWinCtrl
{
public:
	enum SelectType {
		ST_MousePos,
		ST_WindowPos
	};
	CGrLupeWnd();
	virtual ~CGrLupeWnd();
	void Show(HWND hWnd, int page);
	void Hide();
	void Attach(CGrTxtDocument *pDoc);
	void Update(int page = -1);
	void SetFromCanvasRect(const RECT &fromCanvasRect);
	void SetCanvasOffset(int x, int y);
	void UpdateParam();
	//
	void UpdatePos(POINT pos);
	void UpdateWindowPos();
	//
	void SetOffset(POINT pos);
	//
	void Zoom(int izoom);
	int GetZoom() const;
	//
	void SetSelectType(SelectType st);
	SelectType GetSelectType() const { return m_selectType; }
protected:
	// オーバーライド可能なウィンドウ(ダイアログ)プロシージャ
	virtual LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
	void OnSize(HWND hwnd, UINT state, int cx, int cy);
	void OnPaint(HWND hwnd);
	void OnNCMouseMove(HWND hwnd, int x, int y, UINT codeHitTest);
	void OnMove(HWND hwnd, int x, int y);
	BOOL InitApp(HINSTANCE hInst, WNDPROC fnWndProc, LPCTSTR szClassNm);
private:
	void setWindowSize(int cx, int cy);
	bool m_bHide = true;
	RECT m_fromCanvasRect;
	CGrTxtCanvas *m_pTxtCanvas = nullptr;
	CGrTxtDocument *m_pDoc = nullptr;
	POINT m_offset;
	int m_lupeSize = -1;
	int m_canvasLupeSize = -1;
	SelectType m_selectType = ST_WindowPos;
	int m_windowOffsetX     = 0;
	int m_windowOffsetY     = 0;
};

#endif // __LUPEWND_H__
