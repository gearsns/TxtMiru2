// ToolBar.c
//
// ツールバー
//
#define STRICT // 型を宣言および使用時に、より厳密な型チェックが行われます。

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "CSVText.h"
#include "ToolBar.h"
#include <algorithm>
#include <string>
#include <sstream>
#include "Text.h"
//
#define BANDSIZE_X     22  // バンドサイズ
#define BANDSIZE_Y     22
#define TBICONSIZE_X   16  // ボタンビットマップのサイズ
#define TBICONSIZE_Y   16
// ID
#define ID_TOOLBAR    101  // ツールバーのID
#define ID_COOLBAR    102  // クールバーのID
//
#define BUFSIZE       512  // 最大バッファサイズ
// ToolBar作成スタイル
#define TOOLBAR_STYLE (WS_CHILD | WS_VISIBLE | CCS_ADJUSTABLE/*TOOLBAR編集*/ | TBSTYLE_ALTDRAG/*Alt+D&Dで移動*/)

// クールバーを作成します。
//   hWnd   : HWND
//   戻り値 : ツールバーHWND
HWND CGrCoolBar::Create(HWND hWnd, HINSTANCE hInst, UINT id, UINT toobar_id)
{
	// CommonControlの初期化を行う
	InitCommonControlsEx(ICC_COOL_CLASSES | ICC_BAR_CLASSES);
	// クールバーの作成
	m_hWnd = CreateWindowEx(
		WS_EX_TOOLWINDOW,
		REBARCLASSNAME, NULL,
		/*WS_BORDER | */WS_VISIBLE | WS_CHILD /*| WS_CLIPCHILDREN*/ | WS_CLIPSIBLINGS | RBS_AUTOSIZE | RBS_VARHEIGHT | RBS_BANDBORDERS | RBS_FIXEDORDER,
		0, 0, TBICONSIZE_X, TBICONSIZE_Y+10, hWnd, reinterpret_cast<HMENU>(id), hInst, nullptr);

	m_toolBar.Create(m_hWnd, hInst, toobar_id);
	RECT rect;
	GetWindowRect(m_toolBar, &rect);

	RECT rc = {};
	GetClientRect(hWnd, &rc);

	REBARBANDINFO rbbi = {REBARBANDINFO_V3_SIZE/* <- XP用 sizeof(REBARBANDINFO)*/};
	rbbi.fMask = RBBIM_SIZE | RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_ID | RBBIM_STYLE | RBBIM_TEXT | RBBIM_BACKGROUND | RBBIM_COLORS;
	rbbi.cyChild    = rect.bottom - rect.top; // XPは先にサイズを指定してからRB_INSERTBANDしないとサイズが変わらない?
	rbbi.cxMinChild = rect.right - rect.left; // XPは先にサイズを指定してからRB_INSERTBANDしないとサイズが変わらない?
	rbbi.cyMinChild = rect.bottom - rect.top; // XPは先にサイズを指定してからRB_INSERTBANDしないとサイズが変わらない?
	rbbi.cx         = rc.right;
	rbbi.clrBack = GetSysColor(COLOR_3DFACE);
	rbbi.fStyle     = RBBS_FIXEDBMP;
	rbbi.wID        = id;
	rbbi.hwndChild  = m_toolBar;
	rbbi.lpText     = nullptr;
	SendDlgItemMessage(hWnd, id, RB_INSERTBAND, -1, reinterpret_cast<LPARAM>(&rbbi));

	return m_hWnd;
}

// ツールバーを作成します。
//   hWnd   : HWND
//   戻り値 : ツールバーHWND
HWND CGrToolBar::Create(HWND hWnd, HINSTANCE hInst, UINT id)
{
	if(!m_hWnd){
		// CommonControlの初期化を行う
		InitCommonControlsEx(ICC_COOL_CLASSES | ICC_BAR_CLASSES);

		// ツールバーの作成
		m_hWnd = CreateWindowEx(
			WS_EX_TOOLWINDOW,
			TOOLBARCLASSNAME, NULL,
			WS_CHILD | WS_VISIBLE | CCS_NODIVIDER | CCS_ADJUSTABLE | CCS_TOP | CCS_NOPARENTALIGN | TBSTYLE_LIST | TBSTYLE_TOOLTIPS,
			0, 0, TBICONSIZE_X, TBICONSIZE_Y+10, hWnd, (HMENU)id, hInst, nullptr);

		ModifyStyle(0, TBSTYLE_TRANSPARENT);
	}
	return m_hWnd;
}

void CGrToolBar::AddButtons(TBBUTTON *tbb, int max_num, TBADDBITMAP &tb)
{
	for(int i=0; i<max_num; i++){
		AddButton(tbb[i], tb);
	}
}
void CGrToolBar::AddButton(TBBUTTON &tbb, TBADDBITMAP &tb)
{
	m_tbb.push_back(tbb);

	// 標準のツールバービットマップの設定
	auto nIndex = SendMessage(m_hWnd, TB_ADDBITMAP, 1, reinterpret_cast<LPARAM>(&tb));
	registBtn(tbb.idCommand, nIndex);
}

