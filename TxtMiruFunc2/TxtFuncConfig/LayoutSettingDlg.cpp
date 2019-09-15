#pragma warning( disable : 4786 )

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "resource.h"
#include "LayoutSettingDlg.h"
#include "MessageBox.h"
#include "TxtLayout.h"
#include "Text.h"
#include "Shell.h"
#include "TxtFuncConfig.h"
#include "TxtFunc.h"
#include "PaperSizeDlg.h"
#include <time.h>
#include <tuple>

#define UPDATE_LAYOUT_MSG 0x01
#define ERROR_CONTROL_COLOR RGB(0xff,0xb0,0xd0)
#define RULER_SIZE 7
#define RULER_MEM_SIZE 5

//
extern void EnableDlgItemID(HWND hWnd, UINT id, BOOL flag);
static UINT GetCheckDlgItemID(HWND hWnd, UINT id)
{
	return Button_GetCheck(GetDlgItem(hWnd, id));
}
static void updateLayout(HWND hwnd)
{
	::SetTimer(hwnd, UPDATE_LAYOUT_MSG, 500, NULL);
}
static void setImageWindowPos(HWND hwnd, int cx, int cy)
{
	auto hWndImg = GetDlgItem(hwnd, IDC_LAYOUT_IMG);
	if(hWndImg){
		RECT rect;
		GetWindowRect(hWndImg, &rect);
		ScreenToClient(hwnd, reinterpret_cast<LPPOINT>(&rect));
		MoveWindow(hWndImg, rect.left, 0, cx-rect.left, cy, FALSE);
		InvalidateRect(hWndImg, nullptr, FALSE);
	}
}
static void setImageWindowPos(HWND hwnd)
{
	RECT win_rect;
	GetClientRect(hwnd, &win_rect);
	setImageWindowPos(hwnd, win_rect.right - win_rect.left, win_rect.bottom - win_rect.top);
}
////////////////////
static void openLayout(CGrTxtLayout &layout, LPCTSTR type, LPCTSTR lpFileName)
{
	layout.Open(lpFileName);
}
static void saveUpdate(HWND hwnd, CGrTxtLayout &layout, LPCTSTR type)
{
	std::tstring filename;
	layout.GetFileName(filename);
	if(filename.empty()){
		CGrText::FormatMessage(filename, _T("%1!s!/Layout/%2!s!.lay"), CGrTxtFunc::GetDataPath(), type);
		layout.Save(filename.c_str());
	} else {
		layout.Save();
	}

	auto &&param = CGrTxtFunc::Param();
	param.SetText(CGrTxtFuncIParam::LayoutFile, filename.c_str());
	param.SetText(CGrTxtFuncIParam::LayoutType, layout.LayoutType());
	param.UpdateLayout(GetParent(hwnd));
	::SendMessage(GetParent(hwnd), WM_COMMAND, (WPARAM)TxtDocMessage::UPDATE_LAYOUT, (LPARAM)0);
}
//
static int calcPos(int pos, int ws, int ps)
{
	if(ps == 0.0){
		return 0;
	}
	return static_cast<int>(static_cast<double>(pos) * static_cast<double>(ws) / static_cast<double>(ps) + 0.5);
}
static void drawLine(HDC hdc, int left, int top, int right, int bottom)
{
	POINT pos;
	MoveToEx(hdc, left, top, &pos);
	LineTo(hdc, right, bottom);
}
static void drawVertLine(HDC hdc, int left, int top, int bottom){ drawLine(hdc, left, top, left, bottom); }
static void drawRect(HDC hdc, const RECT &rc){ Rectangle(hdc, rc.left, rc.top, rc.right, rc.bottom); }
static void drawRect(HDC hdc, int left, int top, int right, int bottom){ Rectangle(hdc, left, top, right, bottom); }
static void drawEllipse(HDC hdc, const RECT &rc){ ::Ellipse(hdc, rc.left, rc.top, rc.right, rc.bottom); }
static void drawRoundRect(HDC hdc, int left, int top, int right, int bottom){
	int r = min(right - left, bottom - top);
	RoundRect(hdc, left, top, right, bottom, r, r);
}
static void drawRoundRect(HDC hdc, const RECT &rc){ drawRoundRect(hdc, rc.left, rc.top, rc.right, rc.bottom); }
static void drawRuler(HDC hdc, int left, int top, int right, int bottom, int h_num, int v_num)
{
	auto hFocusPen  = CreatePen(PS_SOLID  , 2, RGB(0xFF,0xC0,0xD0));
	auto hFocusPen2 = CreatePen(PS_SOLID  , 1, RGB(0xFF,0x00,0xA0));
	auto oldPen = static_cast<HPEN>(::SelectObject(hdc, hFocusPen));

	int r = min(right - left, bottom - top) / 2;
	SelectObject(hdc, hFocusPen );
	drawRoundRect(hdc, left - RULER_SIZE - r, top - RULER_SIZE - r, right + RULER_SIZE + r, bottom + RULER_SIZE + r);
	SelectObject(hdc, hFocusPen2);
	drawRoundRect(hdc, left - RULER_SIZE - r, top - RULER_SIZE - r, right + RULER_SIZE + r, bottom + RULER_SIZE + r);
	if(h_num > 0){
		int step = (right - left) / h_num;
		int x = right;
		int t0 = top-RULER_MEM_SIZE;
		for(;h_num >= 0; --h_num, x -= step){
			drawLine(hdc, x, top, x, t0);
		}
		drawLine(hdc, left, top, right, top);
	}
	if(v_num > 0){
		int step = (bottom - top) / v_num;
		int y = top;
		int r0 = left-RULER_MEM_SIZE;
		for(;v_num >= 0; --v_num, y += step){
			drawLine(hdc, left, y, r0, y);
		}
		drawLine(hdc, left, top, left, bottom);
	}
	drawRect(hdc, left, top, right, bottom);

	SelectObject(hdc, oldPen);
	DeleteObject(hFocusPen );
	DeleteObject(hFocusPen2);
}
//
static void getRectValue(RECT &rect, HWND hwnd, UINT id_left, UINT id_top, UINT id_right, UINT id_bottom)
{
	BOOL bTranslated;
	rect.left   = GetDlgItemInt(hwnd, id_left  , &bTranslated, TRUE);
	rect.top    = GetDlgItemInt(hwnd, id_top   , &bTranslated, TRUE);
	rect.right  = GetDlgItemInt(hwnd, id_right , &bTranslated, TRUE);
	rect.bottom = GetDlgItemInt(hwnd, id_bottom, &bTranslated, TRUE);
}
static void getPointValue(POINT &point, HWND hwnd, UINT id_x, UINT id_y)
{
	BOOL bTranslated;
	point.x = GetDlgItemInt(hwnd, id_x, &bTranslated, TRUE);
	point.y = GetDlgItemInt(hwnd, id_y, &bTranslated, TRUE);
}
static void getLineSize(TxtMiru::LineSize &ls, HWND hwnd, UINT id_width, UINT id_space)
{
	BOOL bTranslated;
	ls.width = GetDlgItemInt(hwnd, id_width, &bTranslated, TRUE);
	if(id_space == 0){
		ls.space = 0;
	} else {
		ls.space = GetDlgItemInt(hwnd, id_space, &bTranslated, TRUE);
	}
}
static CGrTxtLayout::NombreFormatType getNombreFormatType(HWND hwnd, UINT id_outside, UINT id_center)
{
	if(IsDlgButtonChecked(hwnd, id_outside)){
		return CGrTxtLayout::NFT_outside;
	} else if(IsDlgButtonChecked(hwnd, id_center)){
		return CGrTxtLayout::NFT_center;
	}
	return CGrTxtLayout::NFT_inside;
}
struct ErrorCheck {
	int ctl_id = 0;
	UINT mes_id = 0;
	std::map<UINT,bool> &m_error_id_list;
	ErrorCheck(std::map<UINT,bool> & el) : m_error_id_list(el){}
	~ErrorCheck(){};
	void Check(int id, bool bCheck, int mes = IDS_ERROR_LAY_INPUT)
	{
		if(bCheck){
			m_error_id_list[id] = true;
			if(ctl_id == 0){
				ctl_id = id;
				mes_id = mes;
			}
		}
	}
};
//
struct LayoutInfoBase {
	SIZE paperSize;
	RECT paperSpace;
	int paperCenterXSpace;
	int paperCenterYSpace;
	int paperCenterYNum;
	POINT paperNombrePos[2];
	POINT paperRunningHeadsPos;
	TxtMiru::LineSize lsText        ;
	TxtMiru::LineSize lsRuby        ;
	TxtMiru::LineSize lsNombre      ;
	TxtMiru::LineSize lsNote        ;
	TxtMiru::LineSize lsRunningHeads;
	int lineWidth;
	CGrTxtLayout::NombreFormatType nombreFormatType;
	//
	int lines;
	int characters;
	//
	RECT windowRect;
	RECT clientRect;
	RECT canvasRect;
	SIZE clinetSize;
	POINT clientCenter;

