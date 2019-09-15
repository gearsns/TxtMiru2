#ifndef __TXTFUNC_H__
#define __TXTFUNC_H__

#ifndef MemberSizeOf
#define MemberSizeOf(type, member)       sizeof(((type*)0)->member)
#endif

class CGrTxtFuncIParam;
namespace CGrTxtFunc
{
	void MoveDataDir();
	LPCTSTR GetDataPath();
	CGrTxtFuncIParam &Param();
	LPCTSTR AppName();
	HINSTANCE GetDllModuleHandle();
	void GetBookmarkFolder(CGrTxtFuncIParam *pParam, std::tstring &str);
};

#endif // __TXTFUNC_H__
