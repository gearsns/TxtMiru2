#pragma warning( disable : 4786 )

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <shlobj.h>
#include "resource.h"
#include "Text.h"
#include "TxtConfigFunc.h"

 void SetDlgItemTextType(HWND hwnd, CGrTxtFuncIParam &param, UINT id, CGrTxtFuncIParam::TextType tt)
{
	TCHAR buf[1024];
	param.GetText(tt, buf, sizeof(buf)/sizeof(TCHAR));
	SetDlgItemText(hwnd, id, buf);
}
void GetDlgItemTextType(HWND hwnd, CGrTxtFuncIParam &param, UINT id, CGrTxtFuncIParam::TextType tt)
{
	TCHAR str[2048];
	GetDlgItemText(hwnd, id, str, sizeof(str)/sizeof(TCHAR));
	param.SetText(tt, str);
}

void SetCheckDlgItemID(HWND hWnd, UINT id, BOOL flag)
{
	Button_SetCheck(GetDlgItem(hWnd, id), flag ? BST_CHECKED : BST_UNCHECKED);
}
UINT GetCheckDlgItemID(HWND hWnd, UINT id)
{
	return Button_GetCheck(GetDlgItem(hWnd, id));
}
void EnableDlgItemID(HWND hWnd, UINT id, BOOL flag)
{
	EnableWindow(GetDlgItem(hWnd, id), flag);
}
void EnableCheckDlgItemID(HWND hwnd, UINT id_check, UINT id_edit, UINT id_ctl)
{
	if(GetCheckDlgItemID(hwnd, id_check) == BST_UNCHECKED){
		EnableDlgItemID(hwnd, id_edit, FALSE);
		EnableDlgItemID(hwnd, id_ctl , FALSE);
	} else {
		EnableDlgItemID(hwnd, id_edit, TRUE );
		EnableDlgItemID(hwnd, id_ctl , TRUE );
	}
}

void CheckDlgButton(HWND hwnd, CGrTxtFuncIParam &param, UINT id, CGrTxtFuncIParam::PointsType pt)
{
	int iFlag = 0;
	param.GetPoints(pt, &iFlag, 1);
	CheckDlgButton(hwnd, id, iFlag);
}

void SetDlgItemPointSet(HWND hwnd, PointSet ps[], int ps_size)
{
	auto &&param = CGrTxtFunc::Param();
	int points[4];
	int idx=0;
	auto type = ps[0].type;
	while(1){
		int num = 0;
		while(type == ps[idx+num].type && idx+num < ps_size){
			++num;
		}
		memset(points, 0x00, sizeof(points));
		param.GetPoints(ps[idx].type, points, num);
		int *ppoints = points;
		for(; num>0; --num, ++ppoints){
			SetDlgItemInt(hwnd, ps[idx].id, *ppoints, true);
			++idx;
		}
		if(idx >= ps_size){
			break;
		}
		type = ps[idx].type;
	}
}

void GetDlgItemPointSet(HWND hwnd, PointSet ps[], int ps_size)
{
	auto &&param = CGrTxtFunc::Param();
	int points[4];
	int idx=0;
	auto type = ps[0].type;
	BOOL translated;
	while(1){
		int num = 0;
		memset(points, 0x00, sizeof(points));
		int *ppoints = points;
		while(type == ps[idx+num].type && idx+num < ps_size){
			*ppoints = GetDlgItemInt(hwnd, ps[idx+num].id, &translated, true);
			++num;
			++ppoints;
		}
		if(num > 0){
			param.SetPoints(ps[idx].type, points, num);
		}
		idx += num;
		if(idx >= ps_size){
			break;
		}
		type = ps[idx].type;
	}
}

void SetDlgItemPointRange(HWND hwnd, PointRange pr[], int pr_size)
{
	for(int idx=0; idx<pr_size; ++idx){
		::SendMessage(GetDlgItem(hwnd, pr[idx].id), UDM_SETRANGE32, (WPARAM)pr[idx].max, (LPARAM)pr[idx].min);
	}
}

static int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	if(uMsg == BFFM_INITIALIZED){
		::SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
	}
	return 0;
}

bool BrowseForFolder(HWND hwnd, UINT id, UINT mes_id)
{
	TCHAR fileName[MAX_PATH+1];
	GetDlgItemText(hwnd, id, fileName, sizeof(fileName)/sizeof(TCHAR));

	BROWSEINFO bi = {};
	std::tstring str;
	CGrText::LoadString(mes_id, str);
	bi.hwndOwner      = hwnd;
	bi.pszDisplayName = fileName;
	bi.lpszTitle      = str.c_str();
	bi.ulFlags        = BIF_RETURNONLYFSDIRS;
	bi.lpfn           = &BrowseCallbackProc;
	bi.lParam         = reinterpret_cast<LPARAM>(fileName);
	auto pidl = ::SHBrowseForFolder(&bi);
	if(pidl){
		::SHGetPathFromIDList(pidl, fileName);
		::CoTaskMemFree(pidl);
		SetDlgItemText(hwnd, id, fileName);
		return true;
	}
	return false;
}