	void setWinRect(const RECT &rect, CGrBitmap &bmp)
	{
		windowRect = rect;
		clientRect.left   = windowRect.left   + 5;
		clientRect.top    = windowRect.top    + 5;
		clientRect.right  = windowRect.right  - 5;
		clientRect.bottom = windowRect.bottom - 5;
		//
		clinetSize.cx = clientRect.right - clientRect.left;
		clinetSize.cy = clientRect.bottom - clientRect.top;
		//
		clientCenter.x = clinetSize.cx / 2;
		clientCenter.y = clinetSize.cy / 2;
		//
		int h = clinetSize.cx * paperSize.cy;
		int w = clinetSize.cy * paperSize.cx;
		if(h < w){
			clinetSize.cy = h / max(paperSize.cx, 1);
		} else {
			clinetSize.cx = w / max(paperSize.cy, 1);
		}
		canvasRect.top    = clientRect.top + clientCenter.y - clinetSize.cy / 2;
		canvasRect.bottom = canvasRect.top + clinetSize.cy;
		canvasRect.left   = clientRect.left + clientCenter.x - clinetSize.cx / 2;
		canvasRect.right  = canvasRect.left + clinetSize.cx;
		//
		if(bmp.Height() < (rect.bottom-rect.top) ||
		   bmp.Width () < (rect.right-rect.left)){
			bmp.Create(rect.right-rect.left, rect.bottom-rect.top);
		}
	}
	__declspec(noinline) int getWindowPosX(int paperPos)
	{
		return canvasRect.left + calcPos(paperPos, clinetSize.cx, paperSize.cx);
	}
	__declspec(noinline) int getWindowPosY(int paperPos)
	{
		return canvasRect.top + calcPos(paperPos, clinetSize.cx, paperSize.cx);
	}
	//
	__declspec(noinline) void refresh_sub(HWND hwnd)
	{
		nombreFormatType = getNombreFormatType(hwnd, IDC_RADIO_NOMBRE_OUT, IDC_RADIO_NOMBRE_CENTER);
		//                                 小口                   天                 小口                   地
		::getRectValue(paperSpace, hwnd, IDC_EDIT_PAGE_OUTSIDE, IDC_EDIT_PAGE_TOP, IDC_EDIT_PAGE_OUTSIDE, IDC_EDIT_PAGE_BOTTOM);
		//
		::getPointValue(paperNombrePos[0]   , hwnd, IDC_EDIT_NOMBRE1_LEFT      , IDC_EDIT_NOMBRE1_TOP      );
		::getPointValue(paperNombrePos[1]   , hwnd, IDC_EDIT_NOMBRE2_LEFT      , IDC_EDIT_NOMBRE2_TOP      );
		::getPointValue(paperRunningHeadsPos, hwnd, IDC_EDIT_RUNNING_HEADS_LEFT, IDC_EDIT_RUNNING_HEADS_TOP);
		//
		::getLineSize(lsText        , hwnd, IDC_EDIT_TEXT         , IDC_EDIT_TEXT_SPACE); // 本文文字サイズ, 行間
		::getLineSize(lsRuby        , hwnd, IDC_EDIT_RUBY         , IDC_EDIT_RUBY_SPACE); // ルビサイズ    , ルビ-文字間
		::getLineSize(lsNombre      , hwnd, IDC_EDIT_NOMBRE       ,                   0); // ノンブルサイズ, 行間
		::getLineSize(lsNote        , hwnd, IDC_EDIT_NOTE         , IDC_EDIT_NOTE_SPACE); // 注釈サイズ    , 行間
		::getLineSize(lsRunningHeads, hwnd, IDC_EDIT_RUNNING_HEADS,                   0); // 柱サイズ      , 行間
		//
		BOOL bTranslated;
		lines      = GetDlgItemInt(hwnd, IDC_EDIT_LINES     , &bTranslated, TRUE); // 行数
		characters = GetDlgItemInt(hwnd, IDC_EDIT_CHARACTERS, &bTranslated, TRUE); // 字数
		//
		lineWidth = lsText.width + lsText.space + lsRuby.width + lsRuby.space;
	}
	__declspec(noinline) void drawLIRuler(HDC hdc, int left, int top, int right, int bottom, int h_num, int v_num)
	{
		::drawRuler(hdc,
					getWindowPosX(left  ),
					getWindowPosY(top   ),
					getWindowPosX(right ),
					getWindowPosY(bottom),
					h_num, v_num);
	}
	__declspec(noinline) void setLayoutData(CGrTxtLayout &layout)
	{
		layout.Clear();

		layout.SetNombreFormatType(nombreFormatType);
		layout.SetMaxLine(lines*2);
		layout.SetMaxCharacters(characters);
		layout.SetPaper(paperSize);
		layout.SetLineSize(CGrTxtLayout::LST_Text        , lsText        );
		layout.SetLineSize(CGrTxtLayout::LST_Ruby        , lsRuby        );
		layout.SetLineSize(CGrTxtLayout::LST_Nombre      , lsNombre      );
		layout.SetLineSize(CGrTxtLayout::LST_Note        , lsNote        );
		layout.SetLineSize(CGrTxtLayout::LST_RunningHeads, lsRunningHeads);
	}
	__declspec(noinline) void setInitialData(HWND hwnd, CGrTxtLayout &layout)
	{
		SetDlgItemInt(hwnd, IDC_EDIT_PAPER_WIDTH , paperSize.cx, TRUE);
		SetDlgItemInt(hwnd, IDC_EDIT_PAPER_HEIGHT, paperSize.cy, TRUE);
		//
		lsText         = layout.GetLineSize(CGrTxtLayout::LST_Text        );
		lsRuby         = layout.GetLineSize(CGrTxtLayout::LST_Ruby        );
		lsNombre       = layout.GetLineSize(CGrTxtLayout::LST_Nombre      );
		lsNote         = layout.GetLineSize(CGrTxtLayout::LST_Note        );
		lsRunningHeads = layout.GetLineSize(CGrTxtLayout::LST_RunningHeads);
		//
		const TxtMiru::TxtLayoutList &tllNombre = layout.GetConstLayoutList(CGrTxtLayout::LLT_Nombre);
		if(tllNombre.size() == 2){
			paperNombrePos[0].x = tllNombre[0].left;
			paperNombrePos[0].y = tllNombre[0].top ;
			paperNombrePos[1].x = tllNombre[1].left;
			paperNombrePos[1].y = tllNombre[1].top ;
		}
		//
		const TxtMiru::TxtLayoutList &tllRH = layout.GetConstLayoutList(CGrTxtLayout::LLT_RunningHeads);
		if(tllRH.size() == 1){
			if(tllRH[0].left < paperSize.cx / 2){
				paperRunningHeadsPos.x = tllRH[0].left;
			} else {
				paperRunningHeadsPos.x = tllRH[0].right;
			}
			paperRunningHeadsPos.y = tllRH[0].top ;
		}
		//
		nombreFormatType = layout.GetNombreFormatType();
		UINT check_id = IDC_RADIO_NOMBRE_OUT;
		if(nombreFormatType == CGrTxtLayout::NFT_center){
			check_id = IDC_RADIO_NOMBRE_CENTER;
		} else if(nombreFormatType == CGrTxtLayout::NFT_inside){
			check_id = IDC_RADIO_NOMBRE_IN;
		}
		CheckRadioButton(hwnd, IDC_RADIO_NOMBRE_OUT, IDC_RADIO_NOMBRE_IN, check_id);
		//
		SetDlgItemInt(hwnd, IDC_EDIT_LINES     , lines     , TRUE);
		SetDlgItemInt(hwnd, IDC_EDIT_CHARACTERS, characters, TRUE);
		//
		SetDlgItemInt(hwnd, IDC_EDIT_TEXT         , lsText.width        , TRUE);
		SetDlgItemInt(hwnd, IDC_EDIT_TEXT_SPACE   , lsText.space        , TRUE);
		SetDlgItemInt(hwnd, IDC_EDIT_RUBY         , lsRuby.width        , TRUE);
		SetDlgItemInt(hwnd, IDC_EDIT_RUBY_SPACE   , lsRuby.space        , TRUE);
		SetDlgItemInt(hwnd, IDC_EDIT_NOMBRE       , lsNombre.width      , TRUE);
		SetDlgItemInt(hwnd, IDC_EDIT_NOTE         , lsNote.width        , TRUE);
		SetDlgItemInt(hwnd, IDC_EDIT_NOTE_SPACE   , lsNote.space        , TRUE);
		SetDlgItemInt(hwnd, IDC_EDIT_RUNNING_HEADS, lsRunningHeads.width, TRUE);
		//
		SetDlgItemInt(hwnd, IDC_EDIT_PAGE_TOP    , paperSpace.top   , TRUE);
		SetDlgItemInt(hwnd, IDC_EDIT_PAGE_OUTSIDE, paperSpace.right , TRUE);
		SetDlgItemInt(hwnd, IDC_EDIT_PAGE_BOTTOM , paperSpace.bottom, TRUE);
		//
		SetDlgItemInt(hwnd, IDC_EDIT_NOMBRE1_LEFT, paperNombrePos[0].x, TRUE);
		SetDlgItemInt(hwnd, IDC_EDIT_NOMBRE1_TOP , paperNombrePos[0].y, TRUE);
		SetDlgItemInt(hwnd, IDC_EDIT_NOMBRE2_LEFT, paperNombrePos[1].x, TRUE);
		SetDlgItemInt(hwnd, IDC_EDIT_NOMBRE2_TOP , paperNombrePos[1].y, TRUE);
		//
		SetDlgItemInt(hwnd, IDC_EDIT_RUNNING_HEADS_LEFT, paperRunningHeadsPos.x, TRUE);
		SetDlgItemInt(hwnd, IDC_EDIT_RUNNING_HEADS_TOP , paperRunningHeadsPos.y, TRUE);
	}
	//
	__declspec(noinline) void drawCharacter(HDC hdc, int center, int top_pos, double text_v)
	{
		RECT character_rect;
		int text_h = getWindowPosX(lsText.width) - canvasRect.left;
		int x, y;
		int x_pos = 0;
		int right_pos1 = center - lsText.width;
		int right_pos2 = paperSize.cx - paperSpace.right - lsText.width;
		for(x=0; x<lines; ++x){
			int left1  = getWindowPosX(right_pos1 - x_pos);
			int left2  = getWindowPosX(right_pos2 - x_pos);
			int right1 = left1 + text_h;
			int right2 = left2 + text_h;
			double y_pos = top_pos;
			for(y=0; y<characters; ++y){
				character_rect.top    = getWindowPosY(static_cast<int>(y_pos+0.5));
				character_rect.bottom = character_rect.top + text_h;
				// 奇数(左)
				character_rect.left  = left1 ;
				character_rect.right = right1;
				drawEllipse(hdc, character_rect);
				// 偶数(右)
				character_rect.left  = left2 ;
				character_rect.right = right2;
				drawEllipse(hdc, character_rect);
				//
				y_pos += text_v;
			}
			x_pos += lineWidth;
		}
	}
	//
	__declspec(noinline) void drawOther(HDC hdc, int body_top, int body_bottom, int center)
	{
		RECT character_rect;
		// ルビ
		int x = paperSize.cx - paperSpace.right + lsRuby.space;
		character_rect.left   = getWindowPosX(x);
		character_rect.top    = body_top;
		character_rect.right  = getWindowPosX(x+lsRuby.width);
		character_rect.bottom = body_bottom;
		drawRoundRect(hdc, character_rect);
		x -= lineWidth;
		character_rect.left   = getWindowPosX(x);
		character_rect.right  = getWindowPosX(x+lsRuby.width);
		drawRoundRect(hdc, character_rect);
		// ノンブル
		for(int i=0; i<2; ++i){
			character_rect.left   = getWindowPosX(paperNombrePos[i].x);
			character_rect.top    = getWindowPosY(paperNombrePos[i].y);
			character_rect.right  = getWindowPosX(paperNombrePos[i].x + lsNombre.width * 3);
			character_rect.bottom = getWindowPosY(paperNombrePos[i].y + lsNombre.width);
			drawRoundRect(hdc, character_rect);
		}
		// 柱
		character_rect.left   = getWindowPosX(paperRunningHeadsPos.x);
		character_rect.top    = getWindowPosY(paperRunningHeadsPos.y);
		character_rect.right  = getWindowPosX(center);
		character_rect.bottom = getWindowPosY(paperRunningHeadsPos.y + lsRunningHeads.width);
		if(character_rect.left > character_rect.right){
			int tmp = character_rect.right;
			character_rect.right = character_rect.left;
			character_rect.left  = tmp;
		}
		drawRoundRect(hdc, character_rect);
		// 注記
		int paper_left = getWindowPosX(0);
		int y;
		int paper_x = paperSpace.left;
		int tmp = paper_x+lsNote.width;
		int note_width = getWindowPosX(tmp) - getWindowPosX(paper_x);
		int note_next  = getWindowPosX(tmp+lsNote.space) - getWindowPosX(paper_x);
		character_rect.left = getWindowPosX(paper_x-lsNote.space);
		if(note_width > 0){
			for(x=0; ; ++x){
				character_rect.right = character_rect.left;
				character_rect.left -= note_width;
				if(character_rect.left < paper_left){ break; }
				for(y=0; ; ++y){
					character_rect.top    = getWindowPosY(paperSpace.top+lsNote.width*y);
					character_rect.bottom = character_rect.top + note_width;
					if(character_rect.top >= body_bottom){ break; }
					drawEllipse(hdc, character_rect);
				}
				character_rect.left = character_rect.right - note_next;
			}
		}
	}
	//////////////////
	__declspec(noinline) void drawLineFocus(HDC hdc, int x)
	{
		drawLIRuler(hdc, x, paperSpace.top, paperSize.cx - paperSpace.right, paperSpace.top+lsText.width, lines, 0);
	}
	// ノンブル
	__declspec(noinline) void drawNombreFocus(HDC hdc, int idx)
	{
		const auto &pos = paperNombrePos[idx];
		drawLIRuler(hdc, pos.x, pos.y, pos.x + lsNombre.width * 3, pos.y + lsNombre.width, 2, 0);
	}
	// ページ
	__declspec(noinline) void drawPageOutside(HDC hdc)
	{
		drawLIRuler(hdc, 0, 0, paperSpace.left, paperSize.cy, 2, 0);
		drawLIRuler(hdc, paperSize.cx-paperSpace.right, 0, paperSize.cx, paperSize.cy, 2, 0);
	}
	__declspec(noinline) void drawPageInside(HDC hdc, int left, int width)
	{
		drawLIRuler(hdc, left, 0, left+width, paperSize.cy, 2, 0);
	}
	// 注記
	__declspec(noinline) void drawNoteFocus(HDC hdc)
	{
		int tmp = paperSpace.right - lsNote.space;
		drawLIRuler(hdc,
					tmp-lsNote.width, paperSpace.top,
					tmp, paperSize.cy-paperSpace.bottom,
					2, 0);
	}
	// 柱
	__declspec(noinline) void drawRunningHeadsFocus(HDC hdc, int right)
	{
		int left  = paperRunningHeadsPos.x;
		if(left > right){
			int tmp = left;
			left = right;
			right = tmp;
		}
		drawLIRuler(hdc,
					left,
					paperRunningHeadsPos.y,
					right,
					paperRunningHeadsPos.y + lsRunningHeads.width,
					2, 0);
	}
	//
	__declspec(noinline) void drawOtherFocus(HDC hdc, int id, int center, int center_space)
	{
		switch(id){
		case IDC_EDIT_LINES             : drawLineFocus(hdc, center+center_space); break;
		case IDC_EDIT_NOTE              : drawNoteFocus(hdc); break;
		case IDC_EDIT_NOTE_SPACE        : drawLIRuler(hdc, paperSpace.right-lsNote.space, paperSpace.top, paperSpace.right, paperSize.cy-paperSpace.bottom, 2, 0); break;
		case IDC_EDIT_PAGE_INSIDE       : drawPageInside(hdc, center, center_space); break;
		case IDC_EDIT_PAGE_TOP          : drawLIRuler(hdc, 0, 0, paperSize.cx, paperSpace.top, 0, 2); break;
		case IDC_EDIT_PAGE_BOTTOM       : drawLIRuler(hdc, 0, paperSize.cy - paperSpace.bottom, paperSize.cx, paperSize.cy, 0, 2); break;
		case IDC_EDIT_PAGE_OUTSIDE      : drawPageOutside    (hdc); break;
		case IDC_EDIT_NOMBRE1_LEFT      : /*through*/
		case IDC_EDIT_NOMBRE1_TOP       : drawNombreFocus(hdc, 0); break;
		case IDC_EDIT_NOMBRE2_LEFT      : /*through*/
		case IDC_EDIT_NOMBRE2_TOP       : /*through*/
		case IDC_EDIT_NOMBRE            : drawNombreFocus(hdc, 1); break;
		case IDC_EDIT_RUNNING_HEADS_LEFT: /*through*/
		case IDC_EDIT_RUNNING_HEADS_TOP : /*through*/
		case IDC_EDIT_RUNNING_HEADS     : drawRunningHeadsFocus(hdc, center); break;
		}
	}
};
////////////////////
CGrLayoutSettingDlg::CGrLayoutSettingDlg(LPCTSTR t, LPCTSTR n, LPCTSTR lpFileName) : m_type(t), m_name(n), m_filename(lpFileName)
{
	m_editBrush = ::CreateSolidBrush(ERROR_CONTROL_COLOR);
}

