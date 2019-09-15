#ifndef __TXTCONFIGFUNC_H__
#define __TXTCONFIGFUNC_H__

#include "stltchar.h"
#include "stlutil.h"
#include "TxtFuncConfig.h"
#include "TxtFunc.h"

struct FontSpacing {
	UINT id_spacing;
	UINT id_center;
	CGrTxtFuncIParam::CharType type;
};
struct FontWeightName {
	UINT str_id;
	long lfWeight;
};
struct FontWeight {
	UINT id;
	CGrTxtFuncIParam::CharType type;
};
struct FontName {
	UINT id;
	CGrTxtFuncIParam::CharType type;
};
struct PointSet {
	UINT id;
	CGrTxtFuncIParam::PointsType type;
};
struct ValueSet {
	UINT id;
	CGrTxtFuncIParam::ValueType type;
};
struct PointRange {
	UINT id;
	int min;
	int max;
	UINT err_id;
};

void SetDlgItemTextType(HWND hwnd, CGrTxtFuncIParam &param, UINT id, CGrTxtFuncIParam::TextType tt);
void GetDlgItemTextType(HWND hwnd, CGrTxtFuncIParam &param, UINT id, CGrTxtFuncIParam::TextType tt);
void SetCheckDlgItemID(HWND hWnd, UINT id, BOOL flag);
UINT GetCheckDlgItemID(HWND hWnd, UINT id);
void EnableDlgItemID(HWND hWnd, UINT id, BOOL flag);
void EnableCheckDlgItemID(HWND hwnd, UINT id_check, UINT id_edit, UINT id_ctl);
void CheckDlgButton(HWND hwnd, CGrTxtFuncIParam &param, UINT id, CGrTxtFuncIParam::PointsType pt);
void SetDlgItemPointSet(HWND hwnd, PointSet ps[], int ps_size);
void GetDlgItemPointSet(HWND hwnd, PointSet ps[], int ps_size);
void SetDlgItemPointRange(HWND hwnd, PointRange pr[], int pr_size);
bool BrowseForFolder(HWND hwnd, UINT id, UINT mes_id);

#endif // __TXTCONFIGFUNC_H__
