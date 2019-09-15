#include <iostream>
#include <windows.h>
#include <memory>
#import <msscript.ocx> raw_interfaces_only, named_guids, no_namespace
#include <AtlConv.h>
#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <regex>
#include "tchar.h"
#include <dispex.h>
#include "shell.h"
#include "stlutil.h"
#include "TxtMiru.h"
#include "JScript.h"
#include "CurrentDirectory.h"
#include "BookMarkDB.h"
#include <filesystem>

extern void PostWokerMessage(LPCTSTR message);
extern bool PostWorkerQueue(LPCTSTR lpUrl);

class CGrJScript
{
public:
	CGrJScript();
	virtual ~CGrJScript();
	bool Run(std::tstring &outstr, LPCTSTR funcName, LPCTSTR args[], int num);
	bool Load(LPCTSTR pszFileName);
protected:
	void *m_pScriptCtrl = nullptr;
};

class TxtMiruScriptObject : public IDispatch
{
private:
	IScriptControl *m_pScriptControl = nullptr;
	LONG m_cRef = 0;

	// method type
	using Func = HRESULT(TxtMiruScriptObject::*)(DISPPARAMS*, VARIANT*);

	// method structure
	struct TXTMIRU_OBJECT_METHOD_TABLE {
		DISPID id;
		const char* name;
		Func fn;
	};

	// method table
	TXTMIRU_OBJECT_METHOD_TABLE m_ObjectMethodTable[11] = {
		{1, "ShowError", &TxtMiruScriptObject::ShowError},
		{2, "GetCachePath", &TxtMiruScriptObject::GetCachePath},
		{3, "PutCachePath", &TxtMiruScriptObject::PutCachePath},
		{4, "CacheOpen", &TxtMiruScriptObject::CacheOpen},
		{5, "CacheAdd", &TxtMiruScriptObject::CacheAdd},
		{6, "GetAttachDir", &TxtMiruScriptObject::GetAttachDir},
		{7, "CacheClose", &TxtMiruScriptObject::CacheClose},
		{8, "GetCacheState", &TxtMiruScriptObject::GetCacheState},
		{9, "PostLineMessage", &TxtMiruScriptObject::PostLineMessage},
		{10, "PushDownloadQueue", &TxtMiruScriptObject::PushDownloadQueue},

	};