CGrLayoutSettingDlg::~CGrLayoutSettingDlg()
{
	::DeleteObject(m_editBrush);
}

LRESULT CGrLayoutSettingDlg::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg){
		HANDLE_MSG(hWnd, WM_INITDIALOG    , OnInitDialog   );
		HANDLE_MSG(hWnd, WM_COMMAND       , OnCommand      );
		HANDLE_MSG(hWnd, WM_CTLCOLOREDIT  , OnCtlColor     );
		HANDLE_MSG(hWnd, WM_CTLCOLORSTATIC, OnCtlColor     );
		HANDLE_MSG(hWnd, WM_DRAWITEM      , OnDrawItem     );
		HANDLE_MSG(hWnd, WM_SIZE          , OnSize         );
		HANDLE_MSG(hWnd, WM_TIMER         , OnTimer        );
		HANDLE_MSG(hWnd, WM_GETMINMAXINFO , OnGetMinMaxInfo);
		break;
	}
	return CGrWinCtrl::WndProc(hWnd, uMsg, wParam, lParam);
}

void CGrLayoutSettingDlg::OnTimer(HWND hwnd, UINT id)
{
	if(id == UPDATE_LAYOUT_MSG){
		::KillTimer(m_hWnd, UPDATE_LAYOUT_MSG);
		Refresh();
	}
}
void CGrLayoutSettingDlg::OnGetMinMaxInfo(HWND hwnd, LPMINMAXINFO lpMinMaxInfo)
{
	lpMinMaxInfo->ptMinTrackSize.x = m_minSize.cx;
	lpMinMaxInfo->ptMinTrackSize.y = m_minSize.cy;
}

