#ifndef __DEBUG_H__
#define __DEBUG_H__

#ifdef __DBG__
#include <math.h>
#include <time.h>
#include <map>
class TimeCheck
{
	clock_t start;
	LPCTSTR lpTypeName;
	static auto &getTimeCheckMap()
	{
		static std::map<std::tstring,TimeCheck*> g_TimeCheckMap;
		return g_TimeCheckMap;
	}
public:
	TimeCheck(LPCTSTR n) : lpTypeName(n){
		start = clock();
		getTimeCheckMap()[lpTypeName] = this;
	}
	~TimeCheck(){
		Print();
		getTimeCheckMap()[lpTypeName] = nullptr;
	}
	void Print()
	{
		_ftprintf(stderr, _T("%s:%d:%f\n"), lpTypeName, start, ((double)clock() - (double)start) / CLOCKS_PER_SEC);
		fflush(stderr);
	}
	static void Print(LPCTSTR name)
	{
		auto *p = getTimeCheckMap()[name];
		if(p){
			p->Print();
		}
	}
};

#define DBGPRINT(format, ...) {_ftprintf(stderr, _T("%s:%d:%s:"), _T(__FILE__), __LINE__, _T(__FUNCTION__));_ftprintf(stderr, format, __VA_ARGS__); _ftprintf(stderr, _T("\n")); fflush(stderr); }
#define DBGPRINTA(format, ...) {_ftprintf(stderr, _T("%s:%d:%s:"), _T(__FILE__), __LINE__, _T(__FUNCTION__));fprintf(stderr, format, __VA_ARGS__); _ftprintf(stderr, _T("\n")); fflush(stderr); }
#define DBGTIMECHECK(name) TimeCheck t_##__LINE__(name)
#define DBGTIMECHECK_PRINT(name) TimeCheck::Print(name)
#else
#define DBGPRINT(format, ...)
#define DBGPRINTA(format, ...)
#define DBGTIMECHECK(name)
#define DBGTIMECHECK_PRINT(name)
#endif

#endif // __DEBUG_H__
