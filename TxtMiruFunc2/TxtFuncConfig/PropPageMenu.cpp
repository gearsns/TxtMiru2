#pragma warning( disable : 4786 )

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "resource.h"
#include "PropPageMenu.h"
#include "TxtConfigDlg.h"
#include "Text.h"
#include "CSVText.h"
#include "Shell.h"
#include "CurrentDirectory.h"
#include "TxtFuncConfig.h"
#include "TxtFunc.h"
#include "TxtConfigFunc.h"
#include "MessageBox.h"

static std::map<TxtMiru::FuncNameID,UINT> l_funcNameMap =
{
	{TxtMiru::FuncNameID::ReadFile            ,IDS_READFILE            },
	{TxtMiru::FuncNameID::URLOpen             ,IDS_URLOPEN             },
	{TxtMiru::FuncNameID::OpenFolder          ,IDS_OPENFOLDER          },
	{TxtMiru::FuncNameID::OpenBrowser         ,IDS_OPENBROWSER         },
	{TxtMiru::FuncNameID::ReadClipboard       ,IDS_READCLIPBOAD        },
	{TxtMiru::FuncNameID::Reload              ,IDS_RELOAD              },
	{TxtMiru::FuncNameID::NextPage            ,IDS_NEXTPAGE            },
	{TxtMiru::FuncNameID::PrevPage            ,IDS_PREVPAGE            },
	{TxtMiru::FuncNameID::FirstPage           ,IDS_FIRSTPAGE           },
	{TxtMiru::FuncNameID::EndPage             ,IDS_ENDPAGE             },
	{TxtMiru::FuncNameID::GotoPage            ,IDS_GOTOPAGE            },
	{TxtMiru::FuncNameID::NextFile            ,IDS_NEXTFILE            },
	{TxtMiru::FuncNameID::PrevFile            ,IDS_PREVFILE            },
	{TxtMiru::FuncNameID::ForwardPage         ,IDS_FORWARDPAGE         },
	{TxtMiru::FuncNameID::BackPage            ,IDS_BACKPAGE            },
	{TxtMiru::FuncNameID::AddBookmark         ,IDS_ADDBOOKMARK         },
	{TxtMiru::FuncNameID::GotoBookmark1       ,IDS_GOTOBOOKMARK1       },
	{TxtMiru::FuncNameID::GotoBookmark2       ,IDS_GOTOBOOKMARK2       },
	{TxtMiru::FuncNameID::GotoBookmark3       ,IDS_GOTOBOOKMARK3       },
	{TxtMiru::FuncNameID::GotoBookmark4       ,IDS_GOTOBOOKMARK4       },
	{TxtMiru::FuncNameID::GotoBookmark5       ,IDS_GOTOBOOKMARK5       },
	{TxtMiru::FuncNameID::GotoBookmark6       ,IDS_GOTOBOOKMARK6       },
	{TxtMiru::FuncNameID::GotoBookmark7       ,IDS_GOTOBOOKMARK7       },
	{TxtMiru::FuncNameID::GotoBookmark8       ,IDS_GOTOBOOKMARK8       },
	{TxtMiru::FuncNameID::GotoBookmark9       ,IDS_GOTOBOOKMARK9       },
	{TxtMiru::FuncNameID::GotoBookmark0       ,IDS_GOTOBOOKMARK0       },
	{TxtMiru::FuncNameID::AddFavorite         ,IDS_ADDFAVORITE         },
	{TxtMiru::FuncNameID::ShowBookmark        ,IDS_SHOWBOOKMARK        },
	{TxtMiru::FuncNameID::ShowHScrollBar      ,IDS_SHOW_HSCROLLBAR     },
	{TxtMiru::FuncNameID::ShowVScrollBar      ,IDS_SHOW_VSCROLLBAR     },
	{TxtMiru::FuncNameID::HideHScrollBar      ,IDS_HIDE_HSCROLLBAR     },
	{TxtMiru::FuncNameID::HideVScrollBar      ,IDS_HIDE_VSCROLLBAR     },
	{TxtMiru::FuncNameID::ToggleHScrollBar    ,IDS_TOGGLE_HSCROLLBAR   },
	{TxtMiru::FuncNameID::ToggleVScrollBar    ,IDS_TOGGLE_VSCROLLBAR   },
	{TxtMiru::FuncNameID::FullScreen          ,IDS_FULLSCREEN          },
	{TxtMiru::FuncNameID::ShowSubtitle        ,IDS_SHOWSUBTITLE        },
	{TxtMiru::FuncNameID::ShowSubtitleBookmark,IDS_SHOWSUBTITLEBOOKMARK},
	{TxtMiru::FuncNameID::ShowBookList        ,IDS_FAVORITE            },
	{TxtMiru::FuncNameID::ShowAozoraList      ,IDS_BMAOZORA            },
	{TxtMiru::FuncNameID::ShowRubyList        ,IDS_SHOWRUBYLIST        },
	{TxtMiru::FuncNameID::ShowProperty        ,IDS_SHOWPROPERTY        },
	{TxtMiru::FuncNameID::ShowDocInfo         ,IDS_SHOWDOCINFO         },
	{TxtMiru::FuncNameID::Copy                ,IDS_COPY                },
	{TxtMiru::FuncNameID::Search              ,IDS_SEARCH              },
	{TxtMiru::FuncNameID::SearchNext          ,IDS_SEARCHNEXT          },
	{TxtMiru::FuncNameID::SearchPrev          ,IDS_SEARCHPREV          },
	{TxtMiru::FuncNameID::SearchFiles         ,IDS_SEARCHFILES         },
	{TxtMiru::FuncNameID::Config              ,IDS_CONFIG              },
	{TxtMiru::FuncNameID::Help                ,IDS_SHOW_HELP           },
	{TxtMiru::FuncNameID::Version             ,IDS_SHOW_VERSION        },
	{TxtMiru::FuncNameID::LayoutSet           ,IDS_LAYOUTSET           },
	{TxtMiru::FuncNameID::ToggleLupe          ,IDS_TOGGLE_LUPE         },
	{TxtMiru::FuncNameID::ShowLupe            ,IDS_SHOW_LUPE           },
	{TxtMiru::FuncNameID::HideLupe            ,IDS_HIDE_LUPE           },
	{TxtMiru::FuncNameID::SetLupeZoom100      ,IDS_LUPE100             },
	{TxtMiru::FuncNameID::SetLupeZoom150      ,IDS_LUPE150             },
	{TxtMiru::FuncNameID::SetLupeZoom200      ,IDS_LUPE200             },
	{TxtMiru::FuncNameID::SetLupeZoom400      ,IDS_LUPE400             },
	{TxtMiru::FuncNameID::RefreshPreParserList,IDS_REFRESHPREPARSERLIST},
	{TxtMiru::FuncNameID::ToggleCopyRuby      ,IDS_TOGGLECOPYRUBY      },
	{TxtMiru::FuncNameID::ExecOpenFiile       ,IDS_OPENFILEEXE         },
	{TxtMiru::FuncNameID::ExecOpenFiile1      ,IDS_OPENFILEEXE1        },
	{TxtMiru::FuncNameID::ExecOpenFiile2      ,IDS_OPENFILEEXE2        },
	{TxtMiru::FuncNameID::LinkGoto            ,IDS_LINKGOTO            },
	{TxtMiru::FuncNameID::LinkOpen            ,IDS_LINKOPEN            },
	{TxtMiru::FuncNameID::ToggleSelectionMode ,IDS_SELECTIONMODE       },
	{TxtMiru::FuncNameID::ShowContextMenu     ,IDS_SHOWCONTEXTMENU     },
	{TxtMiru::FuncNameID::MaxNum              ,IDS_CANCEL              },
	{TxtMiru::FuncNameID::Nop                 ,IDS_SEPARATOR           },
	{TxtMiru::FuncNameID::Exit                ,IDS_EXIT                },
};