void CGrLayoutSettingDlg::OnSize(HWND hwnd, UINT nType, int cx, int cy)
{
	::setImageWindowPos(m_hWnd, cx, cy);
}

int CGrLayoutSettingDlg::getValue(int id)
{
	BOOL bTranslated;
	return GetDlgItemInt(m_hWnd, id, &bTranslated, TRUE);
}

void CGrLayoutSettingDlg::OnDrawItem(HWND hwnd, const DRAWITEMSTRUCT * lpDrawItem)
{
	if(lpDrawItem->CtlID == IDC_LAYOUT_IMG){
		::KillTimer(m_hWnd, UPDATE_LAYOUT_MSG);
		drawLayout(lpDrawItem);
	}
}

BOOL CGrLayoutSettingDlg::OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pLResult)
{
	return TRUE;
}

HBRUSH CGrLayoutSettingDlg::OnCtlColor(HWND hwnd, HDC hdc, HWND hwndChild, int type)
{
	if(m_error_id_list[GetDlgCtrlID(hwndChild)]){
		::SetTextColor(hdc, RGB(0xa0,0,0));	// テキストの色
		::SetBkColor(hdc,ERROR_CONTROL_COLOR);	// テキストが書かれている部分のテキストの背景の色
		return m_editBrush;
	}
	return reinterpret_cast<HBRUSH>(CGrWinCtrl::WndProc(hwnd, GET_WM_CTLCOLOR_MSG(type), GET_WM_CTLCOLOR_MPS(hdc, hwnd, type)));
}
//////////////////////////
class CGrCustomLayoutSettingDlg : public CGrLayoutSettingDlg
{
private:
	CGrCustomTxtLayout m_layout;
	struct LayoutInfo : public LayoutInfoBase {
		LayoutInfo(){
			// 105×148
			paperSize.cx = 105*2*100;
			paperSize.cy = 148  *100;
		}
		void refresh(CGrCustomLayoutSettingDlg &dlg);
	} m_li;
	void Calc(int id);
	void getLayoutName(std::tstring &name);
public:
	CGrCustomLayoutSettingDlg(LPCTSTR type, LPCTSTR name, LPCTSTR lpFileName);
	virtual ~CGrCustomLayoutSettingDlg();
	virtual int DoModal(HWND hWnd);
protected:
	// オーバーライド可能なウィンドウ(ダイアログ)プロシージャ
	virtual LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
	virtual void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
	virtual void Refresh();
private:
	void setInitialData();
	std::tuple<int, int> errorCheck();
	void setTxtLayout();
	virtual void drawLayout(const DRAWITEMSTRUCT * lpDrawItem);
};
////
CGrCustomLayoutSettingDlg::CGrCustomLayoutSettingDlg(LPCTSTR type, LPCTSTR name, LPCTSTR lpFileName) : CGrLayoutSettingDlg(type, name, lpFileName)
{
}

CGrCustomLayoutSettingDlg::~CGrCustomLayoutSettingDlg()
{
}

