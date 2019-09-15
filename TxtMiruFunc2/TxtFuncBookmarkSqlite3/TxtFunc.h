#ifndef __TXTFUNC_H__
#define __TXTFUNC_H__

#ifndef MemberSizeOf
#define MemberSizeOf(type, member)       sizeof(((type*)0)->member)
#endif

#include "TxtFuncBookmarkDef.h"

class CGrTxtFuncIParam;
class CGrModelessWnd;
namespace CGrTxtFunc
{
	void MoveDataDir();
	LPCTSTR GetDataPath();
	CGrTxtFuncIParam &Param();
	LPCTSTR AppName();
	HINSTANCE GetDllModuleHandle();
	bool IsWaitting();
	void OpenFile(HWND hWnd, LPCTSTR lpUrl, int page);
	bool UpdateCheck(HWND hWnd, HWND hTarget, LPCTSTR lpUrl);
	int GetCurrentPage(HWND hWnd);
	int GetCurrentDisplayPage(HWND hWnd);
	bool GetSinglePage(HWND hWnd);
	void GetBookmarkFolder(CGrTxtFuncIParam *pParam, std::tstring &str);
	int AddBookmark(HWND hWnd);
	int DeleteBookmark(HWND hWnd, int idx); 
	bool BackupImgFile(int id); 
	bool RemoveBackupImgFolder(HWND hWnd);
	bool GetBookByUrl(TxtFuncBookmark::Book& book, LPCTSTR lpURL);
};

#endif // __TXTFUNC_H__
