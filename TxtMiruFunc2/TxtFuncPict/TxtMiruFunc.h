#ifndef __TXTMIRUFUNC_H__
#define __TXTMIRUFUNC_H__

#ifndef MemberSizeOf
#define MemberSizeOf(type, member)       sizeof(((type*)0)->member)
#endif

namespace CGrTxtFunc
{
	void GetPointsINI(LPCTSTR key, int *pnum, int num);
	void SetPointsINI(LPCTSTR key, int *pnum, int num);
	HINSTANCE GetDllModuleHandle();
};

#endif // __TXTMIRUFUNC_H__