int CGrCustomLayoutSettingDlg::DoModal(HWND hWnd)
{
	return DialogBoxParam(CGrTxtFunc::GetDllModuleHandle(), MAKEINTRESOURCE(IDD_LAYOUTSET), hWnd, (DLGPROC)CGrWinCtrl::WindowMapProc, (LPARAM)this);
}

LRESULT CGrCustomLayoutSettingDlg::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg){
		HANDLE_MSG(hWnd, WM_INITDIALOG    , OnInitDialog   );
		HANDLE_MSG(hWnd, WM_COMMAND       , OnCommand      );
		HANDLE_MSG(hWnd, WM_CTLCOLOREDIT  , OnCtlColor     );
		HANDLE_MSG(hWnd, WM_CTLCOLORSTATIC, OnCtlColor     );
		HANDLE_MSG(hWnd, WM_DRAWITEM      , OnDrawItem     );
		HANDLE_MSG(hWnd, WM_SIZE          , OnSize         );
		HANDLE_MSG(hWnd, WM_TIMER         , OnTimer        );
		HANDLE_MSG(hWnd, WM_GETMINMAXINFO , OnGetMinMaxInfo);
		break;
	}
	return CGrWinCtrl::WndProc(hWnd, uMsg, wParam, lParam);
}

void CGrCustomLayoutSettingDlg::Refresh()
{
	m_li.refresh(*this);
	if(::GetCheckDlgItemID(m_hWnd, IDC_CHECK_AUTOCALC) == BST_CHECKED){
		Calc(0);
		m_li.refresh(*this);
	}
	::InvalidateRect(GetDlgItem(m_hWnd, IDC_LAYOUT_IMG),NULL,FALSE);
	errorCheck();
}

void CGrCustomLayoutSettingDlg::LayoutInfo::refresh(CGrCustomLayoutSettingDlg &dlg)
{
	paperCenterXSpace = dlg.getValue(IDC_EDIT_PAGE_INSIDE  ); // のど
	paperCenterYSpace = dlg.getValue(IDC_EDIT_PAGE_DIV_SIZE); // 段間
	paperCenterYNum   = dlg.getValue(IDC_EDIT_PAGE_DIV_NUM ); // 段間
	refresh_sub(dlg);
}

void CGrCustomLayoutSettingDlg::drawLayout(const DRAWITEMSTRUCT * lpDrawItem)
{
	m_li.setWinRect(lpDrawItem->rcItem, m_bmpWork);
	//
	auto hdc = ::CreateCompatibleDC(lpDrawItem->hDC);
	auto hbmp = static_cast<HBITMAP>(::SelectObject(hdc, m_bmpWork));
	auto hPen      = ::CreatePen(PS_SOLID, 1, RGB(0x00,0x00,0x00));
	auto hDotPen   = ::CreatePen(PS_DOT  , 1, RGB(0xA0,0xD0,0xFF));
	auto hFocusPen = ::CreatePen(PS_DOT  , 2, RGB(0xFF,0xD0,0xA0));
	auto hOldBrush = static_cast<HBRUSH>(::SelectObject(hdc, ::GetStockObject(NULL_BRUSH)));
	auto hOldPen   = static_cast<HPEN>(::SelectObject(hdc, hPen));

	auto oldclr = ::SetBkColor(hdc, RGB(0xff,0xff,0xff));
	//
	::ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &lpDrawItem->rcItem, NULL, 0, NULL); // 背景消去
	drawRect(hdc, m_li.canvasRect); // 文庫枠

	::SelectObject(hdc, hDotPen);
	int paper_center = m_li.paperSize.cx / 2;
	int body_bottom  = m_li.paperSize.cy-m_li.paperSpace.bottom;
	int bp_body_bottom = m_li.getWindowPosY(body_bottom);
	int bp_body_top    = m_li.getWindowPosY(m_li.paperSpace.top);
	int tmp;
	// 中央線
	drawVertLine(hdc, m_li.getWindowPosX(paper_center), m_li.canvasRect.top, m_li.canvasRect.bottom);
	// 基本版面枠
	{
		int num = m_li.paperCenterYNum;
		if(num < 1){
			num = 1;
		}
		int top = m_li.paperSpace.top;
		int step = (m_li.paperSize.cy-m_li.paperSpace.top-m_li.paperSpace.bottom+m_li.paperCenterYSpace) / num;
		int height = step - m_li.paperCenterYSpace;
		for(int i=0; i<num; ++i){
			int bottom = top + height;
			//   奇数(左)
			drawRect(hdc,
					 m_li.getWindowPosX(m_li.paperSpace.left), m_li.getWindowPosY(top),
					 m_li.getWindowPosX(paper_center - m_li.paperCenterXSpace), m_li.getWindowPosY(bottom));
			//   偶数(右)
			drawRect(hdc,
					 m_li.getWindowPosX(paper_center + m_li.paperCenterXSpace), m_li.getWindowPosY(top),
					 m_li.getWindowPosX(m_li.paperSize.cx-m_li.paperSpace.right), m_li.getWindowPosY(bottom));
			top += step;
		}
	}
	//
	::SelectObject(hdc, hPen);
	// 文字
	{
		int num = m_li.paperCenterYNum;
		if(num < 1){
			num = 1;
		}
		int characters = m_li.characters;
		if(characters < 1){
			characters = 1;
		}
		int top = m_li.paperSpace.top;
		int step = (m_li.paperSize.cy-m_li.paperSpace.top-m_li.paperSpace.bottom+m_li.paperCenterYSpace) / num;
		int height = step - m_li.paperCenterYSpace;
		double text_v = static_cast<double>(height) / static_cast<double>(characters);
		for(int i=0; i<num; ++i){
			int bottom = top + height;
			m_li.drawCharacter(hdc, paper_center - m_li.paperCenterXSpace, top, text_v);
			top += step;
		}
		body_bottom = m_li.paperSpace.top + height;
	}
	//
	m_li.drawOther(hdc, bp_body_top, bp_body_bottom, paper_center);
	//
	::SelectObject(hdc, hFocusPen);
	switch(m_focusID){
	case IDC_EDIT_CHARACTERS:
		tmp = paper_center+m_li.paperCenterXSpace;
		m_li.drawLIRuler(hdc,
						 tmp, m_li.paperSpace.top,
						 tmp+m_li.lsText.width, body_bottom,
						 0, m_li.characters);
		break;
	case IDC_EDIT_TEXT              :
		tmp = m_li.paperSize.cx-m_li.paperSpace.right;
		m_li.drawLIRuler(hdc,
						 tmp-m_li.lsText.width, m_li.paperSpace.top,
						 tmp, body_bottom,
						 2, 0);
		break;
	case IDC_EDIT_TEXT_SPACE        :
		tmp = m_li.paperSize.cx-m_li.paperSpace.right-m_li.lineWidth;
		m_li.drawLIRuler(hdc,
						 tmp, m_li.paperSpace.top,
						 tmp+m_li.lsText.space, body_bottom,
						 2, 0);
		break;
	case IDC_EDIT_RUBY              :
		tmp = m_li.paperSize.cx-m_li.paperSpace.right;
		m_li.drawLIRuler(hdc,
						 tmp, m_li.paperSpace.top,
						 tmp+m_li.lsRuby.width, body_bottom,
						 2, 0);
		break;
	case IDC_EDIT_RUBY_SPACE        :
		tmp = m_li.paperSize.cx-m_li.paperSpace.right-m_li.lsText.width;
		m_li.drawLIRuler(hdc,
						 tmp-m_li.lsRuby.space, m_li.paperSpace.top,
						 tmp, body_bottom,
						 2, 0);
		break;
	default:
		m_li.drawOtherFocus(hdc, m_focusID, paper_center, m_li.paperCenterXSpace);
		break;
	}
	//
	::SetBkColor(hdc, oldclr);
	::SelectObject(hdc, hOldBrush);
	::SelectObject(hdc, hOldPen  );
	::DeleteObject(hFocusPen);
	::DeleteObject(hDotPen  );
	::DeleteObject(hPen     );

	::BitBlt(lpDrawItem->hDC, 0, 0, m_bmpWork.Width(), m_bmpWork.Height(), hdc, 0, 0, SRCCOPY);
	::SelectObject(hdc, hbmp);
	::DeleteDC(hdc);
}