	HRESULT map2JSHash(VARIANT &out_ret, const std::map<std::tstring, std::tstring> &data) {
		IDispatch *pdispObject;
		if (FAILED(m_pScriptControl->get_CodeObject(&pdispObject))) {
			return S_FALSE;
		}
		DISPID did = 0;
		LPOLESTR lpNames[] = { A2BSTR("Object") };
		if (FAILED(pdispObject->GetIDsOfNames(IID_NULL, lpNames, 1, LOCALE_USER_DEFAULT, &did))) {
			return S_FALSE;
		}
		DISPPARAMS params = {};
		if (FAILED(pdispObject->Invoke(did, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &params, &out_ret, nullptr, nullptr))) {
			return S_FALSE;
		}
		DISPPARAMS no_params = { nullptr,nullptr,0,0 };
		IDispatchEx *pDEX = nullptr;
		auto hr1 = out_ret.pdispVal->QueryInterface(__uuidof(IDispatchEx), (void**)&pDEX);
		if (FAILED(hr1)) {
			return S_FALSE;
		}
		DISPID propid;
		DISPID disp_propput = DISPID_PROPERTYPUT;
		for (const auto &item : data) {
			pDEX->GetDispID(_bstr_t(item.first.c_str()), fdexNameEnsure, &propid);
			_variant_t arg1(item.second.c_str());
			DISPPARAMS put_prop_parms = { &arg1,&disp_propput,1,1 };
			pDEX->InvokeEx(propid, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYPUT, &put_prop_parms, nullptr, nullptr, nullptr);
		}
		return S_OK;
	}
	HRESULT mapObj2JSHash(VARIANT &out_ret, const std::map<std::tstring, _variant_t> &data) {
		IDispatch *pdispObject;
		if (FAILED(m_pScriptControl->get_CodeObject(&pdispObject))) {
			return S_FALSE;
		}
		DISPID did = 0;
		LPOLESTR lpNames[] = { A2BSTR("Object") };
		if (FAILED(pdispObject->GetIDsOfNames(IID_NULL, lpNames, 1, LOCALE_USER_DEFAULT, &did))) {
			return S_FALSE;
		}
		DISPPARAMS params = {};
		if (FAILED(pdispObject->Invoke(did, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &params, &out_ret, nullptr, nullptr))) {
			return S_FALSE;
		}
		DISPPARAMS no_params = { nullptr,nullptr,0,0 };
		IDispatchEx *pDEX = nullptr;
		auto hr1 = out_ret.pdispVal->QueryInterface(__uuidof(IDispatchEx), (void**)&pDEX);
		if (FAILED(hr1)) {
			return S_FALSE;
		}
		DISPID propid;
		DISPID disp_propput = DISPID_PROPERTYPUT;
		for (const auto &item : data) {
			pDEX->GetDispID(_bstr_t(item.first.c_str()), fdexNameEnsure, &propid);
			_variant_t arg1(item.second);
			DISPPARAMS put_prop_parms = { &arg1,&disp_propput,1,1 };
			pDEX->InvokeEx(propid, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYPUTREF, &put_prop_parms, nullptr, nullptr, nullptr);
		}
		return S_OK;
	}
	static void makeFolderPath(std::tstring &folder, std::initializer_list<LPCTSTR> args) {
		for (const auto &item : args) {
			if (folder.size() > 0 && !CGrShell::IsBackSlash(folder.back())) {
				folder += L"/";
			}
			folder += item;
		}
		CGrShell::ToPrettyFileName(folder);
	}
	HRESULT PutIDispatchProperty(IDispatch* iDisp, const char* name, const wchar_t *val) {
		HRESULT hr;

		DISPPARAMS no_params = { nullptr,nullptr,0,0 };
		IDispatchEx *pDEX = nullptr;
		auto hr1 = iDisp->QueryInterface(__uuidof(IDispatchEx), (void**)&pDEX);
		if (FAILED(hr1)) {
			return S_FALSE;
		}
		DISPID propid;
		DISPID disp_propput = DISPID_PROPERTYPUT;
		pDEX->GetDispID(_bstr_t(name), fdexNameEnsure, &propid);
		_variant_t arg1(val);
		DISPPARAMS put_prop_parms = { &arg1,&disp_propput,1,1 };
		hr = pDEX->InvokeEx(propid, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYPUT, &put_prop_parms, nullptr, nullptr, nullptr);
		return hr;
	}
	_variant_t GetIDispatchProperty(IDispatch* iDisp, const char* name) {
		HRESULT hr;

		DISPID dispid = 0;
		OLECHAR FAR* szMember = A2BSTR(name);
		hr = iDisp->GetIDsOfNames(IID_NULL, &szMember, 1, LOCALE_SYSTEM_DEFAULT, &dispid);
		if (FAILED(hr)) {
			//throw exception("IDispatch::GetIDsOfNames");
		}

		DISPPARAMS params = { nullptr, nullptr, 0, 0 };
		_variant_t var;
		EXCEPINFO excepinfo;
		UINT nArgErr;
		hr = iDisp->Invoke(dispid, IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_PROPERTYGET, &params, &var, &excepinfo, &nArgErr);
		if (FAILED(hr)) {
			//throw exception("IDispatch::Invoke");
		}

		return var;
	}
	int GetArrayLength(IDispatch* iDispatch) {
		auto length = GetIDispatchProperty(iDispatch, "length");
		length.ChangeType(VT_I4);
		return length.intVal;
	}
	std::vector<_variant_t> GetArrayValues(IDispatch* iDispatch) {
		auto length = GetArrayLength(iDispatch);
		std::vector<_variant_t> result(length);
		for (int n = 0; n < length; ++n) {
			char str[10];
			::sprintf_s(str, "%d", n);
			result[n] = GetIDispatchProperty(iDispatch, str);
		}
		return result;
	}
public:
	// method:
	HRESULT PostLineMessage(DISPPARAMS* pDispParams, VARIANT* ret) {
		USES_CONVERSION;

		for (UINT n = 0; n < pDispParams->cArgs; n++) {
			_variant_t arg;
			auto hr = VariantChangeType(&arg, &pDispParams->rgvarg[n], 0, VT_BSTR);
			PostWokerMessage(OLE2T(arg.bstrVal));
		}
		if (ret) {
			VariantInit(ret);
			V_VT(ret) = VT_BOOL;
			V_BOOL(ret) = VARIANT_TRUE;
		}
		return S_OK;
	}
	HRESULT PushDownloadQueue(DISPPARAMS* pDispParams, VARIANT* ret) {
		USES_CONVERSION;

		for (UINT n = 0; n < pDispParams->cArgs; n++) {
			_variant_t arg;
			auto hr = VariantChangeType(&arg, &pDispParams->rgvarg[n], 0, VT_BSTR);
			PostWorkerQueue(OLE2T(arg.bstrVal));
		}
		if (ret) {
			VariantInit(ret);
			V_VT(ret) = VT_BOOL;
			V_BOOL(ret) = VARIANT_TRUE;
		}
		return S_OK;
	}
	// function GetCacheState(url_array) item_hash
	HRESULT GetCacheState(DISPPARAMS* pDispParams, VARIANT* ret) {
		VariantInit(ret);
		V_VT(ret) = VT_BOOL;
		V_BOOL(ret) = VARIANT_FALSE;
		if (pDispParams->cArgs >= 1) {
			const auto &arg = pDispParams->rgvarg[0];
			if (arg.vt == VT_BSTR) {
			}
			else if (arg.vt == VT_ARRAY) {
			}
			else if (arg.vt == VT_DISPATCH) {

				auto arr = GetArrayValues(arg.pdispVal);
				std::vector<std::tstring> url_list;
				for (const auto& item : arr) {
					if (item.vt == VT_DISPATCH) {
						auto type = GetIDispatchProperty(item.pdispVal, "type");
						type.ChangeType(VT_BSTR);
						std::tstring typeStr = OLE2T(type.bstrVal);
						if (typeStr == _T("index")) {
							auto url = GetIDispatchProperty(item.pdispVal, "href");
							url.ChangeType(VT_BSTR);
							url_list.push_back(OLE2T(url.bstrVal));
						}
					}
				}
				std::vector<CacheItem> cache_list;
				if (m_db.Get(cache_list, url_list)) {
					std::map<std::tstring, _variant_t> ret_jshash;
					for (const auto &item : cache_list) {
						std::map<std::tstring, std::tstring> data;
						data[L"insert_date"] = item.insert_date;
						data[L"update_date"] = item.update_date;
						data[L"author"] = item.author;
						data[L"title"] = item.title;
						map2JSHash(ret_jshash[item.url], data);
					}
					mapObj2JSHash(*ret, ret_jshash);
				}
			}
			else {
			}
		}
		if (ret) {
		}
		fflush(stderr);
		return S_OK;
	}
	//
	HRESULT ShowError(DISPPARAMS* pDispParams, VARIANT* ret) {
		USES_CONVERSION;

		for (UINT n = 0; n < pDispParams->cArgs; n++) {
			_variant_t arg;
			auto hr = VariantChangeType(&arg, &pDispParams->rgvarg[n], 0, VT_BSTR);
			MessageBox(0, OLE2T(arg.bstrVal), _T("MyObject"), MB_OK);
		}
		if (ret) {
			VariantInit(ret);
			V_VT(ret) = VT_BOOL;
			V_BOOL(ret) = VARIANT_TRUE;
		}
		return S_OK;
	}
	CGrDBCache m_db;
	CacheItem m_item;
	std::tstring m_cacheDir;
	std::tstring m_cacheAttachDir;
	int m_id = 0;
	std::tstring m_url;
	std::tstring m_folder;
	std::tstring m_filename;
	HRESULT GetAttachDir(DISPPARAMS* pDispParams, VARIANT* ret) {
		if (ret) {
			std::tstring path = m_filename;
			path += L"_files";
			VariantInit(ret);
			V_VT(ret) = VT_BSTR;
			V_BSTR(ret) = ::SysAllocString(W2OLE(const_cast<LPWSTR>(path.c_str())));
		}
		return S_OK;
	}
	// function CacheAdd(text)
	// function CacheAdd(url, data)
	HRESULT CacheAdd(DISPPARAMS* pDispParams, VARIANT* ret) {
		USES_CONVERSION;

		bool bRet = false;
		if (pDispParams->cArgs == 1) {
			do {
				if (m_cacheDir.size() == 0) {
					break;
				}
				const auto &text = pDispParams->rgvarg[0];
				if (text.vt == VT_BSTR) {
					std::tstring filename = m_cacheDir;
					filename += L"/";
					filename += m_filename;
					filename += L".html";
					auto savefilename = std::regex_replace(filename.c_str(), std::wregex(L"%DATADIR%"), CGrTxtMiru::GetDataPath(), std::regex_constants::format_first_only);
					fflush(stderr);
					auto hFile = CreateFile(savefilename.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, 0);
					if (hFile == INVALID_HANDLE_VALUE) {
						break;
					}
					std::string utf8_str;
					std::w2utf8(OLE2T(text.bstrVal), utf8_str);
					DWORD dw;
					WriteFile(hFile, utf8_str.c_str(), utf8_str.size(), &dw, nullptr);
					CloseHandle(hFile);

					m_db.Put(m_url.c_str(), filename.c_str(), L"", L"");
				}
				bRet = true;
			} while (0);
		}
		else if (pDispParams->cArgs == 2) {
			do {
				if (m_cacheDir.size() == 0) {
					break;
				}
				std::tstring url;
				std::tstring filename;
				{
					_variant_t buf;
					if (VariantChangeType(&buf, &pDispParams->rgvarg[1], 0, VT_BSTR) == S_OK) {
						url = OLE2T(buf.bstrVal);
						filename = m_cacheAttachDir;
						filename += L"/";
						filename += CGrShell::GetFileName(const_cast<TCHAR*>(url.c_str()));
					}
				}
				const auto &data = pDispParams->rgvarg[0];
				if (data.vt == (VT_ARRAY | VT_UI1)) {
					auto psa = V_ARRAY(&data);
					if (SafeArrayGetDim(psa) != 1) {
						break;
					}
					LONG lLBound;
					auto hr = SafeArrayGetLBound(psa, 1, &lLBound);
					if (FAILED(hr))
					{
						break;
					}
					LONG lUBound;
					hr = SafeArrayGetUBound(psa, 1, &lUBound);
					if (FAILED(hr))
					{
						break;
					}
					BYTE *pdata;
					hr = SafeArrayAccessData(psa, (LPVOID*)&pdata);
					if (FAILED(hr))
					{
						break;
					}
					auto cElements = lUBound - lLBound + 1;
					auto savefilename = std::regex_replace(filename.c_str(), std::wregex(L"%DATADIR%"), CGrTxtMiru::GetDataPath(), std::regex_constants::format_first_only);
					auto hFile = CreateFile(savefilename.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, 0);
					if (hFile == INVALID_HANDLE_VALUE) {
						break;
					}
					DWORD dw;
					WriteFile(hFile, pdata, cElements, &dw, nullptr);
					CloseHandle(hFile);

					SafeArrayUnaccessData(psa);

					m_db.Put(m_item, url.c_str(), filename.c_str());
				}
				else if (data.vt == VT_BSTR) {
					auto savefilename = std::regex_replace(filename.c_str(), std::wregex(L"%DATADIR%"), CGrTxtMiru::GetDataPath(), std::regex_constants::format_first_only);
					auto hFile = CreateFile(savefilename.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, 0);
					if (hFile == INVALID_HANDLE_VALUE) {
						break;
					}
					std::string utf8_str;
					std::w2utf8(OLE2T(data.bstrVal), utf8_str);
					DWORD dw;
					WriteFile(hFile, utf8_str.c_str(), utf8_str.size(), &dw, nullptr);
					CloseHandle(hFile);
					m_db.Put(m_item, url.c_str(), filename.c_str());
				}
				bRet = true;
			} while (0);
		}
		else if (pDispParams->cArgs == 3) {
			do {
				if (m_cacheDir.size() == 0) {
					break;
				}
				std::vector<std::tstring> args;
				for (int i = pDispParams->cArgs - 1; i >= 0; --i) {
					_variant_t buf;
					if (VariantChangeType(&buf, &pDispParams->rgvarg[i], 0, VT_BSTR) == S_OK) {
						args.push_back(OLE2T(buf.bstrVal));
					}
				}
				const auto &title = args[0];
				const auto &author = args[1];
				const auto &html = args[2];
				std::tstring filename = m_cacheDir;
				filename += L"/";
				filename += m_filename;
				filename += L".html";
				auto savefilename = std::regex_replace(filename.c_str(), std::wregex(L"%DATADIR%"), CGrTxtMiru::GetDataPath(), std::regex_constants::format_first_only);
				auto hFile = CreateFile(savefilename.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, 0);
				if (hFile == INVALID_HANDLE_VALUE) {
					break;
				}
				std::string utf8_str;
				std::w2utf8(html.c_str(), utf8_str);
				DWORD dw;
				WriteFile(hFile, utf8_str.c_str(), utf8_str.size(), &dw, nullptr);
				CloseHandle(hFile);

				m_db.Put(m_url.c_str(), filename.c_str(), title.c_str(), author.c_str());
				bRet = true;
			} while (0);
		}


		if (ret) {
			VariantInit(ret);
			V_VT(ret) = VT_BOOL;
			V_BOOL(ret) = bRet ? VARIANT_TRUE : VARIANT_FALSE;
		}
		return S_OK;
	}
	// function CacheOpen(url, folder, filename)
	HRESULT CacheOpen(DISPPARAMS* pDispParams, VARIANT* ret) {
		USES_CONVERSION;

		bool bRet = false;
		if (pDispParams->cArgs == 3) {
			do {
				std::vector<std::tstring> args;
				for (int i = pDispParams->cArgs - 1; i >= 0; --i) {
					_variant_t buf;
					if (VariantChangeType(&buf, &pDispParams->rgvarg[i], 0, VT_BSTR) == S_OK) {
						args.push_back(OLE2T(buf.bstrVal));
					}
				}
				m_url = args[0];
				m_folder = args[1];
				m_filename = args[2];
				std::tstring path;
				makeFolderPath(path, { L"%DATADIR%/Cache/", m_folder.c_str(), m_filename.c_str() });
				// FolderçÏê¨
				std::unique_ptr<TCHAR> cache_folder_tmp(new TCHAR[path.size() + 1]);
				auto cache_folder = cache_folder_tmp.get();
				path.copy(cache_folder, path.size());
				cache_folder[path.size()] = '\0';
				CGrShell::GetParentDir(cache_folder, nullptr);
				auto len = _tcsclen(cache_folder);
				if (len > 0 && CGrShell::IsBackSlash(cache_folder[len - 1])) {
					cache_folder[len - 1] = '\0';
				}
				m_cacheDir = cache_folder;
				m_cacheAttachDir = m_cacheDir + L"/" + m_filename + L"_files";
				auto cache_dir = std::regex_replace(m_cacheDir.c_str(), std::wregex(L"%DATADIR%"), CGrTxtMiru::GetDataPath(), std::regex_constants::format_first_only);
				if (!CGrShell::GetSearchDir(cache_dir.c_str())) {
					if (!CGrShell::CreateFolder(cache_dir.c_str())) {
						break;
					}
				}
				auto cache_attach_dir = std::regex_replace(m_cacheAttachDir.c_str(), std::wregex(L"%DATADIR%"), CGrTxtMiru::GetDataPath(), std::regex_constants::format_first_only);
				if (!CGrShell::GetSearchDir(cache_attach_dir.c_str())) {
					if (!CGrShell::CreateFolder(cache_attach_dir.c_str())) {
						break;
					}
				}
				m_db.Open();
				m_db.BeginSession();
				m_db.Put(m_url.c_str(), L"", L"", L"");
				m_db.Get(m_item, m_url.c_str());
				bRet = true;
			} while (0);
		}
		if (ret) {
			VariantInit(ret);
			V_VT(ret) = VT_BOOL;
			V_BOOL(ret) = bRet ? VARIANT_TRUE : VARIANT_FALSE;
		}
		return S_OK;
	}
	HRESULT CacheClose(DISPPARAMS* pDispParams, VARIANT* ret) {
		USES_CONVERSION;

		bool bRet = true;
		m_db.Commit();
		if (ret) {
			VariantInit(ret);
			V_VT(ret) = VT_BOOL;
			V_BOOL(ret) = bRet ? VARIANT_TRUE : VARIANT_FALSE;
		}
		return S_OK;
	}
	HRESULT GetCachePath(DISPPARAMS* pDispParams, VARIANT* ret) {
		USES_CONVERSION;

		if (pDispParams->cArgs == 1) {
			do {
				_variant_t arg;
				auto hr = VariantChangeType(&arg, &pDispParams->rgvarg[0], 0, VT_BSTR);
				std::tstring path;
			} while (0);
		}
		if (ret) {
			VariantInit(ret);
			V_VT(ret) = VT_BOOL;
			V_BOOL(ret) = VARIANT_FALSE;
		}
		return S_OK;
	}
	HRESULT PutCachePath(DISPPARAMS* pDispParams, VARIANT* ret) {
		USES_CONVERSION;

		if (pDispParams->cArgs == 2) {
			do {
			} while (0);
		}
		if (ret) {
			VariantInit(ret);
			V_VT(ret) = VT_BOOL;
			V_BOOL(ret) = VARIANT_FALSE;
		}
		return S_OK;
	}

