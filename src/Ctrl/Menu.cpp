#include <windows.h>
#include <windowsx.h>
#include "Menu.h"

namespace CGrMenuFunc
{
	void DeleteMenuRange(HMENU hMenu, int id, int num, bool bBreak/*=true*/)
	{
		for(int idx=0; idx<num; ++idx){
			if(!::DeleteMenu(hMenu, id + idx, MF_BYCOMMAND)){
				if(bBreak){
					break;
				}
			}
		}
	}
	BOOL SetMenuItemState(HMENU hMenu, UINT id, UINT fState)
	{
		if(hMenu){
			MENUITEMINFO mi = {sizeof(MENUITEMINFO)};
			mi.fMask  = MIIM_STATE;
			mi.fState = fState;
			return ::SetMenuItemInfo(hMenu, id, FALSE, &mi);
		} else {
			return FALSE;
		}
	}
	HMENU GetParentMenu(HMENU hMenu, UINT id)
	{
		if(!hMenu){
			return NULL;
		}
		for(int i = GetMenuItemCount(hMenu)-1; i>=0; --i){
			if(id == GetMenuItemID(hMenu, i)){
				return hMenu;
			}
			auto hSubMenu = GetSubMenu(hMenu, i);
			if(hSubMenu){
				hSubMenu = GetParentMenu(hSubMenu, id);
				if(hSubMenu){
					return hSubMenu;
				}
			}
		}
		return NULL;
	}
};