std::tuple<int, int> CGrCustomLayoutSettingDlg::errorCheck()
{
	m_error_id_list.clear();
	ErrorCheck ec(m_error_id_list);
	ec.Check(IDC_EDIT_LINES       , m_li.lines        <= 1                      ); /* 行数   *//* パーサー上 1以下の場合、無限ループになるので */
	ec.Check(IDC_EDIT_CHARACTERS  , m_li.characters   <  1                      ); /* 文字数 */
	ec.Check(IDC_EDIT_TEXT        , m_li.lsText.width <= 0                      ); /* 文字幅 */
	ec.Check(IDC_EDIT_PAGE_INSIDE , m_li.paperCenterXSpace > m_li.paperSize.cx  ); /* のど   */
	ec.Check(IDC_EDIT_PAGE_OUTSIDE, m_li.paperSpace.left > m_li.paperSize.cx / 2); /* 小口   */
	ec.Check(IDC_EDIT_CHARACTERS  , m_li.lsText.width < m_li.lsText.space       , IDS_ERROR_LINES_NUM); /* 文字数 *//* 文字幅より行間が大きい場合はエラー */
	return {ec.ctl_id, ec.mes_id};
}

void CGrCustomLayoutSettingDlg::setInitialData()
{
	// 105×148 文庫
	const auto &tllText = m_layout.GetConstLayoutList(CGrTxtLayout::LLT_Text);
	int tllText_size = tllText.size();
	if(tllText_size < 2 || (tllText_size % 2) != 0){
		m_layout.SetInitialize();
		tllText_size = tllText.size();
		if(tllText_size <= 0){ // ありえない
			return;
		}
	}
	int paperCenterYNum = tllText_size / 2;
	m_li.paperSize = m_layout.GetPaperSize();
	int top    = -1;
	int left   = -1;
	int right  = -1;
	int bottom = -1;
	for(const auto &tl : tllText){
		if(top < 0 || top > tl.top){
			top = tl.top;
		}
		if(left < 0 || left > tl.left){
			left = tl.left;
		}
		if(right < 0 || right > tl.right){
			right = tl.right;
		}
		if(bottom < 0 || bottom < tl.bottom){
			bottom = tl.bottom;
		}
	}
	m_li.paperSpace.left   = left;
	m_li.paperSpace.top    = top;
	m_li.paperSpace.right  = left;
	m_li.paperSpace.bottom = m_li.paperSize.cy - bottom;
	int height = tllText[0].bottom - tllText[0].top;
	m_li.paperCenterXSpace = m_li.paperSize.cx/2 - right;
	m_li.lines             = m_layout.GetMaxLine() / tllText.size();
	m_li.characters        = m_layout.GetMaxCharacters();
	m_li.paperCenterYNum   = paperCenterYNum;
	if(paperCenterYNum > 1){
		m_li.paperCenterYSpace = ((bottom - top) - (height * paperCenterYNum)) / (paperCenterYNum-1);
	} else {
		m_li.paperCenterYSpace = 0;
	}
	//
	m_li.setInitialData(m_hWnd, m_layout);
	//
	SetDlgItemInt(m_hWnd, IDC_EDIT_PAGE_INSIDE  , m_li.paperCenterXSpace, TRUE);
	SetDlgItemInt(m_hWnd, IDC_EDIT_PAGE_DIV_SIZE, m_li.paperCenterYSpace, TRUE);
	SetDlgItemInt(m_hWnd, IDC_EDIT_PAGE_DIV_NUM , m_li.paperCenterYNum  , TRUE);

	m_li.refresh(*this);
}

void CGrCustomLayoutSettingDlg::getLayoutName(std::tstring &name)
{
	m_layout.GetLayoutName(name);
	if(name.size() == 0){
		name = _T("ユーザー");
		if(m_li.paperCenterYNum <= 1){
			m_li.paperCenterYNum = 1;
			if(m_li.paperSize.cx == 21000 && m_li.paperSize.cy == 14800){
				name = _T("文庫本(ユーザー)");
			} else if(m_li.paperSize.cx == 20600 && m_li.paperSize.cy == 18200){
				name = _T("新書(ユーザー)");
			}
		} else if(m_li.paperCenterYNum >= 2){
			if(m_li.paperSize.cx == 21000 && m_li.paperSize.cy == 14800){
				CGrText::FormatMessage(name, _T("文庫本縦%1!d!段(ユーザー)"), m_li.paperCenterYNum);
			} else if(m_li.paperSize.cx == 20600 && m_li.paperSize.cy == 18200){
				CGrText::FormatMessage(name, _T("新書縦%1!d!段(ユーザー)"), m_li.paperCenterYNum);
			}
		}
	}
}

BOOL CGrCustomLayoutSettingDlg::OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	SetWindowPosCenter();

	auto hIcon = static_cast<HICON>(LoadImage(CGrTxtFunc::GetDllModuleHandle(), MAKEINTRESOURCE(IDI_APP), IMAGE_ICON, 32, 32, LR_SHARED));
	SendMessage(hwnd, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(hIcon));
	auto hIconSm = static_cast<HICON>(LoadImage(CGrTxtFunc::GetDllModuleHandle(),MAKEINTRESOURCE(IDI_APP),IMAGE_ICON,GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),LR_DEFAULTCOLOR));
	SendMessage(hwnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(hIconSm));
	SendMessage(GetDlgItem(m_hWnd, IDC_CHECK_AUTOCALC) , BM_SETCHECK , BST_CHECKED , 0);
	::EnableDlgItemID(m_hWnd,  IDC_EDIT_TEXT         , FALSE);
	::EnableDlgItemID(m_hWnd,  IDC_EDIT_TEXT_SPACE   , FALSE);
	::EnableDlgItemID(m_hWnd,  IDC_EDIT_RUBY         , FALSE);
	::EnableDlgItemID(m_hWnd,  IDC_EDIT_RUBY_SPACE   , FALSE);
	::EnableDlgItemID(m_hWnd,  IDC_EDIT_NOTE         , FALSE);
	::EnableDlgItemID(m_hWnd,  IDC_EDIT_NOTE_SPACE   , FALSE);
	::EnableDlgItemID(m_hWnd,  IDC_EDIT_NOMBRE       , FALSE);
	::EnableDlgItemID(m_hWnd,  IDC_EDIT_RUNNING_HEADS, FALSE);
	::EnableDlgItemID(m_hWnd,  IDCALC                , FALSE);
	//
	openLayout(m_layout, m_type.c_str(), m_filename.c_str());
	//
	::setImageWindowPos(m_hWnd);
	RECT win_rect;
	::GetWindowRect(m_hWnd, &win_rect);
	m_minSize.cx = win_rect.right - win_rect.left;
	m_minSize.cy = win_rect.bottom - win_rect.top;

	setInitialData();
	{
		std::tstring name;
		getLayoutName(name);
		SetDlgItemText(hwnd, IDC_EDIT_TITLE, name.c_str());
	}

	return TRUE;
}