CGrPropPage* PASCAL CGrPropPageMenu::CreateProp(CGrTxtConfigDlg *pDlg)
{
	auto *pPage = new CGrPropPageMenu(pDlg);
	pPage->m_hWnd = CreateDialogParam(CGrTxtFunc::GetDllModuleHandle(), MAKEINTRESOURCE(IDD_PROPPAGE_MENU), pDlg->GetWnd(), reinterpret_cast<DLGPROC>(CGrWinCtrl::WindowMapProc), reinterpret_cast<LPARAM>(pPage));
	pPage->Attach(pPage->m_hWnd);
	return pPage;
}

CGrPropPageMenu::CGrPropPageMenu(CGrTxtConfigDlg *pDlg) : CGrPropPageSize(pDlg)
{
}

CGrPropPageMenu::~CGrPropPageMenu()
{
}

// メインウィンドウ
LRESULT CGrPropPageMenu::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg){
		HANDLE_MSG(hWnd, WM_INITDIALOG, OnInitDialog );
		HANDLE_MSG(hWnd, WM_NOTIFY    , OnNotify     );
		HANDLE_MSG(hWnd, WM_COMMAND   , OnCommand    );
		HANDLE_MSG(hWnd, WM_SIZE      , OnSize       );
	case WM_SETPAGEFOCUS:
		::SetFocus(reinterpret_cast<HWND>(wParam));
		break;
	}
	return CGrPropPageSize::WndProc(hWnd, uMsg, wParam, lParam);
}

