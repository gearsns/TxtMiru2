#pragma warning( disable : 4786 )

#include "stdafx.h"

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <Objbase.h>
#include "psapi.h"
#include "atlbase.h"
#include "comutil.h"
#include "resource.h"
#include "TxtFuncBrowser.h"
#include <DDEML.H>
#include <tchar.h>
#include <string>

#pragma comment(lib, "Oleacc.lib")
#pragma comment(lib, "psapi.lib ")
#pragma comment(lib, "comsuppw.lib")

static HDDEDATA CALLBACK DdeCallback(UINT uType, UINT uFmt, HCONV hconv, HSZ hsz1, HSZ hsz2, HDDEDATA hdata, DWORD dwData1, DWORD dwData2)
{
	return NULL;
}
static LPTSTR getURLinDDE(LPCTSTR browser)
{
	DWORD dwDDEID = 0;
	HSZ hszService = NULL;
	HSZ hszTopic   = NULL;
	HSZ hszParam   = NULL;
	HCONV hConv = NULL;

	TCHAR *result = nullptr;

	do {
		if(DdeInitialize(&dwDDEID, DdeCallback, APPCMD_CLIENTONLY, 0L) != DMLERR_NO_ERROR){
			break;
		}
		//ブラウザを列挙する
		hszService = DdeCreateStringHandle(dwDDEID, browser                , CP_WINUNICODE);
		hszTopic   = DdeCreateStringHandle(dwDDEID, _T("WWW_GetWindowInfo"), CP_WINUNICODE);
		hszParam   = DdeCreateStringHandle(dwDDEID, _T("0xFFFFFFFF"       ), CP_WINUNICODE);
		//DDE接続
		hConv = DdeConnect(dwDDEID, hszService, hszTopic, nullptr);
		if(!hConv){
			break;
		}
		//トランザクションの開始
		auto hDDEData = DdeClientTransaction(nullptr, 0, hConv, hszParam, CF_TEXT/*CF_UNICODETEXT*/, XTYP_REQUEST, 10000L, nullptr);
		if(!hDDEData){
			break;
		}
		auto dwData = DdeGetData(hDDEData, nullptr, 0, 0);
		if(dwData <= 0){
			break;
		}
		auto tmpbuf = static_cast<LPSTR>(GlobalAlloc(GPTR, (dwData+1)*(sizeof(char))));
		if (!tmpbuf) {
			DdeFreeDataHandle(hDDEData);
			break;
		}
		/* */result = static_cast<TCHAR *>(GlobalAlloc(GPTR, (dwData+1)*(sizeof(TCHAR))));
		if (!result) {
			DdeFreeDataHandle(hDDEData);
			GlobalFree(tmpbuf);
			break;
		}
		//データの取得
		DdeGetData(hDDEData, reinterpret_cast<LPBYTE>(tmpbuf), dwData, 0);
		// URLは、"で囲まれている ["URL","TITLE"]
		auto pStart = tmpbuf;
		auto pTemp = strchr(pStart, '"');
		if(pTemp){
			++pTemp;
			if(*pTemp){
				auto pEnd = strchr(pTemp, '"');
				if(pEnd){
					*pEnd = '\0';
					pStart = pTemp;
				}
			}
		}
		// ウィンドウに取得した情報を送信
		auto nLen = ::MultiByteToWideChar(CP_THREAD_ACP, 0, pStart, -1, nullptr, 0);
		if(nLen != 0){
			::MultiByteToWideChar(CP_THREAD_ACP, 0, pStart, -1, result, nLen);
		}
		//解放
		DdeFreeDataHandle(hDDEData);
		//解放
		GlobalFree(tmpbuf);
	} while(0);

	if(hConv){ DdeDisconnect(hConv); }

	if(hszService){ DdeFreeStringHandle(dwDDEID, hszParam  ); }
	if(hszTopic  ){ DdeFreeStringHandle(dwDDEID, hszTopic  ); }
	if(hszService){ DdeFreeStringHandle(dwDDEID, hszService); }

	if(dwDDEID){ DdeUninitialize(dwDDEID); }

	return result;
}

static std::wstring GetName(IAccessible *pAcc)
{
	CComBSTR bstrName;
	if(!pAcc || FAILED(pAcc->get_accName(CComVariant(static_cast<int>(CHILDID_SELF)), &bstrName)) || !bstrName.m_str){
		return L"";
	}
	if (bstrName.m_str == nullptr) {
		return L"";
	}
	else {
		return bstrName.m_str;
	}
}
static std::wstring GetValue(IAccessible *pAcc)
{
	CComBSTR bstrName;
	if(!pAcc || FAILED(pAcc->get_accValue(CComVariant(static_cast<int>(CHILDID_SELF)), &bstrName)) || !bstrName.m_str){
		return L"";
	}
	if (bstrName.m_str == nullptr) {
		return L"";
	}
	else {
		return bstrName.m_str;
	}
}
struct URLDataInfo
{
	HWND hwnd;
	LPCTSTR browser;
	std::wstring url;
};

