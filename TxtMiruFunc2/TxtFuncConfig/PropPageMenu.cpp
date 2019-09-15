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
	{TxtMiru::FnN_ReadFile            ,IDS_READFILE            },
	{TxtMiru::FnN_URLOpen             ,IDS_URLOPEN             },
	{TxtMiru::FnN_OpenFolder          ,IDS_OPENFOLDER          },
	{TxtMiru::FnN_OpenBrowser         ,IDS_OPENBROWSER         },
	{TxtMiru::FnN_ReadClipboard       ,IDS_READCLIPBOAD        },
	{TxtMiru::FnN_Reload              ,IDS_RELOAD              },
	{TxtMiru::FnN_NextPage            ,IDS_NEXTPAGE            },
	{TxtMiru::FnN_PrevPage            ,IDS_PREVPAGE            },
	{TxtMiru::FnN_FirstPage           ,IDS_FIRSTPAGE           },
	{TxtMiru::FnN_EndPage             ,IDS_ENDPAGE             },
	{TxtMiru::FnN_GotoPage            ,IDS_GOTOPAGE            },
	{TxtMiru::FnN_NextFile            ,IDS_NEXTFILE            },
	{TxtMiru::FnN_PrevFile            ,IDS_PREVFILE            },
	{TxtMiru::FnN_ForwardPage         ,IDS_FORWARDPAGE         },
	{TxtMiru::FnN_BackPage            ,IDS_BACKPAGE            },
	{TxtMiru::FnN_AddBookmark         ,IDS_ADDBOOKMARK         },
	{TxtMiru::FnN_GotoBookmark1       ,IDS_GOTOBOOKMARK1       },
	{TxtMiru::FnN_GotoBookmark2       ,IDS_GOTOBOOKMARK2       },
	{TxtMiru::FnN_GotoBookmark3       ,IDS_GOTOBOOKMARK3       },
	{TxtMiru::FnN_GotoBookmark4       ,IDS_GOTOBOOKMARK4       },
	{TxtMiru::FnN_GotoBookmark5       ,IDS_GOTOBOOKMARK5       },
	{TxtMiru::FnN_GotoBookmark6       ,IDS_GOTOBOOKMARK6       },
	{TxtMiru::FnN_GotoBookmark7       ,IDS_GOTOBOOKMARK7       },
	{TxtMiru::FnN_GotoBookmark8       ,IDS_GOTOBOOKMARK8       },
	{TxtMiru::FnN_GotoBookmark9       ,IDS_GOTOBOOKMARK9       },
	{TxtMiru::FnN_GotoBookmark0       ,IDS_GOTOBOOKMARK0       },
	{TxtMiru::FnN_AddFavorite         ,IDS_ADDFAVORITE         },
	{TxtMiru::FnN_ShowBookmark        ,IDS_SHOWBOOKMARK        },
	{TxtMiru::FnN_ShowHScrollBar      ,IDS_SHOW_HSCROLLBAR     },
	{TxtMiru::FnN_ShowVScrollBar      ,IDS_SHOW_VSCROLLBAR     },
	{TxtMiru::FnN_HideHScrollBar      ,IDS_HIDE_HSCROLLBAR     },
	{TxtMiru::FnN_HideVScrollBar      ,IDS_HIDE_VSCROLLBAR     },
	{TxtMiru::FnN_ToggleHScrollBar    ,IDS_TOGGLE_HSCROLLBAR   },
	{TxtMiru::FnN_ToggleVScrollBar    ,IDS_TOGGLE_VSCROLLBAR   },
	{TxtMiru::FnN_FullScreen          ,IDS_FULLSCREEN          },
	{TxtMiru::FnN_ShowSubtitle        ,IDS_SHOWSUBTITLE        },
	{TxtMiru::FnN_ShowSubtitleBookmark,IDS_SHOWSUBTITLEBOOKMARK},
	{TxtMiru::FnN_ShowBookList        ,IDS_FAVORITE            },
	{TxtMiru::FnN_ShowAozoraList      ,IDS_BMAOZORA            },
	{TxtMiru::FnN_ShowRubyList        ,IDS_SHOWRUBYLIST        },
	{TxtMiru::FnN_ShowProperty        ,IDS_SHOWPROPERTY        },
	{TxtMiru::FnN_ShowDocInfo         ,IDS_SHOWDOCINFO         },
	{TxtMiru::FnN_Copy                ,IDS_COPY                },
	{TxtMiru::FnN_Search              ,IDS_SEARCH              },
	{TxtMiru::FnN_SearchNext          ,IDS_SEARCHNEXT          },
	{TxtMiru::FnN_SearchPrev          ,IDS_SEARCHPREV          },
	{TxtMiru::FnN_SearchFiles         ,IDS_SEARCHFILES         },
	{TxtMiru::FnN_Config              ,IDS_CONFIG              },
	{TxtMiru::FnN_Help                ,IDS_SHOW_HELP           },
	{TxtMiru::FnN_Version             ,IDS_SHOW_VERSION        },
	{TxtMiru::FnN_LayoutSet           ,IDS_LAYOUTSET           },
	{TxtMiru::FnN_ToggleLupe          ,IDS_TOGGLE_LUPE         },
	{TxtMiru::FnN_ShowLupe            ,IDS_SHOW_LUPE           },
	{TxtMiru::FnN_HideLupe            ,IDS_HIDE_LUPE           },
	{TxtMiru::FnN_SetLupeZoom100      ,IDS_LUPE100             },
	{TxtMiru::FnN_SetLupeZoom150      ,IDS_LUPE150             },
	{TxtMiru::FnN_SetLupeZoom200      ,IDS_LUPE200             },
	{TxtMiru::FnN_SetLupeZoom400      ,IDS_LUPE400             },
	{TxtMiru::FnN_RefreshPreParserList,IDS_REFRESHPREPARSERLIST},
	{TxtMiru::FnN_ToggleCopyRuby      ,IDS_TOGGLECOPYRUBY      },
	{TxtMiru::FnN_ExecOpenFiile       ,IDS_OPENFILEEXE         },
	{TxtMiru::FnN_ExecOpenFiile1      ,IDS_OPENFILEEXE1        },
	{TxtMiru::FnN_ExecOpenFiile2      ,IDS_OPENFILEEXE2        },
	{TxtMiru::FnN_LinkGoto            ,IDS_LINKGOTO            },
	{TxtMiru::FnN_LinkOpen            ,IDS_LINKOPEN            },
	{TxtMiru::FnN_ToggleSelectionMode ,IDS_SELECTIONMODE       },
	{TxtMiru::FnN_ShowContextMenu     ,IDS_SHOWCONTEXTMENU     },
	{TxtMiru::FnN_MaxNum              ,IDS_CANCEL              },
	{TxtMiru::FnN_Nop                 ,IDS_SEPARATOR           },
	{TxtMiru::FnN_Exit                ,IDS_EXIT                },
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
				for (int i = 0; i < TxtMiru::FnN_MaxNum; ++i) {
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
			TxtMiru::FnN_Copy, TxtMiru::FnN_ToggleSelectionMode, TxtMiru::FnN_Nop,
			TxtMiru::FnN_PrevPage, TxtMiru::FnN_NextPage, TxtMiru::FnN_Nop,
			TxtMiru::FnN_ShowSubtitleBookmark, TxtMiru::FnN_AddBookmark, TxtMiru::FnN_Nop,
			TxtMiru::FnN_ShowBookList, TxtMiru::FnN_Nop,
			TxtMiru::FnN_ShowDocInfo, TxtMiru::FnN_ShowProperty, TxtMiru::FnN_Nop,
			TxtMiru::FnN_ExecOpenFiile, TxtMiru::FnN_FullScreen
			};
	}
	fnid_list.push_back(TxtMiru::FnN_Nop);
	for(const auto &id: fnid_list){
		const auto &item = l_funcNameMap.find(static_cast<TxtMiru::FuncNameID>(id));
		if(l_funcNameMap.end() != item){
			std::tstring str;
			CGrText::LoadString(item->second, str);
			int i = m_listMenu.AddString(str.c_str());
			m_listMenu.SetItemData(i, id);
		}
	}
	TxtMiru::FuncNameID func_list[] = {
		TxtMiru::FnN_Nop                 ,
		TxtMiru::FnN_ReadFile            ,
		TxtMiru::FnN_URLOpen             ,
		TxtMiru::FnN_OpenFolder          ,
		TxtMiru::FnN_ReadClipboard       ,
		TxtMiru::FnN_Reload              ,
		TxtMiru::FnN_NextPage            ,
		TxtMiru::FnN_PrevPage            ,
		TxtMiru::FnN_FirstPage           ,
		TxtMiru::FnN_EndPage             ,
		TxtMiru::FnN_GotoPage            ,
		TxtMiru::FnN_NextFile            ,
		TxtMiru::FnN_PrevFile            ,
		TxtMiru::FnN_ForwardPage         ,
		TxtMiru::FnN_BackPage            ,
		TxtMiru::FnN_OpenBrowser         ,
		TxtMiru::FnN_AddBookmark         ,
		TxtMiru::FnN_GotoBookmark1       ,
		TxtMiru::FnN_GotoBookmark2       ,
		TxtMiru::FnN_GotoBookmark3       ,
		TxtMiru::FnN_GotoBookmark4       ,
		TxtMiru::FnN_GotoBookmark5       ,
		TxtMiru::FnN_GotoBookmark6       ,
		TxtMiru::FnN_GotoBookmark7       ,
		TxtMiru::FnN_GotoBookmark8       ,
		TxtMiru::FnN_GotoBookmark9       ,
		TxtMiru::FnN_GotoBookmark0       ,
		TxtMiru::FnN_ShowBookmark        ,
		TxtMiru::FnN_ShowHScrollBar      ,
		TxtMiru::FnN_ShowVScrollBar      ,
		TxtMiru::FnN_HideHScrollBar      ,
		TxtMiru::FnN_HideVScrollBar      ,
		TxtMiru::FnN_ToggleHScrollBar    ,
		TxtMiru::FnN_ToggleVScrollBar    ,
		TxtMiru::FnN_FullScreen          ,
		TxtMiru::FnN_ShowSubtitle        ,
		TxtMiru::FnN_ShowSubtitleBookmark,
		TxtMiru::FnN_AddFavorite         ,
		TxtMiru::FnN_ShowBookList        ,
		TxtMiru::FnN_ShowAozoraList      ,
		TxtMiru::FnN_ShowRubyList        ,
		TxtMiru::FnN_ShowProperty        ,
		TxtMiru::FnN_ShowDocInfo         ,
		TxtMiru::FnN_Copy                ,
		TxtMiru::FnN_Search              ,
		TxtMiru::FnN_SearchNext          ,
		TxtMiru::FnN_SearchPrev          ,
		TxtMiru::FnN_SearchFiles         ,
		TxtMiru::FnN_Config              ,
		TxtMiru::FnN_Help                ,
		TxtMiru::FnN_Version             ,
		TxtMiru::FnN_LayoutSet           ,
		TxtMiru::FnN_ToggleLupe          ,
		TxtMiru::FnN_ShowLupe            ,
		TxtMiru::FnN_HideLupe            ,
		TxtMiru::FnN_SetLupeZoom100      ,
		TxtMiru::FnN_SetLupeZoom150      ,
		TxtMiru::FnN_SetLupeZoom200      ,
		TxtMiru::FnN_SetLupeZoom400      ,
		TxtMiru::FnN_RefreshPreParserList,
		TxtMiru::FnN_ToggleCopyRuby      ,
		TxtMiru::FnN_ExecOpenFiile       ,
		TxtMiru::FnN_ExecOpenFiile1      ,
		TxtMiru::FnN_ExecOpenFiile2      ,
		TxtMiru::FnN_LinkGoto            ,
		TxtMiru::FnN_LinkOpen            ,
		TxtMiru::FnN_ToggleSelectionMode ,
		TxtMiru::FnN_Exit                ,
	};
	for(auto id : func_list){
		const auto &item = l_funcNameMap.find(static_cast<TxtMiru::FuncNameID>(id));
		if(l_funcNameMap.end() != item){
			std::tstring str;
			CGrText::LoadString(item->second, str);
			int i = m_listFunc.AddString(str.c_str());
			m_listFunc.SetItemData(i, id);
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
		if(id >= 0 && id < TxtMiru::FnN_MaxNum){
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
