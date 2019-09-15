#pragma warning( disable : 4786 )

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <algorithm>
#include "resource.h"
#include "PropPageKey.h"
#include "TxtConfigDlg.h"
#include "Text.h"
#include "Shell.h"
#include "TxtFunc.h"
#include "TxtMiruFunc.h"
#include "MessageBox.h"
#include "TxtFuncConfig.h"
#include "TxtConfigFunc.h"

#define KEYBIND_FILE   _T("keybind.ini")

static struct KeyNameMap {
	UINT id;
	TxtMiru::FuncNameID fnid;
} l_KeyNameMap[] = {
	{IDS_READFILE            ,TxtMiru::FnN_ReadFile            },
	{IDS_URLOPEN             ,TxtMiru::FnN_URLOpen             },
	{IDS_OPENFOLDER          ,TxtMiru::FnN_OpenFolder          },
	{IDS_OPENBROWSER         ,TxtMiru::FnN_OpenBrowser         },
	{IDS_READCLIPBOAD        ,TxtMiru::FnN_ReadClipboard       },
	{IDS_RELOAD              ,TxtMiru::FnN_Reload              },
	{IDS_NEXTPAGE            ,TxtMiru::FnN_NextPage            },
	{IDS_PREVPAGE            ,TxtMiru::FnN_PrevPage            },
	{IDS_FIRSTPAGE           ,TxtMiru::FnN_FirstPage           },
	{IDS_ENDPAGE             ,TxtMiru::FnN_EndPage             },
	{IDS_GOTOPAGE            ,TxtMiru::FnN_GotoPage            },
	{IDS_NEXTFILE            ,TxtMiru::FnN_NextFile            },
	{IDS_PREVFILE            ,TxtMiru::FnN_PrevFile            },
	{IDS_FORWARDPAGE         ,TxtMiru::FnN_ForwardPage         },
	{IDS_BACKPAGE            ,TxtMiru::FnN_BackPage            },
	{IDS_ADDBOOKMARK         ,TxtMiru::FnN_AddBookmark         },
	{IDS_GOTOBOOKMARK1       ,TxtMiru::FnN_GotoBookmark1       },
	{IDS_GOTOBOOKMARK2       ,TxtMiru::FnN_GotoBookmark2       },
	{IDS_GOTOBOOKMARK3       ,TxtMiru::FnN_GotoBookmark3       },
	{IDS_GOTOBOOKMARK4       ,TxtMiru::FnN_GotoBookmark4       },
	{IDS_GOTOBOOKMARK5       ,TxtMiru::FnN_GotoBookmark5       },
	{IDS_GOTOBOOKMARK6       ,TxtMiru::FnN_GotoBookmark6       },
	{IDS_GOTOBOOKMARK7       ,TxtMiru::FnN_GotoBookmark7       },
	{IDS_GOTOBOOKMARK8       ,TxtMiru::FnN_GotoBookmark8       },
	{IDS_GOTOBOOKMARK9       ,TxtMiru::FnN_GotoBookmark9       },
	{IDS_GOTOBOOKMARK0       ,TxtMiru::FnN_GotoBookmark0       },
	{IDS_SHOWBOOKMARK        ,TxtMiru::FnN_ShowBookmark        },
	{IDS_SHOW_HSCROLLBAR     ,TxtMiru::FnN_ShowHScrollBar      },
	{IDS_SHOW_VSCROLLBAR     ,TxtMiru::FnN_ShowVScrollBar      },
	{IDS_HIDE_HSCROLLBAR     ,TxtMiru::FnN_HideHScrollBar      },
	{IDS_HIDE_VSCROLLBAR     ,TxtMiru::FnN_HideVScrollBar      },
	{IDS_TOGGLE_HSCROLLBAR   ,TxtMiru::FnN_ToggleHScrollBar    },
	{IDS_TOGGLE_VSCROLLBAR   ,TxtMiru::FnN_ToggleVScrollBar    },
	{IDS_FULLSCREEN          ,TxtMiru::FnN_FullScreen          },
	{IDS_SHOWSUBTITLE        ,TxtMiru::FnN_ShowSubtitle        },
	{IDS_SHOWSUBTITLEBOOKMARK,TxtMiru::FnN_ShowSubtitleBookmark},
	{IDS_ADDFAVORITE         ,TxtMiru::FnN_AddFavorite         },
	{IDS_FAVORITE            ,TxtMiru::FnN_ShowBookList        },
	{IDS_BMAOZORA            ,TxtMiru::FnN_ShowAozoraList      },
	{IDS_SHOWRUBYLIST        ,TxtMiru::FnN_ShowRubyList        },
	{IDS_SHOWPROPERTY        ,TxtMiru::FnN_ShowProperty        },
	{IDS_SHOWDOCINFO         ,TxtMiru::FnN_ShowDocInfo         },
	{IDS_COPY                ,TxtMiru::FnN_Copy                },
	{IDS_SEARCH              ,TxtMiru::FnN_Search              },
	{IDS_SEARCHNEXT          ,TxtMiru::FnN_SearchNext          },
	{IDS_SEARCHPREV          ,TxtMiru::FnN_SearchPrev          },
	{IDS_SEARCHFILES         ,TxtMiru::FnN_SearchFiles         },
	{IDS_CONFIG              ,TxtMiru::FnN_Config              },
	{IDS_SHOW_HELP           ,TxtMiru::FnN_Help                },
	{IDS_SHOW_VERSION        ,TxtMiru::FnN_Version             },
	{IDS_LAYOUTSET           ,TxtMiru::FnN_LayoutSet           },
	{IDS_TOGGLE_LUPE         ,TxtMiru::FnN_ToggleLupe          },
	{IDS_SHOW_LUPE           ,TxtMiru::FnN_ShowLupe            },
	{IDS_HIDE_LUPE           ,TxtMiru::FnN_HideLupe            },
	{IDS_LUPE100             ,TxtMiru::FnN_SetLupeZoom100      },
	{IDS_LUPE150             ,TxtMiru::FnN_SetLupeZoom150      },
	{IDS_LUPE200             ,TxtMiru::FnN_SetLupeZoom200      },
	{IDS_LUPE400             ,TxtMiru::FnN_SetLupeZoom400      },
	{IDS_REFRESHPREPARSERLIST,TxtMiru::FnN_RefreshPreParserList},
	{IDS_TOGGLECOPYRUBY      ,TxtMiru::FnN_ToggleCopyRuby      },
	{IDS_OPENFILEEXE         ,TxtMiru::FnN_ExecOpenFiile       },
	{IDS_OPENFILEEXE1        ,TxtMiru::FnN_ExecOpenFiile1      },
	{IDS_OPENFILEEXE2        ,TxtMiru::FnN_ExecOpenFiile2      },
	{IDS_LINKGOTO            ,TxtMiru::FnN_LinkGoto            },
	{IDS_LINKOPEN            ,TxtMiru::FnN_LinkOpen            },
	{IDS_SHOWCONTEXTMENU     ,TxtMiru::FnN_ShowContextMenu     },
	{IDS_EXIT                ,TxtMiru::FnN_Exit                },
	{IDS_NOP                 ,TxtMiru::FnN_Nop                 },
};

