//magnifying glass
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "resource.h"
#include "LupeWnd.h"
#include "TxtMiru.h"

// ホイールマウス対応
#ifndef WM_MOUSEWHEEL
#include "zmouse.h"
#endif

#define ID_TIMER_LUPE_ZOOM 100

CGrLupeWnd::CGrLupeWnd() : m_fromCanvasRect()
{
	m_offset.x = -100;
	m_offset.y = -100;
}

CGrLupeWnd::~CGrLupeWnd()
{
}

// ウィンドウ・クラスの登録
BOOL CGrLupeWnd::InitApp(HINSTANCE hInst, WNDPROC fnWndProc, LPCTSTR szClassNm)
{
	WNDCLASSEX wc = {sizeof(WNDCLASSEX)};

	wc.hCursor       = LoadCursor(NULL, IDI_APPLICATION);
	wc.lpszClassName = szClassNm;
	wc.hInstance     = hInst;
	wc.lpfnWndProc   = fnWndProc;

	return RegisterClassEx(&wc);
}

#define CGRTYPEID _T("CGrLupeWnd")
void CGrLupeWnd::Show(HWND hWnd, int page)
{
	if(!m_bHide){
		return;
	}
	if(!m_hWnd){
		auto hInst = GetWindowInstance(hWnd);
		InitApp(hInst, CGrWinCtrl::WindowMapProc, CGRTYPEID);
		m_hWnd = CreateWindowEx(
			WS_EX_TOOLWINDOW,
			CGRTYPEID,
			NULL,
			WS_POPUP | WS_THICKFRAME,
			200, 200, 100, 100,
			hWnd,
			NULL,
			hInst,
			this);
		const auto &param = CGrTxtMiru::theApp().Param();
		int window_pos[5] = {};
		param.GetPoints(CGrTxtParam::PointsType::LupePos, window_pos, sizeof(window_pos)/sizeof(int));
		// モニタ内でウインドウを移動
		MoveWindowInMonitor(window_pos[1], window_pos[2], window_pos[3], window_pos[4]);
	}
	if(!m_pTxtCanvas){
		m_pTxtCanvas = new CGrTxtCanvas();
		if(!m_pTxtCanvas){
			return;
		}
		m_pTxtCanvas->Attach(m_pDoc);
	}
	m_bHide = false;
	ShowWindow(m_hWnd, SW_SHOW);
	if(m_lupeSize < 0){
		m_lupeSize = 200;
	}
	m_pTxtCanvas->Initialize();
	m_pTxtCanvas->UpdateParam();
	Zoom(m_lupeSize);
	m_pTxtCanvas->Update(page); // リサイズでフォントサイズが設定されていないと ページ設定ができない(Zoomの後で実行する必要がある)
	::SetTimer(m_hWnd, ID_TIMER_LUPE_ZOOM, 100, nullptr);
}

void CGrLupeWnd::Attach(CGrTxtDocument *pDoc)
{
	m_pDoc = pDoc;
}

void CGrLupeWnd::Update(int page)
{
	if(!m_bHide && m_pTxtCanvas){
		m_pTxtCanvas->Update(page);
		m_pTxtCanvas->ClearCache();
		UpdateWindowPos();
		::InvalidateRect(m_hWnd, NULL, FALSE);
		::UpdateWindow(m_hWnd);
	}
}

void CGrLupeWnd::SetCanvasOffset(int x, int y)
{
	m_windowOffsetX = x;
	m_windowOffsetY = y;
}
void CGrLupeWnd::SetFromCanvasRect(const RECT &fromCanvasRect)
{
	m_fromCanvasRect = fromCanvasRect;
	POINT pos = {0};
	GetCursorPos(&pos);
	ScreenToClient(GetParent(m_hWnd), &pos);
	SetOffset(pos);
	int z = m_canvasLupeSize;
	m_canvasLupeSize = -1;
	if(z == -1){
		z = m_lupeSize;
	}
	Zoom(z);
}

void CGrLupeWnd::UpdateParam()
{
	if(m_pTxtCanvas){
		m_pTxtCanvas->UpdateParam();
	}
}

void CGrLupeWnd::Hide()
{
	if(m_bHide){
		return;
	}
	m_bHide = true;
	if(m_hWnd){
		ShowWindow(m_hWnd, SW_HIDE);
	}
	if(m_pTxtCanvas){
		delete m_pTxtCanvas;
		m_pTxtCanvas = nullptr;
	}
	m_canvasLupeSize = -1;
}

