#pragma warning( disable : 4786 )

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "resource.h"
#include "stdwin.h"
#include "TxtFuncWebFilter.h"
#include "Win32Wrap.h"

class CGrTxtFuncWebFilter : public CGrLoadDllFunc
{
private:
	void (cdecl*m_fRegister)(LPCTSTR lpDataDir, CGrTxtFuncIParam *pParam) = nullptr;
	void (cdecl*m_fUnregister)(HWND hWnd, TIMERPROC lpTimerFunc) = nullptr;
	CGrTxtFuncWebFilter() : CGrLoadDllFunc(_T("TxtFuncWebFilter.dll")){}
public:
	virtual ~CGrTxtFuncWebFilter()
	{
		Unregister();
	}
	bool Register()
	{
		if(!m_fRegister){
			SetProcPtr(m_fRegister, GetProcAddress("TxtFuncWebFilterRegister", NULL));
			if(!m_fRegister){
				return false;
			}
		}
		auto &&param = CGrTxtMiru::theApp().Param();
		m_fRegister(CGrTxtMiru::GetDataPath(), &param);
		return true;
	}
	bool Unregister()
	{
		if(m_hDLL){
			if(!m_fUnregister){
				SetProcPtr(m_fUnregister, GetProcAddress("TxtFuncWebFilterUnregister", NULL));
				if(!m_fUnregister){
					return false;
				}
			}
			m_fUnregister(CGrStdWin::GetWnd(), WebFilterTimerProc);
		}
		return true;
	}
	virtual void Unload()
	{
		if(m_hDLL){
			FreeLibrary(m_hDLL);
		}
		m_hDLL        = NULL;
		m_fRegister   = nullptr;
		m_fUnregister = nullptr;
	}

	static CGrTxtFuncWebFilter &WebFilter()
	{
		static CGrTxtFuncWebFilter webfilter;
		return webfilter;
	}
	static void CALLBACK WebFilterTimerProc(
		HWND hwnd,         // ウィンドウのハンドル
		UINT uMsg,         // WM_TIMER メッセージ
		UINT_PTR idEvent,  // タイマの識別子
		DWORD dwTime       // 現在のシステム時刻
		)
	{
		WebFilter().Unload();
	}
};

CGrWebFilter::CGrWebFilter()
{
	auto &&param = CGrTxtMiru::theApp().Param();
	if(param.GetBoolean(CGrTxtParam::PointsType::WebFilter)){
		CGrTxtFuncWebFilter::WebFilter().Register();
	}
}

CGrWebFilter::~CGrWebFilter()
{
	CGrTxtFuncWebFilter::WebFilter().Unregister();
}