CGrPropPage* PASCAL CGrPropPageKey::CreateProp(CGrTxtConfigDlg *pDlg)
{
	auto *pPage = new CGrPropPageKey(pDlg);
	pPage->m_hWnd = CreateDialogParam(CGrTxtFunc::GetDllModuleHandle(), MAKEINTRESOURCE(IDD_PROPPAGE_KEYBOARD), pDlg->GetWnd(), reinterpret_cast<DLGPROC>(CGrWinCtrl::WindowMapProc), reinterpret_cast<LPARAM>(pPage));
	pPage->Attach(pPage->m_hWnd);
	return pPage;
}

CGrPropPageKey::CGrPropPageKey(CGrTxtConfigDlg *pDlg) : CGrPropPageSize(pDlg)
{
}

CGrPropPageKey::~CGrPropPageKey()
{
}

// メインウィンドウ
LRESULT CGrPropPageKey::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg){
		HANDLE_MSG(hWnd, WM_INITDIALOG, OnInitDialog );
		HANDLE_MSG(hWnd, WM_NOTIFY    , OnNotify     );
		HANDLE_MSG(hWnd, WM_COMMAND   , OnCommand    );
		HANDLE_MSG(hWnd, WM_SIZE      , OnSize       );
	case WM_DRAWITEM:
		{
			auto lpdis = reinterpret_cast<DRAWITEMSTRUCT*>(lParam);
			auto idx = m_listFunc.GetParam(lpdis->itemID);
			auto const &keymap = l_KeyNameMap[idx];
			auto hdc = lpdis->hDC;
			auto &rect = lpdis->rcItem;
			auto bFocus = lpdis->itemState & ODS_FOCUS;
			auto oldfgcolor = ::SetTextColor(hdc, ::GetSysColor(bFocus ? COLOR_HIGHLIGHTTEXT : COLOR_WINDOWTEXT));
			auto oldbkcolor = ::SetBkColor  (hdc, ::GetSysColor(bFocus ? COLOR_HIGHLIGHT     : COLOR_WINDOW    ));
			::ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);

			RECT itemRect = {};
			m_listFunc.GetItemRect(lpdis->itemID, &itemRect, LVIR_LABEL);

			std::tstring str;
			CGrText::LoadString(keymap.id, str);

			DrawText(hdc, str.c_str(), -1 ,&itemRect, DT_SINGLELINE | DT_VCENTER | DT_LEFT);
			//
			ListView_GetSubItemRect(m_listFunc, lpdis->itemID, 1, LVIR_LABEL, &itemRect);
			int left = itemRect.left;
			::SetTextColor(hdc, ::GetSysColor(bFocus ? COLOR_HIGHLIGHT     : COLOR_BTNTEXT));
			::SetBkColor  (hdc, ::GetSysColor(bFocus ? COLOR_HIGHLIGHTTEXT : COLOR_BTNFACE));
			//
			std::vector<std::tstring> keyname_list;
			auto buf = TxtMiru::l_TxtMiruFuncNameList[keymap.fnid];
			for(const auto &it : m_funcMap){
				const auto &ks = it.first;
				const auto &fn = it.second;
				if(fn != buf){
					continue;
				}
				std::tstring key_name;
				ks.GetKeyName(key_name);
				keyname_list.push_back(key_name);
			}
			std::sort(keyname_list.begin(), keyname_list.end(), std::less<std::tstring>());
			for(const auto &key_name : keyname_list){
				auto drawRect = itemRect;
				drawRect.left = left;
				{
					RECT textRect = {};
					DrawText(hdc, key_name.c_str(), -1 ,&textRect, DT_SINGLELINE | DT_CALCRECT);
					drawRect.right = drawRect.left + textRect.right;
					left = drawRect.right + 8;
				}
				drawRect.top += 1;
				drawRect.bottom -= 1;
				{
					auto fillRect = drawRect;
					fillRect.left -= 2;
					fillRect.right += 2;
					::ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &fillRect, NULL, 0, NULL);
				}
				DrawText(hdc, key_name.c_str(), -1 ,&drawRect, DT_SINGLELINE | DT_VCENTER | DT_LEFT);
			}

			::SetTextColor(hdc, oldfgcolor);
			::SetBkColor  (hdc, oldbkcolor);
			return TRUE;
		}
		break;
	case WM_SETPAGEFOCUS:
		::SetFocus(reinterpret_cast<HWND>(wParam));
		break;
	}
	return CGrPropPageSize::WndProc(hWnd, uMsg, wParam, lParam);
}
int CGrPropPageKey::GetFocusedItem() const
{
	LVITEM item = { LVIF_PARAM };
	item.iItem = -1;
	while((item.iItem=m_listFunc.GetNextItem(item.iItem, LVNI_ALL | LVNI_FOCUSED))!=-1){
		break;
	}
	return item.iItem;
}

