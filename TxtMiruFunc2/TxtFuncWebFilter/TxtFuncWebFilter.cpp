// TxtFuncWebFilter.cpp : DLL アプリケーション用にエクスポートされる関数を定義します。
//

#include "stdafx.h"
#include "TxtFuncWebFilter.h"
#include "TxtMiruDef.h"
#include <windows.h>
#include <wininet.h>
#include <shlwapi.h>
#include <shlobj.h>
#include <mshtmdid.h>
#include <regex>
#include "atlbase.h"
#include "TxtFuncBookmark.h"

#ifndef BINDSTRING_ROOTDOC_URL
#define BINDSTRING_ROOTDOC_URL 20
#endif

#include "tchar.h"
#include "TxtFuncIParam.h"
#include "CSVText.h"
#include "Shell.h"
#include "stlutil.h"


bool g_bDebugMode = false;
LPCTSTR g_dataDir = nullptr;
CGrTxtFuncIParam *g_pParam = nullptr;
static LPCTSTR g_ini_filename = nullptr;
struct WebUrlMatch
{
	std::wregex url;
	std::wregex target;
};
std::vector<WebUrlMatch> g_webUrlMatch;
using WebTargetList = std::vector<std::wregex>;
static int g_ref = 0;

namespace CGrTxtMiruFunc
{
	void MoveDataDir()
	{
		SetCurrentDirectory(g_dataDir);
	}
	LPCTSTR GetDataPath()
	{
		return g_dataDir;
	}
	CGrTxtFuncIParam &Param()
	{
		return *g_pParam;
	}
	LPCTSTR AppName()
	{
		return _T("TxtMiru");
	}
	void GetPointsINI(LPCTSTR key, int *pnum, int num)
	{
		TCHAR str[1024];
		::GetPrivateProfileString(_T("points"), key, _T(""), str, sizeof(str)/sizeof(TCHAR), g_ini_filename);
		if(str[0] != _T('\0')){
			auto *pbegin = str;
			for(; num>0; --num, ++pnum){
				auto *p = _tcschr(pbegin, _T(','));
				if(p){
					*p = '\0';
				}
				*pnum = _tstoi(pbegin);
				if(!p){
					break;
				}
				pbegin = p + 1;
			}
		}
	}
	void SetPointsINI(LPCTSTR key, int *pnum, int num)
	{
		TCHAR buf[1024];
		_stprintf_s(buf, _T("%d"), pnum[0]);
		std::tstring str(buf);
		for(int col=1; col<num; ++col){
			_stprintf_s(buf, _T(",%d"), pnum[col]);
			str += buf;
		}
		::WritePrivateProfileString(_T("points"), key, str.c_str(), g_ini_filename);
	}
};
static bool IsBlockUrl(LPCWSTR szFromUrl, LPCWSTR szUrl)
{
	for(const auto &item : g_webUrlMatch){
		if(std::regex_search(szFromUrl, item.url) && std::regex_search(szUrl, item.target)){
			return true;
		}
	}
	return false;
}

#include "CacheDB.h"

