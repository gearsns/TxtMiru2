//
// オプション設定
//
#define STRICT // 型を宣言および使用時に、より厳密な型チェックが行われます。

#include <windows.h>
#include <windowsx.h>
#include "PropPage.h"

CGrPropPage::CGrPropPage()
{
	m_title[0] = _T('\0');
	bApply = false;
}

static UINT CALLBACK PropSheetPageProc(HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp)
{
	if(uMsg != PSPCB_CREATE && uMsg != PSPCB_RELEASE){
		auto *dlg = reinterpret_cast<CGrWinCtrl *>(ppsp->lParam);
		dlg->Attach(hwnd);
		SendMessage(hwnd, WM_INITDIALOG, reinterpret_cast<WPARAM>(ppsp), reinterpret_cast<LPARAM>(hwnd));
	}
	return TRUE;
}

// メッセージを振り分けるウィンドウ(ダイアログ)プロシージャ
LRESULT CALLBACK CGrPropPage::PropWindowMapProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	// ウィンドウのプロパティリストからCGrWinCtrlへのポインタの取得を試みる
	auto* pTargetWnd = static_cast<CGrPropPage*>(GetProp(hWnd, _T("CPP_CLASS")));

	// CGrWinCtrlオブジェクトとウィンドウが結び付けられていない場合は
	// CGrWinCtrlオブジェクトとウィンドウを結び付ける
	if(!pTargetWnd){
		// CGrWinCtrlへのポインタを取得
		if(uMsg == WM_INITDIALOG){
			LPPROPSHEETPAGE ppsp = (LPPROPSHEETPAGE)lParam;
			pTargetWnd = reinterpret_cast<CGrPropPage*>(ppsp->lParam);
		}
		// ウィンドウハンドルとCGrWinCtrlオブジェクトを結び付ける
		if(pTargetWnd){
			pTargetWnd->Attach(hWnd);
		}
	}

	// pTargetWnd の取得に成功した場合は、CGrWinCtrlのWndProcを呼び出す
	if(pTargetWnd){
		// CGrWinCtrlのWndProcを呼び出す
		auto lResult = pTargetWnd->WndProc(hWnd, uMsg, wParam, lParam);
		// WM_DESTROYメッセージでウィンドウとCGrWinCtrlオブジェクトを切り離す
		// このWindowMapProcで行うことにより、派生クラスからウィンドウと
		// クラスの結びつきを意識せずにすむ
		if(uMsg == WM_DESTROY){
			pTargetWnd->Detach();
		}
		return lResult;
	}

	// ダイアログの場合、FALSEを返す
	if(GetWindowLong(hWnd, DWL_DLGPROC)){
		return FALSE;
	}

	// デフォルトウィンドウプロシージャを呼び出す
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

// ページの設定
//   hWnd [in]: HWND
//   psp  [in]: PROPSHEETPAGE
void CGrPropPage::SetPropSheetPage(HINSTANCE hInst, HWND hWnd, PROPSHEETPAGE *psp)
{
	// プロパティシート 1ページ目の設定
	psp->dwSize      = sizeof(PROPSHEETPAGE);
	psp->dwFlags     = PSP_USETITLE | PSP_HASHELP;
	psp->hInstance   = hInst;
	psp->pszTemplate = nullptr;
	psp->pszIcon     = nullptr;
	psp->pfnDlgProc  = reinterpret_cast<DLGPROC>(PropWindowMapProc);
	psp->pszTitle    = m_title;                           // ページ名
	psp->lParam      = reinterpret_cast<LPARAM>(this);
	psp->pfnCallback = nullptr;
}

// メインウィンドウ
LRESULT CGrPropPage::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HWND hcwnd = NULL;
	switch(uMsg){
	case WM_NOTIFY:
		{
			auto pnmh = reinterpret_cast<LPNMHDR>(lParam);
			if(pnmh && pnmh->hwndFrom != hWnd){
				hcwnd = pnmh->hwndFrom;
			}
		}
	case WM_DRAWITEM:
		{
			auto pdis = reinterpret_cast<LPDRAWITEMSTRUCT>(lParam);
			if(pdis && pdis->hwndItem != hWnd){
				hcwnd = pdis->hwndItem;
			}
		}
		break;
	}
	return FALSE;
}

LRESULT CGrPropPage::OnNotify(HWND hWnd, int idFrom, NMHDR FAR *lpnmhdr)
{
	switch(lpnmhdr->code){
	case PSN_APPLY:
		// 適用
		if(Apply()){
			bApply = true;
			::SetWindowLong(hWnd, DWL_MSGRESULT, PSNRET_NOERROR);
		} else {
			::SetWindowLong(hWnd, DWL_MSGRESULT, PSNRET_INVALID);
		}
		break;
	case PSN_RESET:
		break;
	}
	return TRUE;
}
