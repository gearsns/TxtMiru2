#include <windows.h>
#include "JScript.h"
#include "TxtMiru.h"
#import <msscript.ocx> raw_interfaces_only, named_guids, no_namespace

CGrJScript::CGrJScript()
{
}

CGrJScript::~CGrJScript()
{
	if(m_pScriptCtrl){
		auto* pScriptCtrl = static_cast<IScriptControl*>(m_pScriptCtrl);
		pScriptCtrl->Reset();
		pScriptCtrl->Release();
		m_pScriptCtrl = nullptr;
	}
}

static bool create(IScriptControl **ppScriptCtrl)
{
	if(FAILED(CoCreateInstance(CLSID_ScriptControl, NULL, CLSCTX_ALL, IID_IScriptControl, reinterpret_cast<void**>(ppScriptCtrl)))){
		*ppScriptCtrl = nullptr;
		return false;
	}
	auto* pScriptCtrl = static_cast<IScriptControl*>(*ppScriptCtrl);
	if(FAILED(pScriptCtrl->put_Language(_bstr_t("JScript")))){
		return false;
	}
	if(FAILED(pScriptCtrl->put_Timeout(-1))){
		return false;
	}
	if(FAILED(pScriptCtrl->put_AllowUI(VARIANT_FALSE))){
		return false;
	}
	return true;
}
bool CGrJScript::Run(std::tstring &outstr, LPCTSTR funcName, LPCTSTR args[], int num)
{
	bool ret = false;
	LPSAFEARRAY psa = nullptr;
	do {
		SAFEARRAYBOUND rgsabound[]  = { static_cast<ULONG>(num), 0 }; // 0 elements, 0-based
		psa = ::SafeArrayCreate(VT_VARIANT, 1, rgsabound);
		if(!psa){
			break;
		}
		HRESULT hr = 0;
		for(LONG lArgNo=0; lArgNo<num; ++lArgNo){
			_variant_t var(args[lArgNo]);
			hr = ::SafeArrayPutElement(psa, &lArgNo, &var);
			if(FAILED(hr)){
				break;
			}
		}
		if(FAILED(hr)){
			break;
		}
		auto* pScriptCtrl = static_cast<IScriptControl*>(m_pScriptCtrl);
		_variant_t outpar;
		ret = SUCCEEDED(pScriptCtrl->Run(_bstr_t(funcName), &psa, &outpar));
		if(!ret){
			const auto &param = CGrTxtMiru::theApp().Param();
			int usePreParser[2] = {}; /* 0:Pre-Parserを使用する, 1:Pre-Parserのエラーを表示しない */
			param.GetPoints(CGrTxtParam::UsePreParser, usePreParser, sizeof(usePreParser)/sizeof(int));
			if(usePreParser[1] != 1){
				IScriptError *pse = nullptr;
				if(SUCCEEDED(pScriptCtrl->get_Error(&pse)) && pse){
					long line = 0;
					long column = 0;
					pse->get_Line(&line);
					pse->get_Column(&column);
					BSTR bstr;
					pse->get_Description(&bstr);
					_bstr_t bstrt(bstr);
					if(static_cast<TCHAR*>(bstrt)){
						std::tstring description(bstrt);
						TCHAR buf[2048];
						_stprintf_s(buf, _T("%s\nLine:%d\nColumn:%d\n"), description.c_str(), line, column);
						::MessageBox(NULL, buf, _T("Script Error"), MB_OK);
					}
					SysFreeString(bstr);
				}
			}
		}
		if(!ret || outpar.vt == VT_NULL){
			ret = false;
			break;
		}
		outstr = static_cast<TCHAR*>(_bstr_t(outpar));
	} while(0);
	if(psa){
		::SafeArrayDestroy(psa);
	}
	return ret;
}
bool CGrJScript::Load(LPCTSTR pszFileName)
{
	if(!m_pScriptCtrl){
		if(!create(reinterpret_cast<IScriptControl**>(&m_pScriptCtrl))){
			return false;
		}
	}
	if(!m_pScriptCtrl){
		return false;
	}
	bool ret = false;
	HANDLE hFile = NULL;

	HANDLE hBuf  = NULL;
	LPBYTE data = nullptr;
	do {
		hFile = ::CreateFile(pszFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
		if(hFile == INVALID_HANDLE_VALUE){
			break;
		}
		auto dwFileSize = ::GetFileSize(hFile , nullptr);
		if(dwFileSize <= 0){
			break;
		}
		hBuf = ::HeapCreate(0, 0, 0);
		if(!hBuf){ break; }
		data = static_cast<BYTE*>(::HeapAlloc(hBuf, HEAP_ZERO_MEMORY, sizeof(TCHAR)*(dwFileSize+1)));
		if(!data){ break; }
		DWORD dwReadSize;
		if(::ReadFile(hFile, data, dwFileSize, &dwReadSize, nullptr)){
			auto* pScriptCtrl = static_cast<IScriptControl*>(m_pScriptCtrl);
			pScriptCtrl->Reset();
			pScriptCtrl->AddCode(_bstr_t(reinterpret_cast<LPCTSTR>(data)));
			ret = true;
		}
	} while(0);
	if(hFile){
		::CloseHandle(hFile);
	}
	if(hBuf){
		if(data){
			::HeapFree(hBuf, 0, data);
		}
		::HeapDestroy(hBuf);
	}
	return ret;
}