BOOL CGrPropPageMenu::OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	CGrPropPageSize::OnInitDialog(hwnd, hwndFocus, lParam);
	auto &&param = CGrTxtFunc::Param();
	m_listMenu.Attach(GetDlgItem(hwnd, IDC_LIST_MENU));
	m_listFunc.Attach(GetDlgItem(hwnd, IDC_LIST_FUNC));
	//
	std::tstring filename = _T("menu.lis");
	LPCTSTR lpDataPath = CGrTxtFunc::GetDataPath();
	if(lpDataPath){
		CGrShell::ToPrettyFileName(lpDataPath, filename);
		filename += _T("/menu.lis");
	}
	std::vector<TxtMiru::FuncNameID> fnid_list;
	fnid_list.clear();
	CGrCSVText csv;
	if(csv.Open(filename.c_str())){
		for(const auto &item : csv.GetCSVROW()){
			if (!item.empty()) {
				const auto& str = item[0];
				for (int i = 0; i < static_cast<int>(TxtMiru::FuncNameID::MaxNum); ++i) {
					if (lstrcmp(TxtMiru::l_TxtMiruFuncNameList[i], str.c_str()) == 0) {
						fnid_list.push_back(static_cast<TxtMiru::FuncNameID>(i));
						break;
					}
				}
			}
		}
	}
	if(fnid_list.empty()){
		fnid_list = {
			TxtMiru::FuncNameID::Copy, TxtMiru::FuncNameID::ToggleSelectionMode, TxtMiru::FuncNameID::Nop,
			TxtMiru::FuncNameID::PrevPage, TxtMiru::FuncNameID::NextPage, TxtMiru::FuncNameID::Nop,
			TxtMiru::FuncNameID::ShowSubtitleBookmark, TxtMiru::FuncNameID::AddBookmark, TxtMiru::FuncNameID::Nop,
			TxtMiru::FuncNameID::ShowBookList, TxtMiru::FuncNameID::Nop,
			TxtMiru::FuncNameID::ShowDocInfo, TxtMiru::FuncNameID::ShowProperty, TxtMiru::FuncNameID::Nop,
			TxtMiru::FuncNameID::ExecOpenFiile, TxtMiru::FuncNameID::FullScreen
			};
	}
	fnid_list.push_back(TxtMiru::FuncNameID::Nop);
	for(const auto &id: fnid_list){
		const auto &item = l_funcNameMap.find(static_cast<TxtMiru::FuncNameID>(id));
		if(l_funcNameMap.end() != item){
			std::tstring str;
			CGrText::LoadString(item->second, str);
			int i = m_listMenu.AddString(str.c_str());
			m_listMenu.SetItemData(i, static_cast<LPARAM>(id));
		}
	}
	TxtMiru::FuncNameID func_list[] = {
		TxtMiru::FuncNameID::Nop                 ,
		TxtMiru::FuncNameID::ReadFile            ,
		TxtMiru::FuncNameID::URLOpen             ,
		TxtMiru::FuncNameID::OpenFolder          ,
		TxtMiru::FuncNameID::ReadClipboard       ,
		TxtMiru::FuncNameID::Reload              ,
		TxtMiru::FuncNameID::NextPage            ,
		TxtMiru::FuncNameID::PrevPage            ,
		TxtMiru::FuncNameID::FirstPage           ,
		TxtMiru::FuncNameID::EndPage             ,
		TxtMiru::FuncNameID::GotoPage            ,
		TxtMiru::FuncNameID::NextFile            ,
		TxtMiru::FuncNameID::PrevFile            ,
		TxtMiru::FuncNameID::ForwardPage         ,
		TxtMiru::FuncNameID::BackPage            ,
		TxtMiru::FuncNameID::OpenBrowser         ,
		TxtMiru::FuncNameID::AddBookmark         ,
		TxtMiru::FuncNameID::GotoBookmark1       ,
		TxtMiru::FuncNameID::GotoBookmark2       ,
		TxtMiru::FuncNameID::GotoBookmark3       ,
		TxtMiru::FuncNameID::GotoBookmark4       ,
		TxtMiru::FuncNameID::GotoBookmark5       ,
		TxtMiru::FuncNameID::GotoBookmark6       ,
		TxtMiru::FuncNameID::GotoBookmark7       ,
		TxtMiru::FuncNameID::GotoBookmark8       ,
		TxtMiru::FuncNameID::GotoBookmark9       ,
		TxtMiru::FuncNameID::GotoBookmark0       ,
		TxtMiru::FuncNameID::ShowBookmark        ,
		TxtMiru::FuncNameID::ShowHScrollBar      ,
		TxtMiru::FuncNameID::ShowVScrollBar      ,
		TxtMiru::FuncNameID::HideHScrollBar      ,
		TxtMiru::FuncNameID::HideVScrollBar      ,
		TxtMiru::FuncNameID::ToggleHScrollBar    ,
		TxtMiru::FuncNameID::ToggleVScrollBar    ,
		TxtMiru::FuncNameID::FullScreen          ,
		TxtMiru::FuncNameID::ShowSubtitle        ,
		TxtMiru::FuncNameID::ShowSubtitleBookmark,
		TxtMiru::FuncNameID::AddFavorite         ,
		TxtMiru::FuncNameID::ShowBookList        ,
		TxtMiru::FuncNameID::ShowAozoraList      ,
		TxtMiru::FuncNameID::ShowRubyList        ,
		TxtMiru::FuncNameID::ShowProperty        ,
		TxtMiru::FuncNameID::ShowDocInfo         ,
		TxtMiru::FuncNameID::Copy                ,
		TxtMiru::FuncNameID::Search              ,
		TxtMiru::FuncNameID::SearchNext          ,
		TxtMiru::FuncNameID::SearchPrev          ,
		TxtMiru::FuncNameID::SearchFiles         ,
		TxtMiru::FuncNameID::Config              ,
		TxtMiru::FuncNameID::Help                ,
		TxtMiru::FuncNameID::Version             ,
		TxtMiru::FuncNameID::LayoutSet           ,
		TxtMiru::FuncNameID::ToggleLupe          ,
		TxtMiru::FuncNameID::ShowLupe            ,
		TxtMiru::FuncNameID::HideLupe            ,
		TxtMiru::FuncNameID::SetLupeZoom100      ,
		TxtMiru::FuncNameID::SetLupeZoom150      ,
		TxtMiru::FuncNameID::SetLupeZoom200      ,
		TxtMiru::FuncNameID::SetLupeZoom400      ,
		TxtMiru::FuncNameID::RefreshPreParserList,
		TxtMiru::FuncNameID::ToggleCopyRuby      ,
		TxtMiru::FuncNameID::ExecOpenFiile       ,
		TxtMiru::FuncNameID::ExecOpenFiile1      ,
		TxtMiru::FuncNameID::ExecOpenFiile2      ,
		TxtMiru::FuncNameID::LinkGoto            ,
		TxtMiru::FuncNameID::LinkOpen            ,
		TxtMiru::FuncNameID::ToggleSelectionMode ,
		TxtMiru::FuncNameID::Exit                ,
	};
	for(auto id : func_list){
		const auto &item = l_funcNameMap.find(static_cast<TxtMiru::FuncNameID>(id));
		if(l_funcNameMap.end() != item){
			std::tstring str;
			CGrText::LoadString(item->second, str);
			int i = m_listFunc.AddString(str.c_str());
			m_listFunc.SetItemData(i, static_cast<LPARAM>(id));
		}
	}
	m_listMenu.SetCurSel(0);
	m_listMenu.SetCaretIndex(0);
	m_listFunc.SetCurSel(0);
	m_listFunc.SetCaretIndex(0);
	return TRUE;
}