void CGrLupeWnd::Zoom(int izoom)
{
	if(izoom < 100){
		izoom = 100;
	} else if(izoom > 600){
		izoom = 600;
	}
	if(izoom != m_canvasLupeSize){
		m_lupeSize = izoom;
		if(m_bHide || !m_pTxtCanvas){
			return;
		}
		m_canvasLupeSize = izoom;
		const auto &layout = m_pDoc->GetConstLayout();
		const auto &paperSize = layout.GetPaperSize();
		int w = (m_fromCanvasRect.right - m_fromCanvasRect.left) * m_canvasLupeSize / 100;
		int h = w * paperSize.cy / paperSize.cx;
		m_pTxtCanvas->SetOffset(0, 0);
		m_pTxtCanvas->Resize(w, h);
		::SendMessage(GetParent(m_hWnd), WM_COMMAND, TxtDocMessage::UPDATE_CMD_UI, 0);
		::SetTimer(m_hWnd, ID_TIMER_LUPE_ZOOM, 100, NULL);
	}
}

int CGrLupeWnd::GetZoom() const
{
	return m_lupeSize;
}

LRESULT CGrLupeWnd::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg){
		HANDLE_MSG(hWnd, WM_INITDIALOG   , OnInitDialog   );
		HANDLE_MSG(hWnd, WM_SIZE         , OnSize         );
		HANDLE_MSG(hWnd, WM_PAINT        , OnPaint        );
		HANDLE_MSG(hWnd, WM_NCMOUSEMOVE  , OnNCMouseMove  );
		HANDLE_MSG(hWnd, WM_MOVE         , OnMove         );
	case WM_TIMER:
		if(ID_TIMER_LUPE_ZOOM == wParam){
			KillTimer(hWnd, ID_TIMER_LUPE_ZOOM);
			Update();
		}
		break;
	case WM_KEYDOWN:
		SendMessage(GetParent(hWnd), uMsg, wParam, lParam);
		break;
	case WM_MOUSEWHEEL:
		{
			int delta = GET_WHEEL_DELTA_WPARAM(wParam); // マルチモニタ環境では、負の値を持つことがあるので一旦 shortに変換する必要がある
			if((WHEEL_DELTA/6) <= delta){
				Zoom(m_lupeSize - 50);
			} else if(delta <= -(WHEEL_DELTA/6)){
				Zoom(m_lupeSize + 50);
			}
		}
		break;
	case WM_ERASEBKGND:
		return TRUE;
	case WM_NCHITTEST:
		{
			LRESULT result = DefWindowProc(hWnd, uMsg, wParam, lParam);
			switch(result){
			case HTTOPLEFT    : /*               */ break;
			case HTBOTTOMLEFT : /*               */ break;
			case HTTOPRIGHT   : /*               */ break;
			case HTBOTTOMRIGHT: /*               */ break;
			case HTLEFT       : /*               */ break;
			case HTRIGHT      : /*               */ break;
			case HTTOP        : /*               */ break;
			case HTBOTTOM     : /*               */ break;
			case HTCLIENT     : result = HTCAPTION; break;
			}
			return result;
		}
		break;
	case WM_DESTROY:
		{
			auto &&param = CGrTxtMiru::theApp().Param();
			if(!param.GetBoolean(CGrTxtParam::PointsType::SaveWindowSize)){
				break;
			}
			RECT rect;
			int window_pos[5] = {};
			::GetWindowRect(m_hWnd, &rect);
			param.GetPoints(CGrTxtParam::PointsType::LupePos, window_pos, sizeof(window_pos)/sizeof(int));
			window_pos[1] = rect.left   ;
			window_pos[2] = rect.top    ;
			window_pos[3] = rect.right  - rect.left;
			window_pos[4] = rect.bottom - rect.top;
			param.SetPoints(CGrTxtParam::PointsType::LupePos, window_pos, sizeof(window_pos)/sizeof(int));
			if(m_pTxtCanvas){
				delete m_pTxtCanvas;
				m_pTxtCanvas = nullptr;
			}
		}
	}
	return CGrWinCtrl::WndProc(hWnd, uMsg, wParam, lParam);
}

BOOL CGrLupeWnd::OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	return TRUE;
}

