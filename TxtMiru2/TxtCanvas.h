#ifndef __TXTCANVAS_H__
#define __TXTCANVAS_H__

#include "TxtMiruDef.h"
#include "TxtParam.h"
#include "Bitmap.h"
#include "TxtMapper.h"
#include "TxtDocument.h"
#include "TxtRenderer.h"
#include "Tooltips.h"
#include <vector>
#include "stlutil.h"
#include "PageFlip.h"

class CGrTxtCanvasDraw;
class CGrTxtCanvas
{
friend CGrTxtCanvasDraw;
public:
	CGrTxtCanvas();
	~CGrTxtCanvas();
	void Resize(int cx, int cy);
	int Update(int page = -1);
	void Draw(HDC hdc, const RECT &rect, const RECT &paintRect, int offset_x=0, int offset_y=0);
	void Attach(CGrTxtDocument *pDoc);
	void UpdateParam();
	bool SetBeginSelect(const POINT &pos);
	bool SetEndSelect(const POINT &pos);
	void SetBeginSelect(const TxtMiru::TextPoint &tp);
	void SetEndSelect(const TxtMiru::TextPoint &tp);
	void ValidateSelect(bool bSelect);
	bool GetSelectTextPoint(TxtMiru::TextPoint &tpb, TxtMiru::TextPoint &tpe);
	void Initialize();
	void Clear();
	void ClearCache();
	int SetPageFlip(int page, bool bScroll = false);
	bool PageFilpNext();
	const int GetPatternOffset(int iWidth){ return m_pageFlip.GetOffset(iWidth); }

	void SetAntialias(int iAntialias);
	int GetAntialias();

	CGrBitmap &GetBitmap(){ return m_bmpCanvas; }

	const CGrToolTips::TipsInfoList &GetTipsInfoList() const { return m_tipsList; };
	bool IsTextArea(int x, int y);
	int GetLinkIndex(int x, int y);
	int GetCurrentHoverLink();
	void SetCurrentHoverLink(int iHoverLink);
	LPCTSTR GetLinkURL(int id);

	void SetOffset(int x, int y);
	const RECT GetCanvasRect(){ return m_rectCanvas; }
	const RECT GetWindowRect(){ return m_rectWindow; }

protected:
	struct LinkBox {
		int id = 0;
		std::tstring url;
		RECT rect = {};
	};
	using LinkBoxList = std::vector<LinkBox>;
	LinkBoxList m_linkBox;
	struct LayoutBox {
		RECT org_rc = {};
		RECT rc = {};
		int min_line = 0;
		int max_line = 0;
	};
	using LayoutBoxList = std::simple_array<LayoutBox>;
	std::tstring m_bkImageFilename;
	LayoutBoxList m_layoutBox;
	CGrBitmap m_bmpCanvas;
	CGrTxtMapper m_txtMap;
	CGrPageFlip m_pageFlip;

	RECT m_rectCanvas = {};
	RECT m_rectWindow = {};

	CGrToolTips::TipsInfoList m_tipsList;

	CGrTxtDocument *m_pDoc = nullptr;
	CGrTxtRendererMgr m_txtRenderer;

	int m_fontTextSize =  0;      // テキストのフォントサイズ
	int m_startLine    =  0;         // 表示開始の(画面表示上の)行
	int m_totalPage    =  0;
	int m_currentPage  =  0;
	int m_iHoverLink   = -1;
	int m_offsetX      =  0; // フルスクリーン表示の時のMenuの高さ取得用
	int m_offsetY      =  0; //
	//
	struct TextCharPoint {
		bool bEnable = false;
		TxtMiru::TextPoint tp;
		TxtMiru::CharPoint cp;
	};
	TextCharPoint m_selectBeginPoint;
	TextCharPoint m_selectEndPoint;
protected:
	void setFontSize(int iFont);
	bool isDrawArea(int col, int lines);
	bool getHitCharPointMap(int x, int y, TextCharPoint &out_tcp);
	bool getHitLastCharPointMap(int x, int y, TextCharPoint &out_tcp);
	void invertSelectRect(HDC hdc, const RECT &rect);
};

#endif // __TXTCANVAS_H__
