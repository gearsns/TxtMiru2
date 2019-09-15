#ifndef __MENU_H__
#define __MENU_H__

namespace CGrMenuFunc
{
	void DeleteMenuRange(HMENU hMenu, int id, int num, bool bBreak = true);
	BOOL SetMenuItemState(HMENU hMenu, UINT id, UINT fState);
	HMENU GetParentMenu(HMENU hMenu, UINT id);
};

#endif // __MENU_H__
