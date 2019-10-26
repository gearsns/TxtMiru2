#ifndef __TXTMIRUFUNC_H__
#define __TXTMIRUFUNC_H__

#ifdef TXTMIRUFUNCNAMELIST
#undef TXTMIRUFUNCNAMELIST
#undef TXTMIRUFL
#undef TXTMIRUFUNCNAMELIST_END
#undef TXTMIRUFUNCNAMELIST_EXTERN
#endif

#ifdef TXTMIRUAPP
#define TXTMIRUFUNCNAMELIST LPCTSTR l_TxtMiruFuncNameList[] =
#define TXTMIRUFL(__a__) _T(#__a__)
#define TXTMIRUFUNCNAMELIST_END
#define TXTMIRUFUNCNAMELIST_EXTERN
#else
#define TXTMIRUFUNCNAMELIST enum class FuncNameID
#define TXTMIRUFL(__a__) __a__
#define TXTMIRUFUNCNAMELIST_END    MaxNum
#define TXTMIRUFUNCNAMELIST_EXTERN extern LPCTSTR l_TxtMiruFuncNameList[static_cast<int>(FuncNameID::MaxNum)];
#endif

namespace TxtMiru
{
	TXTMIRUFUNCNAMELIST
	{
		TXTMIRUFL(ShowBookList        ),
		TXTMIRUFL(ShowRubyList        ),
		TXTMIRUFL(ShowSubtitleBookmark),
		TXTMIRUFUNCNAMELIST_END
		};
	TXTMIRUFUNCNAMELIST_EXTERN;
};

#endif // __TXTMIRUFUNC_H__
