#ifndef __FILEUPDATEWATCHER_H__
#define __FILEUPDATEWATCHER_H__

#include "TxtMiru.h"
#include "stltchar.h"
#include "TxtMiruDef.h"

class CGrFileUpdateWatcher
{
private:
	static unsigned int __stdcall watch(void *arg);
private:
	int m_waittime;
	std::tstring m_filename;
	HANDLE m_handle;
	bool m_bBreak;
	HWND m_hWnd;
public:
	CGrFileUpdateWatcher();
	~CGrFileUpdateWatcher();
	bool start(LPCTSTR filename, HWND hWnd);
	void stop();
};

#endif // __FILEUPDATEWATCHER_H__