void CGrLupeWnd::OnPaint(HWND hwnd)
{
	PAINTSTRUCT lpps;
	auto hdc = ::BeginPaint(hwnd, &lpps);

	RECT rect = {0};
	::GetClientRect(hwnd, &rect);

	const auto &param = CGrTxtMiru::theApp().Param();
	COLORREF page_color[3] = {}; /* paper, shadow, back */
	param.GetPoints(CGrTxtParam::PointsType::PageColor, (int*)page_color, sizeof(page_color)/sizeof(COLORREF));
	auto oldclr = ::SetBkColor(hdc, page_color[2]);
	auto temp_rect = rect;
	temp_rect.right = -m_offset.x;
	::ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &temp_rect, NULL, 0, NULL);
	//
	if(m_pTxtCanvas){
		const auto &canvas_rect = m_pTxtCanvas->GetCanvasRect();
		temp_rect.left = canvas_rect.right - m_offset.x;
		temp_rect.right = rect.right;
		::ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &temp_rect, NULL, 0, NULL);
		temp_rect = rect;
		temp_rect.bottom = -m_offset.y;
		::ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &temp_rect, NULL, 0, NULL);
		temp_rect.top = canvas_rect.bottom - m_offset.y;
		temp_rect.bottom = rect.bottom;
		::ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &temp_rect, NULL, 0, NULL);
		::SetBkColor(hdc, oldclr);
		m_pTxtCanvas->Draw(hdc, rect, lpps.rcPaint, m_offset.x, m_offset.y);
	}

	::EndPaint(hwnd, &lpps);
}

void CGrLupeWnd::OnSize(HWND hwnd, UINT state, int cx, int cy)
{
	if(state == SIZE_MINIMIZED || state == SIZE_MAXHIDE){
		return;
	}
	setWindowSize(cx, cy);
}

void CGrLupeWnd::setWindowSize(int cx, int cy)
{
	if(cx <= 0 || cy <= 0){
		return;
	}
	::InvalidateRect(m_hWnd, NULL, FALSE);
	::UpdateWindow(m_hWnd);
}

void CGrLupeWnd::UpdatePos(POINT pos)
{
	if(m_selectType == SelectType::MousePos){
		SetOffset(pos);
	}
}

void CGrLupeWnd::UpdateWindowPos()
{
	if(m_selectType == SelectType::WindowPos){
		RECT winRect;
		GetWindowRect(m_hWnd, &winRect);
		POINT pos = {
			winRect.left + (winRect.right - winRect.left) / 2,
			winRect.top  + (winRect.bottom - winRect.top) / 2
		};
		ScreenToClient(GetParent(m_hWnd), &pos);
		SetOffset(pos);
	}
}

void CGrLupeWnd::SetOffset(POINT pos)
{
	m_offset.x = 0;
	m_offset.y = 0;
	if(!m_bHide && m_pTxtCanvas){
		//
		const auto &from_canvas_rect = m_fromCanvasRect;
		int from_w = (from_canvas_rect.right - from_canvas_rect.left);
		int from_h = (from_canvas_rect.bottom - from_canvas_rect.top);
		if(from_w <= 0 || from_h <= 0){
			return;
		}
		const auto &to_canvas_rect = m_pTxtCanvas->GetCanvasRect();
		int to_w = (to_canvas_rect.right - to_canvas_rect.left);
		int to_h = (to_canvas_rect.bottom - to_canvas_rect.top);
		if(from_w <= 0 || from_h <= 0){
			return;
		}
		RECT client_rect = {0};
		GetClientRect(m_hWnd, &client_rect);
		int w = client_rect.right - client_rect.left;
		int h = client_rect.bottom - client_rect.top;
		if(w <= 0 || h <= 0){
			return;
		}
		m_offset.x = (pos.x + m_windowOffsetX - from_canvas_rect.left) * to_w / from_w - to_canvas_rect.left - w / 2;
		m_offset.y = (pos.y + m_windowOffsetY - from_canvas_rect.top ) * to_h / from_h - to_canvas_rect.top  - h / 2;
		InvalidateRect(m_hWnd, NULL, FALSE);
	}
}

void CGrLupeWnd::OnNCMouseMove(HWND hwnd, int x, int y, UINT codeHitTest)
{
	if(m_selectType == SelectType::MousePos){
		POINT pos = {x, y};
		ScreenToClient(GetParent(m_hWnd), &pos);
		SetOffset(pos);
	}
}

void CGrLupeWnd::SetSelectType(SelectType st)
{
	if(m_selectType != st){
		m_selectType = st;
	}
}

void CGrLupeWnd::OnMove(HWND hwnd, int x, int y)
{
	UpdateWindowPos();
}