VARIANT CreateI4Variant(LONG value) {
  VARIANT variant = {};
  V_VT(&variant) = VT_I4;
  V_I4(&variant) = value;
  return variant;
}
static HRESULT WalkTreeWithAccessibleChildren(CComPtr<IAccessible> pAcc, URLDataInfo &data)
{
	long childCount = 0;
	long returnCount = 0;

	auto hr = pAcc->get_accChildCount(&childCount);
	if (childCount == 0){
		return S_OK;
	}
	auto* pArray = new CComVariant[childCount];
	hr = ::AccessibleChildren(pAcc, 0L, childCount, pArray, &returnCount);
	if (FAILED(hr)){
		return hr;
	}
	for (int x = 0; x < returnCount; x++)
	{
		CComVariant vtChild = pArray[x];
		if (vtChild.vt != VT_DISPATCH)
			continue;
		
		CComPtr<IDispatch> pDisp = vtChild.pdispVal;
		CComQIPtr<IAccessible> pAccChild(pDisp);
		if (!pAccChild)
			continue;
		std::wstring name = GetName(pAccChild).data();
		std::wstring url = ::GetValue(pAccChild).data();
		if(/**/ 0 == url.find(_T("http://"))
		   ||   0 == url.find(_T("https://"))
		   ||   0 == url.find(_T("file://"))){
			CComVariant var_role = {};
			pAcc->get_accRole(CreateI4Variant(CHILDID_SELF), &var_role);// ROLE_SYSTEM_TEXT
			TCHAR role_str[2048];
			if(var_role.vt == VT_BSTR){
				USES_CONVERSION;
				lstrcpy(role_str, OLE2T(var_role.bstrVal));
				// browser + STATE_SYSTEM_FOCUSABLE
			} else {
				GetRoleText(var_role.lVal, role_str, _countof(role_str));
				// group
			}
			CComVariant var_state = {};
			pAcc->get_accState(CreateI4Variant(CHILDID_SELF), &var_state);// ROLE_SYSTEM_TEXT
			TCHAR state_str[2048];
			if(var_state.vt == VT_BSTR){
				USES_CONVERSION;
				lstrcpy(state_str, OLE2T(var_state.bstrVal));
			} else {
				GetStateText(var_state.lVal, state_str, _countof(state_str));
			}
			if(var_role.lVal == ROLE_SYSTEM_GROUPING){
				// chrome  Chrome_WidgetWin_1
				data.url = url;
				return S_FALSE;
			} else if(var_role.lVal == ROLE_SYSTEM_WINDOW && var_state.lVal == STATE_SYSTEM_FOCUSABLE){
				// egdge   Internet Explorer_Server Windows.UI.Core.CoreWindow ApplicationFrameWindow
				data.url = url;
				return S_FALSE;
			} else if((lstrcmp(role_str, _T("group")) == 0
					   || lstrcmp(role_str, _T("browser")) == 0)
					  && var_state.lVal == STATE_SYSTEM_FOCUSABLE){
				// firefox MozillaWindowClass
				// egdge   Internet Explorer_Server Windows.UI.Core.CoreWindow ApplicationFrameWindow
				data.url = url;
				return S_FALSE;
			} else {
			}
		}
		if(WalkTreeWithAccessibleChildren(pAccChild, data) == S_FALSE){
			return S_FALSE;
		}
	}

	delete[] pArray;
	return S_OK;
}
static BOOL CALLBACK EnumChildProc(
	HWND hwnd,      // 子ウィンドウのハンドル
	LPARAM lParam   // アプリケーション定義の値
	)
{
	auto *pData = reinterpret_cast<URLDataInfo*>(lParam);
	TCHAR classname[2048];
	if(GetClassName(hwnd, classname, _countof(classname))){
		if(lstrcmp(classname, pData->browser) == 0){
			DWORD dwProcessId = 0;
			GetWindowThreadProcessId(hwnd, &dwProcessId);
			auto h = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwProcessId);
			TCHAR filename[2048];
			GetModuleFileNameEx(h, 0, filename, _countof(filename));
			if(_tcsstr(filename, _T("thunderbird.exe"))){
				return TRUE;
			}
			TCHAR buf[2048];
			GetWindowText(hwnd, buf, _countof(buf));
			pData->hwnd = hwnd;
			CComPtr<IAccessible> ac1;
			::AccessibleObjectFromWindow(hwnd, 1, IID_IAccessible, reinterpret_cast<void**>(&ac1));
			CComPtr<IAccessible> ac2;
			::AccessibleObjectFromWindow(hwnd, OBJID_CLIENT, IID_IAccessible, reinterpret_cast<void**>(&ac2));
			std::wstring url = ::GetName(ac2).data();
			if(ac2){
				::WalkTreeWithAccessibleChildren(ac2, *pData);
			}
			if(!pData->url.empty()){
				return FALSE;
			}
		}
	}
	return TRUE;
}

static LPTSTR getURLinWindow(LPCTSTR browser)
{
	if (::CoInitialize(NULL) == S_OK) {
		;
	}
	URLDataInfo data = { NULL, browser };
	EnumChildWindows(NULL, EnumChildProc, reinterpret_cast<LPARAM>(&data));
	if(!data.url.empty()){
		auto *result = static_cast<TCHAR *>(GlobalAlloc(GPTR, (data.url.size()+1)*(sizeof(TCHAR))));
		if(result){
			lstrcpy(result, data.url.c_str());
			return result;
		}
	}
	::CoUninitialize();
	return nullptr;
}
LPTSTR getURLinBrowser(LPCTSTR browser)
{
	auto lpURL = getURLinWindow(browser);
	if(!lpURL){
		lpURL = getURLinDDE(browser);
	}
	return lpURL;
}