class CInternetProtocol : public IInternetProtocol
{
public:
	static void CloseDB()
	{
		GetDBCache().Close();
	}
private:
	static CGrDBCache &GetDBCache()
	{
		static CGrDBCache db;
		return db;
	}
	bool GetCachePath(std::tstring &filename, LPCTSTR szUrl) {
		auto ret = GetDBCache().Get(filename, szUrl);
		if (!ret) {
			std::tstring url(szUrl);
			auto pos = url.find(L'#');
			if (std::string::npos != pos) {
				url[pos] = L'\0';
				ret = GetDBCache().Get(filename, url.c_str());
			}
		}
		return ret;
	}
public:
	//
	STDMETHODIMP QueryInterface(REFIID riid, void **ppvObject)
	{
		*ppvObject = nullptr;

		if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IInternetProtocolRoot) || IsEqualIID(riid, IID_IInternetProtocol)){
			*ppvObject = static_cast<IInternetProtocol *>(this);
		} else {
			return E_NOINTERFACE;
		}
		AddRef();

		return S_OK;
	}
	STDMETHODIMP_(ULONG) AddRef()
	{
		++g_ref;
		m_pProtocol->AddRef();
		return InterlockedIncrement(&m_cRef);
	}
	STDMETHODIMP_(ULONG) Release()
	{
		--g_ref;
		m_pProtocol->Release();
		if (InterlockedDecrement(&m_cRef) == 0) {
			delete this;
			return 0;
		}

		return m_cRef;
	}
	bool m_bCacheAvailable = false;
	std::vector<BYTE> m_Data;
	STDMETHODIMP Start(LPCWSTR szUrl, IInternetProtocolSink *pOIProtSink, IInternetBindInfo *pOIBindInfo, DWORD grfPI, HANDLE_PTR dwReserved)
	{
		std::tstring fromUrl;
		if(fromUrl.size() == 0 && pOIBindInfo){
			LPOLESTR urlMS;
			ULONG dwSize;
			auto hrTMP = pOIBindInfo->GetBindString(BINDSTRING_ROOTDOC_URL, &urlMS, 1, &dwSize);
			if(SUCCEEDED(hrTMP))
			{
				fromUrl = urlMS;
				CoTaskMemFree(urlMS);
			} else {
			}
		}
		if(fromUrl.size() == 0 && pOIBindInfo){
			LPOLESTR urlMS;
			ULONG dwSize;
			auto hrTMP = pOIBindInfo->GetBindString(BINDSTRING_XDR_ORIGIN, &urlMS, 1, &dwSize);
			if(SUCCEEDED(hrTMP))
			{
				fromUrl = urlMS;
				CoTaskMemFree(urlMS);
			} else {
			}
		}
		if(fromUrl.size() == 0 && pOIBindInfo){
			LPOLESTR urlMS;
			ULONG dwSize;
			auto hrTMP = pOIBindInfo->GetBindString(BINDSTRING_URL, &urlMS, 1, &dwSize);
			if(SUCCEEDED(hrTMP))
			{
				fromUrl = urlMS;
				CoTaskMemFree(urlMS);
			} else {
			}
		}
		m_url = szUrl;
		if(::IsBlockUrl(fromUrl.c_str(), szUrl)){
			return S_FALSE;
		}
		do {
			int iWebFilter[2] = {};
			const auto &param = CGrTxtMiruFunc::Param();
			param.GetPoints(CGrTxtFuncIParam::PointsType::WebFilter, iWebFilter, _countof(iWebFilter));
			std::tstring filename;
			if (iWebFilter[1] == 1 && GetCachePath(filename, szUrl) && filename.size() > 0) {
				if (PathFileExists(filename.c_str())) {
					auto hFile = CreateFileW(filename.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
					if (hFile == INVALID_HANDLE_VALUE) {
						break;
					}
					if (pOIBindInfo) {
						DWORD dwBindFlags;
						DWORD dwNoCacheFlag = BINDF_ASYNCHRONOUS | BINDF_ASYNCSTORAGE | BINDF_PULLDATA;
						BINDINFO bi = { sizeof(BINDINFO) };
						pOIBindInfo->GetBindInfo(&dwBindFlags, &bi);
						if ((dwBindFlags & dwNoCacheFlag) != dwNoCacheFlag) {
							pOIProtSink->ReportProgress(BINDSTATUS_CACHEFILENAMEAVAILABLE, filename.c_str());
							m_bCacheAvailable = true;
						}
					}
					auto ext = CGrShell::GetFileExtConst(filename.c_str());
					if (lstrcmpi(ext, L"jpeg") == 0 || lstrcmp(ext, L"jpg") == 0) {
						pOIProtSink->ReportProgress(BINDSTATUS_MIMETYPEAVAILABLE, L"image/jpeg");
						pOIProtSink->ReportProgress(BINDSTATUS_VERIFIEDMIMETYPEAVAILABLE, L"image/jpeg");
					}
					else if (lstrcmpi(ext, L"png") == 0) {
						pOIProtSink->ReportProgress(BINDSTATUS_VERIFIEDMIMETYPEAVAILABLE, L"image/png");
					}
					else if (lstrcmpi(ext, L"gif") == 0) {
						pOIProtSink->ReportProgress(BINDSTATUS_VERIFIEDMIMETYPEAVAILABLE, L"image/gif");
					}
					else if (lstrcmpi(ext, L"txt") == 0) {
						pOIProtSink->ReportProgress(BINDSTATUS_VERIFIEDMIMETYPEAVAILABLE, L"text/plain");
					}
					else {
						pOIProtSink->ReportProgress(BINDSTATUS_VERIFIEDMIMETYPEAVAILABLE, L"text/html");
					}
					//auto dwTotalSize = dwFileSize + header.size();
					DWORD dwTotalSize = 0;
					if (m_bCacheAvailable) {
						dwTotalSize = 0;
					}
					else {
						auto dwFileSize = GetFileSize(hFile, nullptr);
						dwTotalSize = dwFileSize;
						m_Data.resize(dwTotalSize);
						DWORD a;
						if (ReadFile(hFile, &m_Data[0], dwFileSize, &a, nullptr)) {
							; // success
						}
					}
					CloseHandle(hFile);
					pOIProtSink->ReportData(BSCF_FIRSTDATANOTIFICATION | BSCF_LASTDATANOTIFICATION | BSCF_DATAFULLYAVAILABLE, dwTotalSize, dwTotalSize);
					pOIProtSink->ReportResult(S_OK, 200, NULL); // 200 = HTTP OK
					return S_OK;
				}
			}
		} while (0);
		return m_pProtocol->Start(szUrl, pOIProtSink, pOIBindInfo, grfPI, dwReserved);
	}
	STDMETHODIMP Continue(PROTOCOLDATA *pProtocolData){ return m_pProtocol->Continue(pProtocolData); }
	STDMETHODIMP Abort(HRESULT hrReason, DWORD dwOptions){
		return m_pProtocol->Abort(hrReason, dwOptions);
	}
	STDMETHODIMP Terminate(DWORD dwOptions){
		m_Data.clear();
		return m_pProtocol->Terminate(dwOptions);
	}
	STDMETHODIMP Suspend(){ return m_pProtocol->Suspend(); }
	STDMETHODIMP Resume(){ return m_pProtocol->Resume(); }

	DWORD m_dwReadSize = 0;
	STDMETHODIMP Read(void *pv, ULONG cb, ULONG *pcbRead)
	{
		if (m_bCacheAvailable) {
			*pcbRead = m_Data.size();
			return S_FALSE;
		}
		if (m_Data.size() > 0) {
			if (m_dwReadSize >= m_Data.size()) {
				return S_FALSE;
			}
			DWORD rb = m_Data.size() - m_dwReadSize;
			if (cb > rb) {
				cb = static_cast<ULONG>(rb);
			}
			*pcbRead = cb;
			CopyMemory(pv, &m_Data[0] + m_dwReadSize, cb);
			m_dwReadSize += cb;
			return S_OK;
		}

		auto hr = m_pProtocol->Read(pv, cb, pcbRead);
		return hr;
	}
	STDMETHODIMP Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition)
	{
		return m_pProtocol->Seek(dlibMove, dwOrigin, plibNewPosition);
	}
	STDMETHODIMP LockRequest(DWORD dwOptions){ return m_pProtocol->LockRequest(dwOptions); }
	STDMETHODIMP UnlockRequest(){ return m_pProtocol->UnlockRequest(); }

	CInternetProtocol(REFCLSID clsid)
	{
		m_cRef = 1;
		if (CoCreateInstance(clsid, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_pProtocol)) == S_OK) {
			;
		}
	}
	~CInternetProtocol()
	{
	}