bool CGrToolBar::GetTBBUTTON(int iItem, TBBUTTON &tb)
{
	if(iItem >= 0 && iItem < static_cast<signed int>(m_tbb.size())){
		tb = m_tbb[iItem];
		return true;
	}
	return false;
}

// ボタンの順を取得します。
//   戻り値 : ボタンの順
std::tstring &CGrToolBar::GetButtons()
{
	return m_buttons;
}

// ボタンを設定します。
//   buttons : ボタンの順
void CGrToolBar::SetButtons(LPCTSTR buttons)
{
	int i;

	if(_tcslen(buttons) == 0){
		return;
	}
	// 設定内容保存(リセット時に使用)
	std::tstring str = buttons;
	m_buttons = str;
	// 一旦全て削除
	i = SendMessage(m_hWnd, TB_BUTTONCOUNT, 0L, 0L);
	for(i=i-1; i>=0; i--){
		SendMessage(m_hWnd, TB_DELETEBUTTON, 0L, 0L);
	}

	SendMessage(m_hWnd, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);
	CGrCSVText csv_text;
	csv_text.AddTail(m_buttons.c_str());
	int num = csv_text.GetColmnSize(0);
	for(int idx=0; idx<num; idx++){
		auto *str = csv_text.GetString(0, idx);
		if(!str){
			continue;
		}
		i = _ttoi(str->c_str());
		if(i < static_cast<signed int>(m_tbb.size())){
			// アイコンを追加します。
			SendMessage(m_hWnd, TB_ADDBUTTONS, 1, reinterpret_cast<LPARAM>(&m_tbb[i]));
		}
	}
	SendMessage(m_hWnd, TB_AUTOSIZE, 0L, 0L);
}

void CGrToolBar::Reset()
{
	SetButtons(m_buttons.c_str());
}

// ボタンのビットマップインデックスを登録します。
//   id          : イベントID
//   bitmapIndex : 開始位置
void CGrToolBar::registBtn(int id, int bitmapIndex)
{
	int i;

	for(i=0; i<static_cast<signed int>(m_tbb.size()); i++){
		if(m_tbb[i].idCommand == id){
			m_tbb[i].iBitmap += bitmapIndex;
			return;
		}
	}
}

/////////////////////////////
// 以下イベントは
// 親のWM_NOTIFYで処理するか 親のWndProcからNotifyProcを呼ぶ
LRESULT CGrToolBar::NotifyProc(LPARAM lParam)
{
	auto *pHdr = reinterpret_cast<TBNOTIFY *>(lParam);
	TOOLTIPTEXT *pTT = nullptr;
	static TCHAR buf[BUFSIZE] = { };
	RECT rc = { };
	TPMPARAMS tpm = { };
	UINT dwtemp = 0;
	HMENU hMenu = NULL, hPopup = NULL;

	switch(pHdr->hdr.code)
	{
	case TBN_QUERYINSERT:
		return TRUE;
	case TBN_QUERYDELETE:
		return TRUE;
	case TBN_RESET:
		// リセット
		SetButtons(m_buttons.c_str());
		break;
	case TBN_GETBUTTONINFO:
		//ボタン文字列の取得
		if(pHdr->iItem < static_cast<signed int>(m_tbb.size())){
			pHdr->tbButton = m_tbb[pHdr->iItem];
			dwtemp = SendMessage(pHdr->hdr.hwndFrom, TB_GETSTATE, m_tbb[pHdr->iItem].idCommand, 0);
			if(dwtemp != -1){
				pHdr->tbButton.fsState = static_cast<BYTE>(dwtemp);
			}
			if(!pHdr->pszText){
				pHdr->pszText = buf;
				pHdr->cchText = sizeof(buf)/sizeof(TCHAR);
			}
			auto hInst = reinterpret_cast<HINSTANCE>(GetWindowLong(GetParent(pHdr->hdr.hwndFrom), GWL_HINSTANCE));
			if(::LoadString(hInst, m_tbb[pHdr->iItem].idCommand, pHdr->pszText, pHdr->cchText) == 0){
				*(pHdr->pszText) = '\0';
			}
			return TRUE;
		}
		return FALSE;
	case TTN_NEEDTEXT:
		//ツールチップ用の文字列の取得
		pTT = reinterpret_cast<TOOLTIPTEXT*>(lParam);
		pTT->hinst = reinterpret_cast<HINSTANCE>(GetWindowLong(GetParent(pHdr->hdr.hwndFrom), GWL_HINSTANCE));
		pTT->lpszText = MAKEINTRESOURCE(pTT->hdr.idFrom);
		break;
	}
	return FALSE;
}

LRESULT CGrToolBar::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg){
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
		if(::SendMessage(hWnd, TB_GETHOTITEM, 0, 0) >= 0){
			ReleaseCapture();
		}
		return 0;
	default:
		return CGrWinCtrl::WndProc(hWnd, uMsg, wParam, lParam);
	}
	return FALSE;
}