void CGrPropPageMenu::OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	switch(id){
	case IDC_BUTTON_ATTACH:
		{
			int iFunc = m_listFunc.GetCurSel();
			if(iFunc == LB_ERR){
				break;
			}
			int iMenu = m_listMenu.GetCurSel();
			if(iMenu == LB_ERR){
				break;
			}
			TCHAR buf[2048];
			m_listFunc.GetText(iFunc, buf);
			iMenu = m_listMenu.InsertString(iMenu, buf);
			m_listMenu.SetItemData(iMenu, m_listFunc.GetItemData(iFunc));
			PropSheet_Changed(::GetParent(hwnd), hwnd);
		}
		break;
	case IDC_BUTTON_DETACH:
		{
			int iMenu = m_listMenu.GetCurSel();
			if(iMenu == LB_ERR){
				break;
			}
			if(iMenu == m_listMenu.GetCount()-1){
				break;
			}
			m_listMenu.DeleteString(iMenu);
			m_listMenu.SetCurSel(iMenu);
			PropSheet_Changed(::GetParent(hwnd), hwnd);
		}
		break;
	case IDC_BUTTON_UP:
		{
			int iMenu = m_listMenu.GetCurSel();
			if(iMenu == LB_ERR){
				break;
			}
			if(iMenu == 0 || iMenu == m_listMenu.GetCount()-1){
				break;
			}
			TCHAR buf[2048];
			m_listMenu.GetText(iMenu, buf);
			int id = m_listMenu.GetItemData(iMenu);
			m_listMenu.DeleteString(iMenu);
			iMenu = m_listMenu.InsertString(iMenu-1, buf);
			m_listMenu.SetItemData(iMenu, id);
			m_listMenu.SetCurSel(iMenu);
			PropSheet_Changed(::GetParent(hwnd), hwnd);
		}
		break;
	case IDC_BUTTON_DOWN:
		{
			int iMenu = m_listMenu.GetCurSel();
			if(iMenu == LB_ERR){
				break;
			}
			if(iMenu >= m_listMenu.GetCount()-2){
				break;
			}
			TCHAR buf[2048];
			m_listMenu.GetText(iMenu, buf);
			int id = m_listMenu.GetItemData(iMenu);
			m_listMenu.DeleteString(iMenu);
			iMenu = m_listMenu.InsertString(iMenu+1, buf);
			m_listMenu.SetItemData(iMenu, id);
			m_listMenu.SetCurSel(iMenu);
			PropSheet_Changed(::GetParent(hwnd), hwnd);
		}
		break;
	}
}

