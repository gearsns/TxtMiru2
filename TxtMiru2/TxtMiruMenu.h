#ifndef __TXTMIRUMENU_H__
#define __TXTMIRUMENU_H__

class CGrTxtMiruMenu
{
private:
	std::vector<TxtMiru::FuncNameID> m_list;
public:
	CGrTxtMiruMenu();
	virtual ~CGrTxtMiruMenu();
	bool Open();
	TxtMiru::FuncNameID Show(HWND hWnd, int x, int y, bool bSelectionMode, bool bLinkHover);
	static void ConvertMenuString(HMENU hMenu, UINT *funcMenuNameMap);
	static void Load();
};

#endif // __TXTMIRUMENU_H__
