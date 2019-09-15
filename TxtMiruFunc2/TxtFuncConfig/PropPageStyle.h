#ifndef __PROPPAGESTYLE_H__
#define __PROPPAGESTYLE_H__

#include "PropPageSize.h"
#include "ColorButton.h"
#include "TxtLayoutMgr.h"
#include <set>
#include <vector>
typedef std::set<std::tstring> FontNameList;

#include "ComonCtrl.h"
class CGrEditImgFile : public CGrComonCtrl
{
public:
	CGrEditImgFile() : CGrComonCtrl(){}
	virtual ~CGrEditImgFile(){}
protected:
	virtual LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

class CGrPropPageStyle : public CGrPropPageSize
{
public:
	static CGrPropPage* PASCAL CreateProp(CGrTxtConfigDlg *pDlg);
	virtual ~CGrPropPageStyle();
	virtual bool Apply();
private:
	CGrColorButton m_colBtnText;
	CGrColorButton m_colBtnBold;
	CGrColorButton m_colBtnRuby;
	CGrColorButton m_colBtnNote;
	CGrColorButton m_colBtnNombre;
	CGrColorButton m_colBtnRunningHeads;
	CGrColorButton m_colBtnPaperBkColor;
	CGrColorButton m_colBtnPaperShadowBkColor;
	CGrColorButton m_colBtnBkColor;
	CGrColorButton m_colBtnLinkColor;
	CGrEditImgFile m_imgEditBox;
	FontNameList m_fontNameList;
	std::vector<CGrTxtLayoutMgr::LaytoutInfo> m_lil;
private:
	CGrPropPageStyle(CGrTxtConfigDlg *pDlg);
protected:
	BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
	void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
protected:
	bool bApply = false;
	// オーバーライド可能なウィンドウ(ダイアログ)プロシージャ
	virtual LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	// コンボボックスに文字列を追加します。
	//   ctrl_id [in]: コンボボックスのID
	//   str_id  [in]: 文字列のID
	//   data    [in]: DATA
	void comboAddString(int ctrl_id, int str_id, int data);
	// button attach window handle
	//   id    [in]: ID
	//   btn   [in]: CGrColorButton
	//   color [in]: color
	void attachBtn(UINT id, CGrColorButton &btn, COLORREF color);
};


#endif // __PROPPAGESTYLE_H__