bool CGrPropPageMenu::Apply()
{
	auto &&param = CGrTxtFunc::Param();
	//
	CGrCSVText csv;
	int icnt = m_listMenu.GetCount()-1;
	for(int i=0; i<icnt; ++i){
		int id = m_listMenu.GetItemData(i);
		if(id >= 0 && id < static_cast<int>(TxtMiru::FuncNameID::MaxNum)){
			csv.AddFormatTail(_T("%s"), TxtMiru::l_TxtMiruFuncNameList[id]);
		}
	}
	std::tstring filename = _T("menu.lis");
	auto lpDataPath = CGrTxtFunc::GetDataPath();
	if(lpDataPath){
		CGrShell::ToPrettyFileName(lpDataPath, filename);
		filename += _T("/menu.lis");
	}
	csv.Save(filename.c_str());
	//
	return true;
}

static int g_padding = -1;
void CGrPropPageMenu::setWindowSize(int cx, int cy)
{
	if(g_padding < 0){
		RECT rect = {0,0,7,7};
		MapDialogRect(m_hWnd, &rect);
		g_padding = rect.right;
	}
	RECT win_rect;
	::GetClientRect(m_hWnd, &win_rect);
	cx = win_rect.right - win_rect.left;
	cy = win_rect.bottom - win_rect.top;

	if(cy < m_minClientSize.cy){
		cy = m_minClientSize.cy;
	}
	auto hdwp = BeginDeferWindowPos(2);
	{
		RECT rect;
		GetWindowRect(m_listMenu, &rect);
		ScreenToClientRect(&rect);
		hdwp = DeferWindowPos(hdwp, m_listMenu, NULL, 0, 0, rect.right-rect.left, cy-rect.top, SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOMOVE);
		GetWindowRect(m_listFunc, &rect);
		ScreenToClientRect(&rect);
		hdwp = DeferWindowPos(hdwp, m_listFunc, NULL, 0, 0, rect.right-rect.left, cy-rect.top, SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOMOVE);
	}
	EndDeferWindowPos(hdwp);
}

void CGrPropPageMenu::OnSize(HWND hwnd, UINT nType, int cx, int cy)
{
	CGrPropPageSize::OnSize(hwnd, nType, cx, cy);
	setWindowSize(cx, cy);
}
