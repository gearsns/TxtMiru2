#ifndef __STLTCHAR_H__
#define __STLTCHAR_H__

#include <string>
#include <sstream>
#include "tchar.h"
namespace std {
	typedef basic_string<TCHAR> tstring;
	typedef basic_ostringstream<TCHAR> tostringstream;
}

#endif // __STLTCHAR_H__