void CGrCustomLayoutSettingDlg::setTxtLayout()
{
	m_li.setLayoutData(m_layout);
	{
		TCHAR buf[2048];
		GetDlgItemText(m_hWnd, IDC_EDIT_TITLE, buf, sizeof(buf)/sizeof(TCHAR));
		m_layout.SetLayoutName(buf);
	}

	LPCTSTR lpName = _T("Custom");
	m_layout.SetOpenLayoutType(_T("CUSTOM 2.0"));
	if(m_li.paperCenterYNum <= 1){
		m_li.paperCenterYNum = 1;
		if(m_li.paperSize.cx == 21000 && m_li.paperSize.cy == 14800){
			lpName = _T("Bunko");
		}
	} else if(m_li.paperCenterYNum == 2){
		if(m_li.paperSize.cx == 20600 && m_li.paperSize.cy == 18200){
			lpName = _T("ShinshoTate2");
		}
	}
	int paper_center = m_li.paperSize.cx / 2;
	int step = (m_li.paperSize.cy-m_li.paperSpace.top-m_li.paperSpace.bottom+m_li.paperCenterYSpace) / m_li.paperCenterYNum;
	int height = step - m_li.paperCenterYSpace;
	TxtMiru::TxtLayout layout;
	layout.lines      = m_li.lines;
	layout.characters = m_li.characters;
	{
		layout.left       = paper_center + m_li.paperCenterXSpace;
		layout.right      = m_li.paperSize.cx - m_li.paperSpace.right;
		layout.top        = m_li.paperSpace.top;
		for(int i=0; i<m_li.paperCenterYNum; ++i){
			layout.bottom = layout.top + height;
			m_layout.AddLayout(CGrTxtLayout::LLT_Text, layout);
			layout.top += step;
		}
	}
	{
		layout.left       = m_li.paperSpace.left;
		layout.right      = paper_center - m_li.paperCenterXSpace;;
		layout.top        = m_li.paperSpace.top;
		for(int i=0; i<m_li.paperCenterYNum; ++i){
			layout.bottom = layout.top + height;
			m_layout.AddLayout(CGrTxtLayout::LLT_Text, layout);
			layout.top += step;
		}
	}
	int i;
	for(i=0; i<2; ++i){
		layout.left   = m_li.paperNombrePos[i].x;
		layout.top    = m_li.paperNombrePos[i].y;
		layout.right  = layout.left + m_li.lsNombre.width * 3;
		layout.bottom = layout.top  + m_li.lsNombre.width;
		m_layout.AddLayout(CGrTxtLayout::LLT_Nombre, layout);
	}
	layout.left   = m_li.paperRunningHeadsPos.x;
	layout.top    = m_li.paperRunningHeadsPos.y;
	layout.right  = paper_center;
	layout.bottom = layout.top + m_li.lsRunningHeads.width;
	if(layout.right < layout.left){
		int tmp = layout.right;
		layout.right = layout.left;
		layout.left  = tmp;
	}
	m_layout.AddLayout(CGrTxtLayout::LLT_RunningHeads, layout);
	layout.left       = 0;
	layout.top        = m_li.paperSpace.top;
	layout.right      = m_li.paperSpace.right;
	layout.bottom     = m_li.paperSize.cy - m_li.paperSpace.bottom;
	layout.characters = (layout.bottom - layout.top) / max(m_li.lsNote.width, 1);
	layout.lines      = (layout.right - layout.left) / max(m_li.lsNote.width+m_li.lsNote.space, 1);
	m_layout.AddLayout(CGrTxtLayout::LLT_Note, layout);

	saveUpdate(m_hWnd, m_layout, lpName);
}

void CGrCustomLayoutSettingDlg::Calc(int id)
{
	switch(id){
	case IDC_EDIT_TEXT         :
	case IDC_EDIT_TEXT_SPACE   :
	case IDC_EDIT_RUBY         :
	case IDC_EDIT_RUBY_SPACE   :
	case IDC_EDIT_NOTE         :
	case IDC_EDIT_NOTE_SPACE   :
	case IDC_EDIT_NOMBRE       :
	case IDC_EDIT_RUNNING_HEADS:
		return;
	}
	int num = m_li.paperCenterYNum;
	if(num <= 0){
		num = 1;
	}
	int line = m_li.lines;
	if(line <= 1){
		line = 2;
	}
	int characters = m_li.characters;
	if(characters <= 1){
		characters = 2;
	}
	int width = m_li.paperSize.cx / 2 - m_li.paperCenterXSpace - m_li.paperSpace.right;
	int height = ((m_li.paperSize.cy - m_li.paperSpace.top - m_li.paperSpace.bottom) - (m_li.paperCenterYSpace * (num - 1))) / num;
	int text_width_w = 30 * width / (49 * line - 19);
	int text_width_h = height / characters;
	int text_width = min(text_width_w, text_width_h);
	int ruby_width = text_width / 2;
	int ruby_space = text_width / 15;
	int text_space = text_width / 15;
	if(text_width_h < text_width_w){
		text_space = (width - (text_width * line) - (ruby_width + ruby_space) * (line-1)) / (line-1);
	}
	int note_space = ruby_space;
	int note_width = min(m_li.paperSpace.left / 3 - note_space, (text_width+ruby_width)/2);
	//
	int nombre = m_li.paperSpace.top / 3;

	auto hwnd = m_hWnd;
	SetDlgItemInt(hwnd, IDC_EDIT_TEXT         , text_width, TRUE);
	SetDlgItemInt(hwnd, IDC_EDIT_TEXT_SPACE   , text_space, TRUE);
	SetDlgItemInt(hwnd, IDC_EDIT_RUBY         , ruby_width, TRUE);
	SetDlgItemInt(hwnd, IDC_EDIT_RUBY_SPACE   , ruby_space, TRUE);
	SetDlgItemInt(hwnd, IDC_EDIT_NOTE         , note_width, TRUE);
	SetDlgItemInt(hwnd, IDC_EDIT_NOTE_SPACE   , note_space, TRUE);
	SetDlgItemInt(hwnd, IDC_EDIT_NOMBRE       , nombre    , TRUE);
	SetDlgItemInt(hwnd, IDC_EDIT_RUNNING_HEADS, nombre    , TRUE);
}

