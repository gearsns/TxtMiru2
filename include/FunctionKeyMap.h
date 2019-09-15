#ifndef __FUNCTIONKEYMAP_H__
#define __FUNCTIONKEYMAP_H__

#include "KeyState.h"
#include <map>

namespace CGrFunctionKeyMap
{
	typedef std::map<CGrKeyboardState,std::tstring> FunctionMap;

	void add(LPCTSTR line, int len, FunctionMap &fmap);
	bool Load(LPCTSTR lpFileName, FunctionMap &fmap);
	bool Save(LPCTSTR lpFileName, const FunctionMap &fmap);
};

#endif // __FUNCTIONKEYMAP_H__
