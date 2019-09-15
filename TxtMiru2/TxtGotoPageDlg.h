#ifndef __TXTGOTOPAGEDLG_H__
#define __TXTGOTOPAGEDLG_H__

#include <windows.h>
namespace CGrTxtGotoPageDlg
{
	int DoModal(HWND hWnd, int maxnum, int cur_page, int *out_page);
};

#endif // __TXTGOTOPAGEDLG_H__
