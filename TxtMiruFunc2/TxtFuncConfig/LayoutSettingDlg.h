#ifndef __LAYOUTSETTINGDLG_H__
#define __LAYOUTSETTINGDLG_H__

#include "WindowCtrl.h"
#include <vector>
#include <map>
#include "TxtMiruDef.h"
#include "Bitmap.h"

class CGrLayoutSettingDlg : public CGrWinCtrl
{
protected:
	CGrBitmap m_bmpWork;
	int m_focusID = 0;
	std::map<UINT,bool> m_error_id_list;
	HBRUSH m_editBrush = NULL;
	SIZE m_minSize = {};

	std::tstring m_type;
	std::tstring m_name;
	std::tstring m_filename;
protected:
	CGrLayoutSettingDlg(LPCTSTR type, LPCTSTR name, LPCTSTR lpFileName);
	virtual ~CGrLayoutSettingDlg();
public:
	virtual int DoModal(HWND hWnd) = 0;
protected:
	// オーバーライド可能なウィンドウ(ダイアログ)プロシージャ
	virtual LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam) = 0;
	virtual void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify) = 0;
	HBRUSH OnCtlColor(HWND hwnd, HDC hdc, HWND hwndChild, int type);
	BOOL OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pLResult);
	void OnDrawItem(HWND hwnd, const DRAWITEMSTRUCT * lpDrawItem);
	void OnSize(HWND hwnd, UINT nType, int cx, int cy);
	void OnGetMinMaxInfo(HWND hwnd, LPMINMAXINFO lpMinMaxInfo);
	void OnTimer(HWND hwnd, UINT id);
	virtual void Refresh() = 0;
protected:
	int getValue(int id);
	virtual void drawLayout(const DRAWITEMSTRUCT * lpDrawItem) = 0;
};

#endif // __LAYOUTSETTINGDLG_H__