BOOL CGrPropPageKey::OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	CGrPropPageSize::OnInitDialog(hwnd, hwndFocus, lParam);
	auto &&param = CGrTxtFunc::Param();

	m_filename = m_pConfigDlg->GetKeyBindFileName();

	CGrFunctionKeyMap::Load(m_filename.c_str(), m_funcMap);
	m_listFunc.Attach(GetDlgItem(hwnd, IDC_LIST_FUNC));
	m_listKey.Attach(GetDlgItem(hwnd, IDC_LIST_KEY));
	m_editKeyBind.Attach(GetDlgItem(hwnd, IDC_EDIT_KEYBIND));
	std::tstring str_name;
	SetWindowFont(m_listFunc, GetWindowFont(hwnd), TRUE);
	LVCOLUMN lvc = {};
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvc.fmt = LVCFMT_LEFT;
	//
	//
	CGrText::LoadString(IDS_LH_FUNCNAME, str_name);
	lvc.pszText = const_cast<LPTSTR>(str_name.c_str());
	lvc.iSubItem = 0;
	lvc.cx = 200;
	m_listFunc.InsertColumn(lvc.iSubItem, &lvc);
	//
	CGrText::LoadString(IDS_LH_KEYBIND, str_name);
	lvc.pszText = const_cast<LPTSTR>(str_name.c_str());
	lvc.iSubItem = 1;
	lvc.cx = 300;
	m_listFunc.InsertColumn(lvc.iSubItem, &lvc);
	UINT styleex = m_listFunc.GetExtendedListViewStyle();
	styleex |= LVS_EX_FLATSB | LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER;
	m_listFunc.SetExtendedListViewStyle(styleex);
	//
	LVITEM item = {};
	item.mask     = LVIF_TEXT | LVIF_PARAM;
	item.iSubItem = 0;
	for(int idx=0; idx<_countof(l_KeyNameMap); ++idx){
		std::tstring str;
		CGrText::LoadString(l_KeyNameMap[idx].id, str);
		item.lParam = idx;
		item.pszText  = const_cast<LPTSTR>(str.c_str());
		item.iItem = m_listFunc.InsertItem(&item);
		++item.iItem;
	}
	m_listFunc.SetColumnWidth(0, LVSCW_AUTOSIZE);
	updateKeyList();
	setWindowSize(m_minClientSize.cx, m_minClientSize.cy);

	return TRUE;
}