private:
	LONG                 m_cRef;
	std::tstring         m_url;
	IInternetProtocolEx *m_pProtocol = nullptr;
};

class CClassFactory : public IClassFactory
{
private:
	LONG     m_cRef = 1;
	REFCLSID m_clsid;
public:
	STDMETHODIMP QueryInterface(REFIID riid, void **ppvObject)
	{
		*ppvObject = nullptr;

		if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IClassFactory)){
			*ppvObject = static_cast<IClassFactory *>(this);
		} else {
			return E_NOINTERFACE;
		}
		AddRef();

		return S_OK;
	}
	STDMETHODIMP_(ULONG) AddRef()
	{
		return InterlockedIncrement(&m_cRef);
	}
	STDMETHODIMP_(ULONG) Release()
	{
		if (InterlockedDecrement(&m_cRef) == 0) {
			delete this;
			return 0;
		}

		return m_cRef;
	}

	CClassFactory(REFCLSID clsid) : m_clsid(clsid)
	{
	}
	STDMETHODIMP CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppvObject)
	{
		*ppvObject = nullptr;

		if(pUnkOuter != nullptr){
			return CLASS_E_NOAGGREGATION;
		}
		auto p = new CInternetProtocol(m_clsid);
		if (p == nullptr){
			return E_OUTOFMEMORY;
		}
		auto hr = p->QueryInterface(riid, ppvObject);
		p->Release();

		return hr;
	}
	STDMETHODIMP LockServer(BOOL fLock){ return S_OK; }
	~CClassFactory()
	{
	}
};