void CGrCustomLayoutSettingDlg::OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	int error_id, mess_id;
	switch(id){
	case IDPAPER_SIZE:
		{
			CGrPaperSizeDlg dlg;
			if(IDOK == dlg.DoModal(hwnd, m_li.paperSize)){
				auto size = dlg.GetPaperSize();
				if(size.cx == m_li.paperSize.cx && size.cy == m_li.paperSize.cy){
					break;
				}
				m_li.paperSize = size;
				SetDlgItemInt(hwnd, IDC_EDIT_PAPER_WIDTH , m_li.paperSize.cx, TRUE);
				SetDlgItemInt(hwnd, IDC_EDIT_PAPER_HEIGHT, m_li.paperSize.cy, TRUE);
				if(size.cx == 20600 && size.cy == 18200){
					SetDlgItemInt(m_hWnd, IDC_EDIT_LINES         ,   17, TRUE); // 行数
					SetDlgItemInt(m_hWnd, IDC_EDIT_CHARACTERS    ,   40, TRUE); // 文字数
					SetDlgItemInt(m_hWnd, IDC_EDIT_PAGE_INSIDE   , 1000, TRUE); // のど
					SetDlgItemInt(m_hWnd, IDC_EDIT_PAGE_OUTSIDE  ,  500, TRUE); // 小口
					SetDlgItemInt(m_hWnd, IDC_EDIT_PAGE_TOP      , 1000, TRUE); // 天
					SetDlgItemInt(m_hWnd, IDC_EDIT_PAGE_BOTTOM   , 1000, TRUE); // 地
					SetDlgItemInt(m_hWnd, IDC_EDIT_PAGE_CENTER   ,    0, TRUE); // 段間
					SetDlgItemInt(m_hWnd, IDC_EDIT_PAGE_DIV_NUM  ,    1, TRUE); // 段数
				} else if(size.cx == 21000 && size.cy == 14800){
					SetDlgItemInt(m_hWnd, IDC_EDIT_LINES         ,   17, TRUE); // 行数
					SetDlgItemInt(m_hWnd, IDC_EDIT_CHARACTERS    ,   40, TRUE); // 文字数
					SetDlgItemInt(m_hWnd, IDC_EDIT_PAGE_INSIDE   , 1000, TRUE); // のど
					SetDlgItemInt(m_hWnd, IDC_EDIT_PAGE_OUTSIDE  ,  500, TRUE); // 小口
					SetDlgItemInt(m_hWnd, IDC_EDIT_PAGE_TOP      , 1000, TRUE); // 天
					SetDlgItemInt(m_hWnd, IDC_EDIT_PAGE_BOTTOM   , 1000, TRUE); // 地
					SetDlgItemInt(m_hWnd, IDC_EDIT_PAGE_CENTER   ,    0, TRUE); // 段間
					SetDlgItemInt(m_hWnd, IDC_EDIT_PAGE_DIV_NUM  ,    1, TRUE); // 段数
				}
				{
					TCHAR buf[2048];
					GetDlgItemText(m_hWnd, IDC_EDIT_TITLE, buf, sizeof(buf)/sizeof(TCHAR));
					if(lstrlen(buf) == 0){
						std::tstring name;
						getLayoutName(name);
						SetDlgItemText(hwnd, IDC_EDIT_TITLE, name.c_str());
					}
				}
				if(::GetCheckDlgItemID(m_hWnd, IDC_CHECK_AUTOCALC) == BST_CHECKED){
					Calc(0);
				}
			}
		}
		break;
	case IDCALC: Calc(0); break;
	case IDOK:
		{
			std::tie(error_id, mess_id) = errorCheck();
			if(error_id != 0){
				SetFocus(GetDlgItem(m_hWnd, error_id));
				CGrMessageBox::Show(CGrTxtFunc::GetDllModuleHandle(), m_hWnd, mess_id);
			} else {
				{
					std::tstring filename;
					m_layout.GetFileName(filename);
					if(filename.size() == 0 || filename == _T("Layout/Bunko.lay") || filename == _T("Layout/ShinshoTate2.lay")){
						time_t now;
						time(&now);
						struct tm ltm = {};
						if(0 == localtime_s(&ltm, &now)){
							TCHAR date_str[512] = {};
							_tcsftime(date_str, sizeof(date_str)/sizeof(TCHAR), _T("%Y%m%d%H%M%S"), &ltm);
							std::tstring filename;
							CGrText::FormatMessage(filename, _T("%1!s!/Layout/%2!s!.lay"), CGrTxtFunc::GetDataPath(), date_str);
							CGrShell::ToPrettyFileName(filename);
							m_layout.SetFileName(filename.c_str());
						}
					}
				}
				setTxtLayout();
				::EndDialog(m_hWnd, id);
			}
		}
		break;
	case IDSAVEAS:
		{
			std::tie(error_id, mess_id) = errorCheck();
			if(error_id != 0){
				SetFocus(GetDlgItem(m_hWnd, error_id));
				CGrMessageBox::Show(CGrTxtFunc::GetDllModuleHandle(), m_hWnd, mess_id);
			} else {
				time_t now;
				time(&now);
				struct tm ltm = {0};
				if(0 == localtime_s(&ltm, &now)){
					TCHAR date_str[512] = {};
					_tcsftime(date_str, sizeof(date_str)/sizeof(TCHAR), _T("%Y%m%d%H%M%S"), &ltm);
					std::tstring filename;
					CGrText::FormatMessage(filename, _T("%1!s!/Layout/%2!s!.lay"), CGrTxtFunc::GetDataPath(), date_str);
					CGrShell::ToPrettyFileName(filename);
					m_layout.SetFileName(filename.c_str());
				}
				setTxtLayout();
				::EndDialog(m_hWnd, id);
			}
		}
		break;
	case IDAPPLY:
		std::tie(error_id, mess_id) = errorCheck();
		if(error_id != 0){
			SetFocus(GetDlgItem(m_hWnd, error_id));
			CGrMessageBox::Show(CGrTxtFunc::GetDllModuleHandle(), m_hWnd, mess_id);
		} else {
			setTxtLayout();
		}
		break;
	case IDRESET:
		{
			TCHAR buf[512];
			CGrText::LoadString(IDS_RESET_LAYOUT, buf, sizeof(buf)/sizeof(TCHAR));
			if(IDYES == CGrMessageBox::Show(CGrTxtFunc::GetDllModuleHandle(), m_hWnd, buf, CGrTxtFunc::AppName(), MB_YESNO)){
				m_layout.SetInitialize();
				setInitialData();
				Refresh();
			}
		}
		break;
	case IDC_CHECK_AUTOCALC:
		{
			BOOL bEnable = ::GetCheckDlgItemID(m_hWnd, IDC_CHECK_AUTOCALC) == BST_CHECKED ? FALSE : TRUE;
			::EnableDlgItemID(m_hWnd,  IDC_EDIT_TEXT         , bEnable);
			::EnableDlgItemID(m_hWnd,  IDC_EDIT_TEXT_SPACE   , bEnable);
			::EnableDlgItemID(m_hWnd,  IDC_EDIT_RUBY         , bEnable);
			::EnableDlgItemID(m_hWnd,  IDC_EDIT_RUBY_SPACE   , bEnable);
			::EnableDlgItemID(m_hWnd,  IDC_EDIT_NOTE         , bEnable);
			::EnableDlgItemID(m_hWnd,  IDC_EDIT_NOTE_SPACE   , bEnable);
			::EnableDlgItemID(m_hWnd,  IDC_EDIT_NOMBRE       , bEnable);
			::EnableDlgItemID(m_hWnd,  IDC_EDIT_RUNNING_HEADS, bEnable);
			::EnableDlgItemID(m_hWnd,  IDCALC                , bEnable);
		}
		break;
	case IDCANCEL: ::EndDialog(m_hWnd, id); break;
	case IDC_EDIT_LINES             : case IDC_EDIT_CHARACTERS       :
	case IDC_EDIT_TEXT              : case IDC_EDIT_TEXT_SPACE       :
	case IDC_EDIT_RUBY              : case IDC_EDIT_RUBY_SPACE       :
	case IDC_EDIT_NOTE              : case IDC_EDIT_NOTE_SPACE       :
	case IDC_EDIT_PAGE_TOP          : case IDC_EDIT_PAGE_BOTTOM      :
	case IDC_EDIT_PAGE_OUTSIDE      : case IDC_EDIT_PAGE_INSIDE      :
	case IDC_EDIT_PAGE_DIV_SIZE     : case IDC_EDIT_PAGE_DIV_NUM     :
	case IDC_EDIT_NOMBRE            : case IDC_EDIT_RUNNING_HEADS    :
	case IDC_EDIT_NOMBRE1_LEFT      : case IDC_EDIT_NOMBRE1_TOP      :
	case IDC_EDIT_NOMBRE2_LEFT      : case IDC_EDIT_NOMBRE2_TOP      :
	case IDC_EDIT_RUNNING_HEADS_LEFT: case IDC_EDIT_RUNNING_HEADS_TOP:
		switch(codeNotify){
		case EN_SETFOCUS:
			m_focusID = id;
			::InvalidateRect(GetDlgItem(m_hWnd, IDC_LAYOUT_IMG),NULL,FALSE);
			break;
		case EN_CHANGE:
			updateLayout(m_hWnd);
			break;
		case EN_KILLFOCUS:
			Refresh();
			break;
		}
		break;
	case IDC_RADIO_NOMBRE_OUT: case IDC_RADIO_NOMBRE_CENTER: case IDC_RADIO_NOMBRE_IN:
		updateLayout(m_hWnd);
		break;
	}
}
int CGrCustomTxtLayout::ConfigurationDlg(HWND hWnd, LPCTSTR type, LPCTSTR name, LPCTSTR lpFileName)
{
	CGrCustomLayoutSettingDlg dlg(type, name, lpFileName);
	dlg.DoModal(hWnd);
	return 0;
}
/////