void CGrPropPageKey::OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	switch(id){
	case IDC_BUTTON_ML: m_editKeyBind.SetKeyboardState(CGrKeyboardState(VK_LBUTTON, m_bDelay)); break;
	case IDC_BUTTON_MM: m_editKeyBind.SetKeyboardState(CGrKeyboardState(VK_MBUTTON, m_bDelay)); break;
	case IDC_BUTTON_MR: m_editKeyBind.SetKeyboardState(CGrKeyboardState(VK_RBUTTON, m_bDelay)); break;
	case IDC_BUTTON_MLS: m_editKeyBind.SetKeyboardState(CGrKeyboardState(VK_XBUTTON1, m_bDelay)); break;
	case IDC_BUTTON_MRS: m_editKeyBind.SetKeyboardState(CGrKeyboardState(VK_XBUTTON2, m_bDelay)); break;
	case IDC_BUTTON_TLS: m_editKeyBind.SetKeyboardState(CGrKeyboardState(VK_TILT_LBUTTON, m_bDelay)); break;
	case IDC_BUTTON_TRS: m_editKeyBind.SetKeyboardState(CGrKeyboardState(VK_TILT_RBUTTON, m_bDelay)); break;
	case IDC_BUTTON_WSU: m_editKeyBind.SetKeyboardState(CGrKeyboardState(VK_WHEEL_SCROLLUP, m_bDelay)); break;
	case IDC_BUTTON_WSD: m_editKeyBind.SetKeyboardState(CGrKeyboardState(VK_WHEEL_SCROLLDW, m_bDelay)); break;
	case IDC_BUTTON_ML2: m_editKeyBind.SetKeyboardState(CGrKeyboardState(VK_LBUTTON_DBL, m_bDelay)); break;
	case IDC_BUTTON_MM2: m_editKeyBind.SetKeyboardState(CGrKeyboardState(VK_MBUTTON_DBL, m_bDelay)); break;
	case IDC_BUTTON_MR2: m_editKeyBind.SetKeyboardState(CGrKeyboardState(VK_RBUTTON_DBL, m_bDelay)); break;
	case IDC_BUTTON_MLS2: m_editKeyBind.SetKeyboardState(CGrKeyboardState(VK_XBUTTON1_DBL, m_bDelay)); break;
	case IDC_BUTTON_MRS2: m_editKeyBind.SetKeyboardState(CGrKeyboardState(VK_XBUTTON2_DBL, m_bDelay)); break;
	case IDC_BUTTON_TLS2: m_editKeyBind.SetKeyboardState(CGrKeyboardState(VK_TILT_LBUTTON_DBL, m_bDelay)); break;
	case IDC_BUTTON_TRS2: m_editKeyBind.SetKeyboardState(CGrKeyboardState(VK_TILT_RBUTTON_DBL, m_bDelay)); break;
	case IDC_BUTTON_DELAY:
		{
			auto key = m_editKeyBind.GetKeyboardState();
			m_bDelay = ::GetCheckDlgItemID(m_hWnd, IDC_BUTTON_DELAY) == BST_CHECKED;
			m_editKeyBind.SetKeyboardState(CGrKeyboardState(key.Key(), key.Shift(), key.Ctrl(), key.Alt(), m_bDelay));
		}
		break;
	case IDC_BUTTON_ATTACH:
		{
			auto ks_input = m_editKeyBind.GetKeyboardState();
			if(ks_input.Key() == 0){
				CGrMessageBox::Show(CGrTxtFunc::GetDllModuleHandle(), m_hWnd, IDS_ERROR_NOKEYINPUT, IDS_MES_ERROR);
				break;
			}
			int index = GetFocusedItem();
			if(index < 0){
				CGrMessageBox::Show(CGrTxtFunc::GetDllModuleHandle(), m_hWnd, IDS_ERROR_SELECT_FUNC, IDS_MES_ERROR);
				break;
			}
			int icur = m_listFunc.GetParam(index);
			auto buf = TxtMiru::l_TxtMiruFuncNameList[l_KeyNameMap[icur].fnid];
			auto e = m_funcMap.end();
			auto it = m_funcMap.find(ks_input);
			if(it != e){
				const auto &fn = it->second;
				if(fn != buf){
					std::tstring key_name;
					std::tstring mes;
					std::tstring new_func;
					std::tstring old_func;
					std::tstring title;
					CGrText::LoadString(l_KeyNameMap[icur].id, new_func);
					CGrText::LoadString(IDS_MES_CONF         , title   );
					for(int idx=0; idx<sizeof(l_KeyNameMap)/sizeof(KeyNameMap); ++idx){
						if(fn == TxtMiru::l_TxtMiruFuncNameList[l_KeyNameMap[idx].fnid]){
							CGrText::LoadString(l_KeyNameMap[idx].id, old_func);
							break;
						}
					}
					ks_input.GetKeyName(key_name);
					CGrText::FormatMessage(CGrTxtFunc::GetDllModuleHandle(), mes, IDS_ERROR_KEYATTACH, key_name.c_str(), old_func.c_str(), new_func.c_str());
					if(CGrMessageBox::Show(CGrTxtFunc::GetDllModuleHandle(), m_hWnd, mes.c_str(), title.c_str(), MB_YESNO) == IDYES){
						m_funcMap[ks_input] = buf;
						updateKeyList();
						PropSheet_Changed(::GetParent(hwnd), hwnd);
					}
					break;
				}
			} else {
				m_funcMap[ks_input] = buf;
				updateKeyList();
				PropSheet_Changed(::GetParent(hwnd), hwnd);
			}
		}
		break;
	case IDC_BUTTON_DETACH:
		{
			auto ks_input = m_editKeyBind.GetKeyboardState();
			if(ks_input.Key() != 0){
				m_funcMap.erase(ks_input);
				updateKeyList();
				PropSheet_Changed(::GetParent(hwnd), hwnd);
			}
		}
		break;
	case IDC_LIST_KEY:
		if(codeNotify == LBN_SELCHANGE){
			int index = GetFocusedItem();
			if(index < 0){
				break;
			}
			int icur = m_listKey.GetItemData(index);
			m_editKeyBind.SetKeyboardState(m_keyStateList[icur]);
			m_bDelay = m_keyStateList[icur].Delay();
			::SetCheckDlgItemID(m_hWnd, IDC_BUTTON_DELAY, m_bDelay);
		}
		break;
	case IDC_LIST_FUNC:
		if(codeNotify == LBN_SELCHANGE){ updateKeyList(); }
		break;
	}
}

