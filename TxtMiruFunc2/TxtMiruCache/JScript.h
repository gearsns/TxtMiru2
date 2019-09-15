#pragma once

#include <windows.h>
#include "stlutil.h"

bool MakeCache(std::tstring &outstr, LPCTSTR lpFileName);
int DeleteCache(LPCTSTR lpFileName);