	// constructor
	TxtMiruScriptObject(IScriptControl *pScriptControl) : m_pScriptControl(pScriptControl) {
	}

	STDMETHODIMP QueryInterface(REFIID rid, LPVOID *ppv) {
		*ppv = nullptr;
		if (::IsEqualIID(rid, IID_IUnknown)) {
			*ppv = this;
			AddRef();
			return S_OK;
		}
		else if (::IsEqualIID(rid, IID_IDispatch)) {
			*ppv = this;
			AddRef();
			return S_OK;
		}
		return E_NOINTERFACE;
	}
	ULONG STDMETHODCALLTYPE AddRef() {
		return InterlockedIncrement(&m_cRef);
	}
	ULONG STDMETHODCALLTYPE Release() {
		return InterlockedDecrement(&m_cRef);
	}
	STDMETHODIMP GetTypeInfoCount(UINT *ptiCount) {
		if (ptiCount) *ptiCount = 0;
		return S_OK;
	}
	STDMETHODIMP GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo **pptInfo) {
		if (pptInfo) *pptInfo = nullptr;
		return S_OK;
	}
	STDMETHODIMP GetIDsOfNames(REFIID riid, OLECHAR **rgszNames, UINT cNames, LCID lcid, DISPID *rgDispID) {
		USES_CONVERSION;

		for (UINT n = 0; n < cNames; n++) {
			rgDispID[n] = 0;
			for (const auto it : m_ObjectMethodTable) {
				bstr_t str(rgszNames[n]);
				if (!strcmp(it.name, str)) {
					rgDispID[n] = it.id;
					break;
				}
			}
		}
		return S_OK;
	}
	STDMETHODIMP Invoke(DISPID dispID, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr) {
		for (const auto it : m_ObjectMethodTable) {
			if (it.id == dispID) {
				return (this->*(it.fn))(pDispParams, pVarResult);
			}
		}
		return S_OK;
	}
};

