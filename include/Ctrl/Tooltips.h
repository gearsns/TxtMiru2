#ifndef __TOOLTIPS_H__
#define __TOOLTIPS_H__

#include "ComonCtrl.h"
#include <vector>

class CGrToolTips : public CGrComonCtrl
{
public:
	struct TipsInfo
	{
		RECT         rect = {};
		std::tstring str ;
	};
	typedef std::vector<TipsInfo> TipsInfoList;
public:
	CGrToolTips();
	virtual HWND Create(HWND hWnd);

	void SetTipsList(HWND hWnd, const TipsInfoList &tipsList);
	void AddToolTips(HWND targetWnd, int uId, LPCTSTR str, const RECT &rect);
	void DellToolTips(HWND hWnd, int uId = 0);
	void AllDellToolTips(HWND hWnd);
	void SetRect(const RECT &rect);
	void Pop();
	int GetToolCount() const;
private:
protected:
	// オーバーライド可能なウィンドウ(ダイアログ)プロシージャ
	virtual LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
private:
protected:
};

#endif