class CWebFilterData
{
private:
	bool m_bRegist = false;
	LONG m_cRef = 0;
	IClassFactory *m_pCFHttp  = nullptr;
	IClassFactory *m_pCFHttps = nullptr;
public:
	bool RegisterNameSpace()
	{
		++m_cRef;
		if(m_cRef > 1){
			return true;
		}
		m_bRegist = true;
		if(!m_pCFHttp){
			m_pCFHttp  = new CClassFactory(CLSID_HttpProtocol );
		}
		if(!m_pCFHttps){
			m_pCFHttps = new CClassFactory(CLSID_HttpSProtocol);
		}
		CComPtr<IInternetSession> spSession;
		auto hr = CoInternetGetSession(0, &spSession, 0);
		if(FAILED(hr) || !spSession){
			return false;
		}
		if(!SUCCEEDED(spSession->RegisterNameSpace(m_pCFHttp , CLSID_NULL, L"http" , 0, nullptr, 0))){
			return false;
		}
		if(!SUCCEEDED(spSession->RegisterNameSpace(m_pCFHttps, CLSID_NULL, L"https", 0, nullptr, 0))){
			return false;
		}
		return true;
	}
	bool UnregisterNameSpace()
	{
		--m_cRef;
		if(m_cRef <= 0){
			m_cRef = 0;
			if(m_bRegist){
				m_bRegist = false;
				CComPtr<IInternetSession> spSession;
				auto hr = CoInternetGetSession(0, &spSession, 0);
				if(FAILED(hr) || !spSession){
				} else {
					if(m_pCFHttp){
						spSession->UnregisterNameSpace(m_pCFHttp , L"http" );
					}
					if(m_pCFHttps){
						spSession->UnregisterNameSpace(m_pCFHttps, L"https");
					}
				}
				return true;
			}
		}
		return false;
	}
	~CWebFilterData()
	{
		UnregisterNameSpace();
		if(m_pCFHttp){
			m_pCFHttp->Release();
			delete m_pCFHttp;
		}
		if(m_pCFHttps){
			m_pCFHttps->Release();
			delete m_pCFHttps;
		}
	}
};