CGrJScript::CGrJScript()
{
}

CGrJScript::~CGrJScript()
{
	if (m_pScriptCtrl) {
		auto* pScriptCtrl = static_cast<IScriptControl*>(m_pScriptCtrl);
		pScriptCtrl->Reset();
		pScriptCtrl->Release();
		m_pScriptCtrl = nullptr;
	}
}

static bool create(IScriptControl **ppScriptCtrl)
{
	if (FAILED(CoCreateInstance(CLSID_ScriptControl, NULL, CLSCTX_ALL, IID_IScriptControl, reinterpret_cast<void**>(ppScriptCtrl)))) {
		*ppScriptCtrl = nullptr;
		return false;
	}
	auto* pScriptCtrl = static_cast<IScriptControl*>(*ppScriptCtrl);
	if (FAILED(pScriptCtrl->put_Language(_bstr_t("JScript")))) {
		return false;
	}
	if (FAILED(pScriptCtrl->put_Timeout(-1))) {
		return false;
	}
	if (FAILED(pScriptCtrl->put_AllowUI(VARIANT_FALSE))) {
		return false;
	}
	return true;
}

bool CGrJScript::Run(std::tstring &outstr, LPCTSTR funcName, LPCTSTR args[], int num)
{
	bool ret = false;
	LPSAFEARRAY psa = nullptr;
	TxtMiruScriptObject *pScriptObject = nullptr;
	do {
		SAFEARRAYBOUND rgsabound[] = { static_cast<ULONG>(num), 0 }; // 0 elements, 0-based
		psa = ::SafeArrayCreate(VT_VARIANT, 1, rgsabound);
		if (!psa) {
			break;
		}
		HRESULT hr = 0;
		for (LONG lArgNo = 0; lArgNo < num; ++lArgNo) {
			_variant_t var(args[lArgNo]);
			hr = ::SafeArrayPutElement(psa, &lArgNo, &var);
			if (FAILED(hr)) {
				break;
			}
		}
		if (FAILED(hr)) {
			break;
		}
		auto* pScriptCtrl = static_cast<IScriptControl*>(m_pScriptCtrl);
		//
		pScriptObject = new TxtMiruScriptObject(pScriptCtrl);
		hr = pScriptCtrl->AddObject(_bstr_t("TxtMiru"), pScriptObject, VARIANT_TRUE);
		//
		_variant_t outpar;
		ret = SUCCEEDED(pScriptCtrl->Run(_bstr_t(funcName), &psa, &outpar));
		if (!ret) {
			IScriptError *pse = nullptr;
			if (SUCCEEDED(pScriptCtrl->get_Error(&pse)) && pse) {
				long line = 0;
				long column = 0;
				pse->get_Line(&line);
				pse->get_Column(&column);
				BSTR bstr;
				pse->get_Description(&bstr);
				_bstr_t bstrt(bstr);
				if (static_cast<TCHAR*>(bstrt)) {
					std::tstring description(bstrt);
					TCHAR buf[2048];
					_stprintf_s(buf, _T("%s\nLine:%d\nColumn:%d\n"), description.c_str(), line, column);
					::MessageBox(NULL, buf, _T("Script Error"), MB_OK);
				}
				SysFreeString(bstr);
			}
		}
		if (!ret || outpar.vt == VT_NULL) {
			ret = false;
			break;
		}
		if (outpar.vt == VT_BOOL) {
			ret = (outpar.boolVal == VARIANT_TRUE);
			break;
		}
		ret = false;
		if (outpar.vt == VT_BSTR) {
			outstr = static_cast<TCHAR*>(_bstr_t(outpar));
		}
	} while (0);
	if (psa) {
		::SafeArrayDestroy(psa);
	}
	if (pScriptObject) {
		pScriptObject->Release();
	}
	return ret;
}
bool CGrJScript::Load(LPCTSTR pszFileName)
{
	if (!m_pScriptCtrl) {
		if (!create(reinterpret_cast<IScriptControl**>(&m_pScriptCtrl))) {
			return false;
		}
	}
	if (!m_pScriptCtrl) {
		return false;
	}
	bool ret = false;
	HANDLE hFile = NULL;

	HANDLE hBuf = NULL;
	LPBYTE data = nullptr;
	do {
		hFile = ::CreateFile(pszFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
		if (hFile == INVALID_HANDLE_VALUE) {
			break;
		}
		auto dwFileSize = ::GetFileSize(hFile, nullptr);
		if (dwFileSize <= 0) {
			break;
		}
		hBuf = ::HeapCreate(0, 0, 0);
		if (!hBuf) { break; }
		data = static_cast<BYTE*>(::HeapAlloc(hBuf, HEAP_ZERO_MEMORY, sizeof(TCHAR)*(dwFileSize + 1)));
		if (!data) { break; }
		DWORD dwReadSize;
		if (::ReadFile(hFile, data, dwFileSize, &dwReadSize, nullptr)) {
			auto* pScriptCtrl = static_cast<IScriptControl*>(m_pScriptCtrl);
			pScriptCtrl->Reset();
			pScriptCtrl->AddCode(_bstr_t(reinterpret_cast<LPCTSTR>(data)));
			ret = true;
		}
	} while (0);
	if (hFile) {
		::CloseHandle(hFile);
	}
	if (hBuf) {
		if (data) {
			::HeapFree(hBuf, 0, data);
		}
		::HeapDestroy(hBuf);
	}
	return ret;
}

static bool LoadParser(CGrJScript &script, LPCTSTR lpType)
{
	CGrCurrentDirectory cur;
	TCHAR *lpfilepart;
	std::tstring filename(CGrTxtMiru::GetDataPath());
	do {
		if (!filename.empty()) {
			TCHAR foldername[MAX_PATH + 1];
			::GetFullPathName(filename.c_str(), sizeof(foldername) / sizeof(TCHAR), foldername, &lpfilepart);
			CGrShell::AddBackSlash(foldername);
			filename = foldername;
			filename += _T("Cache\\");
			filename += lpType;
			filename += _T(".txt");
			if (CGrShell::GetSearchDir(filename.c_str())) {
				break;
			}
			else {
				filename = foldername;
				filename += _T("Cache\\");
				filename += lpType;
				filename += _T(".js");
				if (CGrShell::GetSearchDir(filename.c_str())) {
					break;
				}
			}
		}
		TCHAR folderName[MAX_PATH];
		CGrShell::GetExePath(folderName);
		if (folderName[0]) {
			TCHAR datapath[MAX_PATH + 1];
			::GetFullPathName(folderName, sizeof(datapath) / sizeof(TCHAR), datapath, &lpfilepart);
			CGrShell::AddBackSlash(datapath);
			filename = datapath;
			filename += _T("Cache\\");
			if (_tcsicmp(folderName, datapath) == 0) {
				return false;
			}
			filename += lpType;
			std::tstring basefilename(filename);
			filename += _T(".txt");
			if (CGrShell::GetSearchDir(filename.c_str())) {
				break;
			}
			else {
				filename = basefilename;
				filename += _T(".js");
				if (CGrShell::GetSearchDir(filename.c_str())) {
					break;
				}
				else {
					return false;
				}
			}
		}
	} while (0);
	return script.Load(filename.c_str());
}

static bool MakeCache(CGrJScript &script, std::tstring &outstr, LPCTSTR lpFileName)
{
	LPCTSTR argv[] = { lpFileName };

	return script.Run(outstr, _T("MakeCache"), argv, sizeof(argv) / sizeof(LPCTSTR));
}

bool MakeCache(std::tstring &outstr, LPCTSTR lpFileName)
{
	CGrJScript script;
	if (!LoadParser(script, L"html_cache")) {
		outstr = L"Load Error";
		return false;
	}
	return MakeCache(script, outstr, lpFileName);
}

int DeleteCache(LPCTSTR lpurl)
{
	CGrDBCache m_db;
	if (!m_db.Open()) {
		return -1;
	}
	CacheItem item;
	if (!m_db.Get(item, lpurl)) {
		return -1;
	}
	m_db.BeginSession();
	m_db.Delete(item.id);
	SHFILEOPSTRUCT shfo;
	shfo.hwnd = NULL;
	shfo.wFunc = FO_DELETE;
	shfo.pTo = nullptr;
	shfo.fFlags = FOF_SILENT | FOF_NOCONFIRMATION | FOF_ALLOWUNDO;
	shfo.hNameMappings = nullptr;
	shfo.lpszProgressTitle = nullptr;

	auto cache_filename = std::regex_replace(item.path.c_str(), std::wregex(L"%DATADIR%"), CGrTxtMiru::GetDataPath(), std::regex_constants::format_first_only);
	if (!cache_filename.empty()) {
		if (cache_filename.back() == L'/' || cache_filename.back() == L'\\') {
			cache_filename.pop_back();
		}
		if (std::filesystem::exists(cache_filename.c_str())) {
			std::vector<wchar_t> buf;
			buf.resize(cache_filename.size() + 2);
			_tcscpy_s(&buf[0], buf.size(), cache_filename.c_str());
			buf.push_back(L'\0');
			buf.push_back(L'\0');
			shfo.pFrom = &buf[0];
			if (0 != SHFileOperation(&shfo)) {
				m_db.Rollback();
				return -1;
			}
		}
	}
	m_db.Commit();
	return 0;
}
