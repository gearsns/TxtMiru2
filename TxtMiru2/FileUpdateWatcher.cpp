#pragma warning( disable : 4786 )

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "resource.h"
#include "FileUpdateWatcher.h"
#include "Shell.h"
#include <process.h>

//
unsigned int __stdcall CGrFileUpdateWatcher::watch(void *arg)
{
	auto *pfuw = static_cast<CGrFileUpdateWatcher *>(arg);
	std::tstring filename = pfuw->m_filename;
	auto hWnd = pfuw->m_hWnd;
	WIN32_FIND_DATA org_wfd;
	if(!CGrShell::getFileInfo(filename.c_str(), org_wfd)){
		return 1;
	}
	// 監視条件
	DWORD filter = FILE_NOTIFY_CHANGE_FILE_NAME /* ファイル名の変更 */ | FILE_NOTIFY_CHANGE_DIR_NAME /* ディレクトリ名の変更 */ | FILE_NOTIFY_CHANGE_ATTRIBUTES /* 属性の変更 */ | FILE_NOTIFY_CHANGE_SIZE /* サイズの変更 */ | FILE_NOTIFY_CHANGE_LAST_WRITE /* 最終書き込み日時の変更 */;
	TCHAR dir[_MAX_PATH] = {0};
	_tcscpy_s(dir, pfuw->m_filename.c_str());
	if(!CGrShell::GetParentDir(dir, NULL)){
		return 1;
	}
	auto hWatch = FindFirstChangeNotification(dir, FALSE/*サブディレクトリ監視*/, filter);
	if(hWatch == INVALID_HANDLE_VALUE){
		return 1;
	}
	const auto &param = CGrTxtMiru::theApp().Param();
	int iFileAutoReload[3] = {};
	param.GetPoints(CGrTxtParam::FileAutoReload, iFileAutoReload, sizeof(iFileAutoReload)/sizeof(int));
	int iSleep = iFileAutoReload[2];
	if(iSleep <= 0){
		iSleep = 1000;
	}
	HANDLE hFile = NULL;
	while(true){
		auto waitResult = WaitForSingleObject(hWatch, 500);
		if(pfuw->m_bBreak){
			break;
		}
		if(waitResult == WAIT_TIMEOUT){
			continue;
		}
		Sleep(iSleep); // 更新書き込みまでiSleep(ms)待つ
		if(iFileAutoReload[1] == 1){
			// 排他制御で読めるかどうかを判断
			for(int i=0; i<100; ++i){
				hFile = ::CreateFile(filename.c_str(), GENERIC_READ, 0/*許可しない*/, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, 0);
				if(hFile == NULL || INVALID_HANDLE_VALUE == hFile){
					Sleep(iSleep); // 更新書き込みまでiSleep(ms)待つ
				} else {
					CloseHandle(hFile);
					hFile = NULL;
					break;
				}
			}
		}
		WIN32_FIND_DATA wfd;
		if(!CGrShell::getFileInfo(filename.c_str(), wfd)){
			continue;
		}
		if(wfd.ftLastWriteTime.dwLowDateTime != org_wfd.ftLastWriteTime.dwLowDateTime
		   || wfd.ftLastWriteTime.dwHighDateTime != org_wfd.ftLastWriteTime.dwHighDateTime){
			FORWARD_WM_COMMAND(hWnd, IDRELOAD/*id*/, NULL/*hwndCtl*/, 0/*codeNotify*/, PostMessage);
			break;
		}
		// 監視の継続
		if(!FindNextChangeNotification(hWatch)){
			break;
		}
	}
	if (hFile) {
		CloseHandle(hFile);
	}
	// 監視の終了
	FindCloseChangeNotification(hWatch);
	return 0;
}

CGrFileUpdateWatcher::CGrFileUpdateWatcher() : m_waittime(0), m_handle(NULL), m_bBreak(false), m_hWnd(NULL)
{
}
CGrFileUpdateWatcher::~CGrFileUpdateWatcher()
{
	stop();
}
bool CGrFileUpdateWatcher::start(LPCTSTR filename, HWND hWnd)
{
	stop();
	if(!filename || !filename[0]){
		return false;
	}
	m_hWnd = hWnd;
	m_bBreak = false;
	m_filename = filename;
	m_handle = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, watch, this, 0, nullptr));
	return true;
}
void CGrFileUpdateWatcher::stop()
{
	m_bBreak = true;
	if(m_handle){
		::WaitForSingleObject(m_handle, INFINITE);
		::CloseHandle(m_handle);
	}
	m_handle = NULL;
}