static bool LoadWebFilter()
{
	if(g_webUrlMatch.size() > 0){
		return true;
	}
	TCHAR block_file[512];
	_stprintf_s(block_file, _T("%s/urlblock.txt"), g_dataDir);
	CGrCSVText csv;
	if(csv.Open(block_file)){
		int max_row = csv.GetRowSize();
		auto *p = g_webUrlMatch.data();
		for(int row=0; row<max_row; ++row, ++p){
			int col_num = csv.GetColmnSize(row);
			if(col_num == 2 && csv.GetText(row, 0)[0] != _T('#')){
				try {
					WebUrlMatch wum;
					wum.url    = std::wregex(csv.GetText(row, 0), std::regex_constants::icase);
					wum.target = std::wregex(csv.GetText(row, 1), std::regex_constants::icase);
					g_webUrlMatch.push_back(wum);
				} catch(...){}
			}
		}
	}
	return true;
}

#define MUTEX TEXT("TXTMIRUWEBFILTER_MUTEX_%u")
class CWaitMutex
{
private:
	HANDLE m_hMutex;
public:
	CWaitMutex(){
		TCHAR buf[2048];
		_stprintf_s(buf, MUTEX, ::GetCurrentProcessId());
		m_hMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, buf);
		if (m_hMutex) {
			WaitForSingleObject(m_hMutex, INFINITE);
		}
	}
	~CWaitMutex(){
		ReleaseMutex(m_hMutex);
		CloseHandle(m_hMutex);
	}
};

static HWND g_hWnd = NULL;
static UINT_PTR g_webfilter_timerid = 0;
static TIMERPROC g_lpTimerFunc = nullptr;
static CWebFilterData *g_pWfd = nullptr;

void UnInstallAllWebFilter()
{
	if(g_pWfd){
		g_pWfd->UnregisterNameSpace();
		delete g_pWfd;
		g_pWfd = nullptr;
	}
	g_webUrlMatch.clear();
}

static void CALLBACK WebFilterTimerProc(
	HWND hwnd,         // ウィンドウのハンドル
	UINT uMsg,         // WM_TIMER メッセージ
	UINT_PTR idEvent,  // タイマの識別子
	DWORD dwTime       // 現在のシステム時刻
	)
{
	KillTimer(hwnd, idEvent);
	g_webfilter_timerid = 0;

	auto lpTimerFunc = g_lpTimerFunc;
	do {
		CWaitMutex w;
		if(g_lpTimerFunc == nullptr){
			return;
		}
		g_lpTimerFunc = nullptr;
	} while(0);
	if(lpTimerFunc){
		if(g_ref <= 0){
			g_webUrlMatch.clear();
		} else if(g_hWnd){
			g_lpTimerFunc = lpTimerFunc;
			g_webfilter_timerid = SetTimer(g_hWnd, 1, 5000, WebFilterTimerProc);
		}
	}
}


using FuncCreateThreadpoolTimer = PTP_TIMER
(WINAPI*)(
	__in        PTP_TIMER_CALLBACK   pfnti,
	__inout_opt PVOID                pv,
	__in_opt    PTP_CALLBACK_ENVIRON pcbe
	);
using FuncSetThreadpoolTimer = VOID
(WINAPI*)
(
	__inout  PTP_TIMER pti,
	__in_opt PFILETIME pftDueTime,
	__in     DWORD     msPeriod,
	__in_opt DWORD     msWindowLength
	);
using FuncCloseThreadpoolTimer = VOID
(WINAPI*)
(
	__inout PTP_TIMER pti
	);

