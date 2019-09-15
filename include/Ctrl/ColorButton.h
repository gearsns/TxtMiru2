#ifndef __COLOR_BUTTON_H__
#define __COLOR_BUTTON_H__

#include "ComonCtrl.h"

class CGrColorButton : public CGrComonCtrl
{
public:
	CGrColorButton();

	// カラー選択ボタンを作成します。
	//   hWnd    [in]: 親ウインドウのハンドル
	//   戻り値 [out]: 作成したステータスウインドウのハンドル
	virtual HWND Create(HINSTANCE hInst, HWND hWnd);
	virtual void DrawItem(HWND hwnd, const DRAWITEMSTRUCT *lpDrawItemStruct);
	virtual BOOL OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pLResult);
	void OnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);

	// 色設定
	//   color [in]: COLORREF
	void SetColor(COLORREF color);
	// 色取得
	//   戻り値 [out]: COLORREF
	COLORREF GetColor() { return m_Color; }

	BOOL Attach(HWND hWnd);
private:
	COLORREF m_Color;
	COLORREF m_DefaultColor;
protected:
	// オーバーライド可能なウィンドウ(ダイアログ)プロシージャ
	virtual LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void DrawArrow(HDC hdc,
				   RECT* pRect,
				   int iDirection,
				   COLORREF clrArrow = RGB(0,0,0));
	// 色を選択します。
	//   戻り値 : 選択した場合は TRUE
	BOOL chooseColor();
};

#endif
