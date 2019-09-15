// ToolBar.h
//
// ツールバー
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
	// ツールバーを作成します。
	//   hWnd   : HWND
	//   戻り値 : ツールバーHWND
	HWND Create(HWND hWnd, HINSTANCE hInst, UINT id);
	// ボタンの順を取得します。
	//   戻り値 : ボタンの順
	std::tstring &GetButtons();
	// ボタンを設定します。
	//   buttons : ボタンの順
	void SetButtons(LPCTSTR buttons);
	void Reset();

	void AddButtons(TBBUTTON *tbb, int max_num, TBADDBITMAP &tb);
	void AddButton(TBBUTTON &tbb, TBADDBITMAP &tb);
	bool GetTBBUTTON(int iItem, TBBUTTON &tb);

	LRESULT NotifyProc(LPARAM lParam);
private:
	// ボタンの順を取得します。
	//   戻り値 : ボタンの順
	char *getButtons();

	// ボタンのビットマップインデックスを登録します。
	//   id          : イベントID
	//   bitmapIndex : 開始位置
	void registBtn(int id, int bitmapIndex);
protected:
	// オーバーライド可能なウィンドウ(ダイアログ)プロシージャ
	virtual LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
private:
	TBB_VEC       m_tbb;
	std::tstring  m_buttons;
};

class CGrCoolBar : public CGrWinCtrl
{
public:
	// ツールバーを作成します。
	//   hWnd   : HWND
	//   戻り値 : ツールバーHWND
	HWND Create(HWND hWnd, HINSTANCE hInst, UINT id, UINT toobar_id);

	CGrToolBar &GetToolBar(){ return m_toolBar; }
private:
	CGrToolBar m_toolBar;
};

#endif
