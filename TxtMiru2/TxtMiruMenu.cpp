#pragma warning( disable : 4786 )

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "resource.h"
#include "TxtMiru.h"
#include "TxtMiruDef.h"
#define TXTMIRUMENU_STR_H 
#include "TxtMiruFunc.h"
#include "TxtMiruMenu.h"
#include "TxtMiruTheme.h"
#include "Text.h"
#include "Menu.h"
#include "Shell.h"
#include "CSVText.h"

#define MOUSEEVENTF_FROMTOUCH 0xFF515700
#define l_funcNameMap TxtMiru::l_TxtMiruFuncStrIDList

CGrTxtMiruMenu::CGrTxtMiruMenu()
{
}

CGrTxtMiruMenu::~CGrTxtMiruMenu()
{
}

static void addMenuItem(HMENU hMenu, TxtMiru::FuncNameID id)
{
	MENUITEMINFO mii = {sizeof(MENUITEMINFO)};
	mii.fMask  = MIIM_ID | MIIM_TYPE;
	mii.wID    = static_cast<int>(id)+1;

	std::tstring str;
	auto str_id = l_funcNameMap[static_cast<int>(id)];
	if(str_id > 0){
		CGrText::LoadString(str_id, str);
		mii.fType      = MFT_STRING;
		mii.dwTypeData = const_cast<LPTSTR>(str.c_str());
	} else {
		mii.fType = MFT_SEPARATOR;
	}
	InsertMenuItem(hMenu, GetMenuItemCount(hMenu), TRUE, &mii);
}

static std::map<TxtMiru::FuncNameID,std::tstring> l_menu_list;
void CGrTxtMiruMenu::Load()
{
	l_menu_list.clear();
	std::tstring filename = _T("menu_name.lis");
	auto lpDataPath = CGrTxtMiru::GetDataPath();
	if(lpDataPath){
		CGrShell::ToPrettyFileName(lpDataPath, filename);
		filename += _T("/menu_name.lis");
	}
	CGrCSVText csv;
	if(csv.Open(filename.c_str())){
		for(const auto &item : csv.GetCSVROW()){
			if(item.size() < 2){
				continue;
			}
			const auto &str = item[0];
			for(int i=0; i<static_cast<int>(TxtMiru::FuncNameID::MaxNum); ++i){
				if(lstrcmp(TxtMiru::l_TxtMiruFuncNameList[i], str.c_str()) == 0){
					l_menu_list[static_cast<TxtMiru::FuncNameID>(i)] = item[1];
					break;
				}
			}
		}
	}
}
void CGrTxtMiruMenu::ConvertMenuString(HMENU hMenu, UINT *funcMenuNameMap)
{
	for(const auto &item : l_menu_list){
		auto id = static_cast<int>(item.first);
		const auto &str = item.second;
		MENUITEMINFO mi = {sizeof(MENUITEMINFO)};
		mi.fMask = MIIM_STRING;
		mi.dwTypeData = const_cast<TCHAR*>(str.c_str());
		mi.cch        = str.size();
		if(funcMenuNameMap && id >= 0 && id <= static_cast<int>(TxtMiru::FuncNameID::MaxNum) && funcMenuNameMap[id] > 0){
			::SetMenuItemInfo(hMenu, funcMenuNameMap[id], FALSE, &mi);
		} else {
			::SetMenuItemInfo(hMenu, id+1, FALSE, &mi);
		}
	}
}

bool CGrTxtMiruMenu::Open()
{
	std::tstring filename = _T("menu.lis");
	auto lpDataPath = CGrTxtMiru::GetDataPath();
	if(lpDataPath){
		CGrShell::ToPrettyFileName(lpDataPath, filename);
		filename += _T("/menu.lis");
	}
	m_list.clear();
	CGrCSVText csv;
	if(csv.Open(filename.c_str())){
		for(const auto &item : csv.GetCSVROW()){
			const auto &str = item[0];
			for(int i=0; i< static_cast<int>(TxtMiru::FuncNameID::MaxNum); ++i){
				if(lstrcmp(TxtMiru::l_TxtMiruFuncNameList[i], str.c_str()) == 0){
					m_list.push_back(static_cast<TxtMiru::FuncNameID>(i));
					break;
				}
			}
		}
	}
	if(m_list.empty()){
		m_list = {
			TxtMiru::FuncNameID::Copy, TxtMiru::FuncNameID::ToggleSelectionMode, TxtMiru::FuncNameID::Nop,
			TxtMiru::FuncNameID::PrevPage, TxtMiru::FuncNameID::NextPage, TxtMiru::FuncNameID::Nop,
			TxtMiru::FuncNameID::ShowSubtitleBookmark, TxtMiru::FuncNameID::AddBookmark, TxtMiru::FuncNameID::Nop,
			TxtMiru::FuncNameID::ShowBookList, TxtMiru::FuncNameID::Nop,
			TxtMiru::FuncNameID::ShowDocInfo, TxtMiru::FuncNameID::ShowProperty, TxtMiru::FuncNameID::Nop,
			TxtMiru::FuncNameID::ExecOpenFiile, TxtMiru::FuncNameID::FullScreen, TxtMiru::FuncNameID::Nop,
			TxtMiru::FuncNameID::MaxNum};
	} else {
		m_list.push_back(TxtMiru::FuncNameID::Nop   );
		m_list.push_back(TxtMiru::FuncNameID::MaxNum);
	}
	return true;
}

TxtMiru::FuncNameID CGrTxtMiruMenu::Show(HWND hWnd, int x, int y, bool bSelectionMode, bool bLinkHover)
{
	auto hMenu = CreatePopupMenu();
	for(const auto &id : m_list){
		addMenuItem(hMenu, id);
	}
	CGrTxtMiruMenu::ConvertMenuString(hMenu, nullptr);
	auto &&param = CGrTxtMiru::theApp().Param();
	if(param.GetBoolean(CGrTxtParam::PointsType::TouchMenu)){
		// タッチ操作でメニューを出したときは、OwnerDrawで幅を広げる
		TxtMiruTheme_SetTouchMenu(hMenu);
	}
	//
	UINT clipFormat = CF_TEXT;
	if(::GetPriorityClipboardFormat(&clipFormat, 1) <= 0){
		CGrMenuFunc::SetMenuItemState(hMenu, static_cast<int>(TxtMiru::FuncNameID::Copy)+1, MFS_DISABLED);
	}
	CGrMenuFunc::SetMenuItemState(hMenu, static_cast<int>(TxtMiru::FuncNameID::ToggleSelectionMode)+1, bSelectionMode ? MFS_CHECKED : MFS_UNCHECKED);
	if(!bLinkHover){
		CGrMenuFunc::SetMenuItemState(hMenu, static_cast<int>(TxtMiru::FuncNameID::LinkGoto)+1, MFS_DISABLED);
		CGrMenuFunc::SetMenuItemState(hMenu, static_cast<int>(TxtMiru::FuncNameID::LinkOpen)+1, MFS_DISABLED);
	}
	UINT ret = ::TrackPopupMenu(hMenu, TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_VERTICAL|TPM_RETURNCMD, x, y, 0, hWnd, NULL);
	::DestroyMenu(hMenu);
	if(ret == 0){
		return TxtMiru::FuncNameID::Nop;
	} else {
		return static_cast<TxtMiru::FuncNameID>(ret-1);
	}
}