static HMODULE g_hKernel32 = NULL;
FuncCreateThreadpoolTimer pfnCreateThreadpoolTimer = nullptr;
FuncSetThreadpoolTimer pfncSetThreadpoolTimer = nullptr;
FuncCloseThreadpoolTimer pfncCloseThreadpoolTimer = nullptr;
static bool LoadTimerFunc()
{
	static bool bLoadTimerFunc = false;
	if (!bLoadTimerFunc) {
		bLoadTimerFunc = true;
		g_hKernel32 = ::LoadLibrary(_T("Kernel32.dll"));
		if (!g_hKernel32) {
			return false;
		}
		pfnCreateThreadpoolTimer = reinterpret_cast<FuncCreateThreadpoolTimer>(GetProcAddress(g_hKernel32, "CreateThreadpoolTimer"));
		pfncSetThreadpoolTimer = reinterpret_cast<FuncSetThreadpoolTimer>(GetProcAddress(g_hKernel32, "SetThreadpoolTimer"));
		pfncCloseThreadpoolTimer = reinterpret_cast<FuncCloseThreadpoolTimer>(GetProcAddress(g_hKernel32, "CloseThreadpoolTimer"));
	}
	return pfnCreateThreadpoolTimer != nullptr;
}
static void FreeTimerFunc()
{
	if (g_hKernel32) {
		::FreeLibrary(g_hKernel32);
	}
	g_hKernel32 = NULL;
}
PTP_TIMER g_Timer = nullptr;
static void clearTimer()
{
	if(g_hWnd && !g_webfilter_timerid){
		KillTimer(g_hWnd, g_webfilter_timerid);
	}
	if (g_Timer) {
		pfncCloseThreadpoolTimer(g_Timer);
		g_Timer = nullptr;
	}
	g_lpTimerFunc = nullptr;
	g_webfilter_timerid = 0;
}

static void __stdcall timer_fired(PTP_CALLBACK_INSTANCE, PVOID context, PTP_TIMER timer)
{
	g_Timer = nullptr;
	pfncCloseThreadpoolTimer(timer);
	CInternetProtocol::CloseDB();
}
static PTP_TIMER startTimer(long long sec)
{
	if (!LoadTimerFunc()) {
		return nullptr;
	}
	auto lpTimer = pfnCreateThreadpoolTimer(timer_fired, NULL, NULL);
	ULARGE_INTEGER ulRelativeStartTime;
	ulRelativeStartTime.QuadPart = (LONGLONG)(-sec * 10000000LL);//100nano秒単位で指定する

	FILETIME ftStartTime;
	ftStartTime.dwHighDateTime = ulRelativeStartTime.HighPart;
	ftStartTime.dwLowDateTime = ulRelativeStartTime.LowPart;

	pfncSetThreadpoolTimer(lpTimer,	&ftStartTime, 0, 0);

	return lpTimer;
}


extern "C" TXTFUNCWEBFILTER_API void cdecl TxtFuncWebFilterRegister(LPCTSTR lpDataDir, CGrTxtFuncIParam *pParam)
{
	{
		g_bDebugMode = false;
		TCHAR *buffer = nullptr;
		size_t len = 0;
		if (0 == _tdupenv_s(&buffer, &len, _T("TXTMIRU_WEBFILTER_DEBUG"))) {
			g_bDebugMode = (buffer && (0 == _tcscmp(buffer, _T("1"))));
			free(buffer);
		}
	}
	CWaitMutex w;
	clearTimer();
	g_dataDir = lpDataDir;
	g_pParam  = pParam;

	if(!g_pWfd){
		g_pWfd = new CWebFilterData();
	}
	if(g_pWfd->RegisterNameSpace()){
		LoadWebFilter();
	} else {
		g_pWfd->UnregisterNameSpace();
	}
}

extern "C" TXTFUNCWEBFILTER_API void cdecl TxtFuncWebFilterUnregister(HWND hWnd, TIMERPROC lpTimerFunc)
{
	CWaitMutex w;
	clearTimer();
	g_hWnd        = hWnd;
	g_lpTimerFunc = lpTimerFunc;
	if(g_pWfd && g_pWfd->UnregisterNameSpace() && g_hWnd && g_lpTimerFunc){
		g_webfilter_timerid = SetTimer(g_hWnd, 1, 5000, WebFilterTimerProc);
		g_Timer = startTimer(5);
	}
}