LRESULT CGrPropPageKey::OnNotify(HWND hWnd, int idFrom, NMHDR FAR *lpnmhdr)
{
	if(idFrom == IDC_LIST_FUNC && lpnmhdr->code == LVN_ITEMCHANGED){
		auto *lpnmlist = reinterpret_cast<LPNMLISTVIEW>(lpnmhdr);
		if(lpnmlist->iItem >= 0 && lpnmlist->uNewState == (LVIS_SELECTED | LVIS_FOCUSED)){
			updateKeyList();
			return 0;
		}
	}
	return CGrPropPage::OnNotify(hWnd, idFrom, lpnmhdr);
}

bool CGrPropPageKey::Apply()
{
	CGrFunctionKeyMap::Save(m_filename.c_str(), m_funcMap);
	auto &&param = CGrTxtFunc::Param();
	param.UpdateKeybord(GetParent(GetParent(m_hWnd)));
	//
	return true;
}

void CGrPropPageKey::updateKeyList()
{
	int index = GetFocusedItem();
	if(index < 0){
		return;
	}
	int icur = m_listFunc.GetParam(index);
	auto buf = TxtMiru::l_TxtMiruFuncNameList[l_KeyNameMap[icur].fnid];
	m_keyStateList.clear();
	m_listKey.ResetContent();
	for(const auto &it : m_funcMap){
		const auto &ks = it.first;
		const auto &fn = it.second;
		if(fn != buf){
			continue;
		}
		std::tstring key_name;
		ks.GetKeyName(key_name);
		int row = m_listKey.AddString(key_name.c_str());
		m_listKey.SetItemData(row, m_listKey.GetCount()-1);
		m_keyStateList.push_back(ks);
	}
	InvalidateRect(m_listFunc, nullptr, FALSE);
}

