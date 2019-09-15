#pragma once

#define SetProcAddressPtr(_func, _hModule, _procName) {_func = reinterpret_cast<decltype(_func)>(::GetProcAddress(_hModule, _procName));}
#define SetProcPtr(_func, _ptr) {_func = reinterpret_cast<decltype(_func)>(_ptr);}