static int g_padding = -1;
void CGrPropPageKey::setWindowSize(int cx, int cy)
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
	RECT rect;
	int listFunc_height = cy;
	int listKey_top = 0;
	int mouse_top = 0;
	auto hdwp = BeginDeferWindowPos(9);
	{
		GetWindowRect(m_listKey, &rect);
		ScreenToClientRect(&rect);
		int height = rect.bottom - rect.top;
		listKey_top = cy-height;
		hdwp = DeferWindowPos(hdwp, m_listKey, NULL, rect.left, listKey_top, -1, -1, SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOSIZE);
		listFunc_height -= (height + g_padding);
	}
	{
		GetWindowRect(m_editKeyBind, &rect);
		ScreenToClientRect(&rect);
		hdwp = DeferWindowPos(hdwp, m_editKeyBind, NULL, rect.left, listKey_top, -1, -1, SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOSIZE);
	}
	{
		auto hCWnd = GetDlgItem(m_hWnd, IDC_BUTTON_ATTACH);
		GetWindowRect(hCWnd, &rect);
		ScreenToClientRect(&rect);
		hdwp = DeferWindowPos(hdwp, hCWnd, NULL, rect.left, listKey_top, -1, -1, SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOSIZE);
	}
	{
		auto hCWnd = GetDlgItem(m_hWnd, IDC_BUTTON_DETACH);
		GetWindowRect(hCWnd, &rect);
		ScreenToClientRect(&rect);
		hdwp = DeferWindowPos(hdwp, hCWnd, NULL, rect.left, listKey_top, -1, -1, SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOSIZE);
		mouse_top = listKey_top + (rect.bottom - rect.top) + 1;
	}
	// 2.0.11.0
	for(auto id : {IDC_STATIC_MOUSE,IDC_STATIC_MOUSE_SIDE,IDC_STATIC_MOUSE_TILT,IDC_STATIC_WHEEL,IDC_BUTTON_DELAY}){
		auto hCWnd = GetDlgItem(m_hWnd, id);
		GetWindowRect(hCWnd, &rect);
		ScreenToClientRect(&rect);
		hdwp = DeferWindowPos(hdwp, hCWnd, NULL, rect.left, mouse_top, -1, -1, SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOSIZE);
	}
	mouse_top += (rect.bottom - rect.top) + 1;
	for(auto id : {IDC_STATIC_SINGLE,IDC_BUTTON_WSU,IDC_BUTTON_WSD,IDC_BUTTON_ML, IDC_BUTTON_MM, IDC_BUTTON_MR,IDC_BUTTON_MLS,IDC_BUTTON_MRS,IDC_BUTTON_TLS,IDC_BUTTON_TRS}){
		auto hCWnd = GetDlgItem(m_hWnd, id);
		GetWindowRect(hCWnd, &rect);
		ScreenToClientRect(&rect);
		hdwp = DeferWindowPos(hdwp, hCWnd, NULL, rect.left, mouse_top, -1, -1, SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOSIZE);
	}
	mouse_top += (rect.bottom - rect.top) + 1;
	for(auto id : {IDC_STATIC_DOUBLE,IDC_BUTTON_ML2, IDC_BUTTON_MM2, IDC_BUTTON_MR2,IDC_BUTTON_MLS2,IDC_BUTTON_MRS2,IDC_BUTTON_TLS2,IDC_BUTTON_TRS2}){
		auto hCWnd = GetDlgItem(m_hWnd, id);
		GetWindowRect(hCWnd, &rect);
		ScreenToClientRect(&rect);
		hdwp = DeferWindowPos(hdwp, hCWnd, NULL, rect.left, mouse_top, -1, -1, SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOSIZE);
	}
	{
		hdwp = DeferWindowPos(hdwp, m_listFunc, NULL, 0, 0, cx-GetSystemMetrics(SM_CXEDGE), listFunc_height, SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOMOVE);
	}
	EndDeferWindowPos(hdwp);
}

void CGrPropPageKey::OnSize(HWND hwnd, UINT nType, int cx, int cy)
{
	CGrPropPageSize::OnSize(hwnd, nType, cx, cy);
	setWindowSize(cx, cy);
}
