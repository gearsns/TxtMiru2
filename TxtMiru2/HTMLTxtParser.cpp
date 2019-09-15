#pragma warning( disable : 4786 )

#include <windows.h>
#include <wininet.h>
#include <shlwapi.h>
#include <shlobj.h>
#include <mshtmdid.h>
#include "resource.h"
#include "Text.h"
#include "Shell.h"
#include "Stdwin.h"
#include "TxtMiru.h"
#include "JScript.h"
#include "AozoraData.h"
#include "AozoraTxtParser.h"
#include "HTMLTxtParser.h"
#include "PictRenderer.h"
#include "TxtFuncWebFilter.h"
#include "TxtMiruTextType.h"

#import <mshtml.tlb> rename_namespace("MSHTML") named_guids raw_interfaces_only exclude("wireHDC","_RemotableHandle") rename( "TranslateAccelerator", "TranslateAcceleratorHTML" )

//#define __DBG__
#include "Debug.h"

CGrHTMLTxtParser::CGrHTMLTxtParser()
{
}

CGrHTMLTxtParser::~CGrHTMLTxtParser()
{
}
static HRESULT LoadURLFromMoniker(MSHTML::IHTMLDocument2Ptr spIHTMLDocument2, _bstr_t bstrURL)
{
	HRESULT hr = S_OK;
	try {
		IMonikerPtr spMoniler = nullptr;
		HRESULT hr = ::CreateURLMoniker(nullptr, bstrURL, &spMoniler);
		if (FAILED(hr)) {
			return hr;
		}
		if (!spMoniler) {
			return E_FAIL;
		}
		IBindCtxPtr spBindCtx = nullptr;
		hr = ::CreateBindCtx(0, &spBindCtx);
		if (FAILED(hr)) {
			return hr;
		}
		if (!spBindCtx) {
			return E_FAIL;
		}
		IPersistMonikerPtr spPersistMoniker = spIHTMLDocument2;
		if (spPersistMoniker == nullptr) {
			return E_FAIL;
		}
		hr = spPersistMoniker->Load(FALSE, spMoniler, spBindCtx, STGM_READ);
	}
	catch (...) {
		return E_FAIL;
	}
	return hr;
}
static HRESULT LoadURLFromFile(MSHTML::IHTMLDocument2Ptr spIHTMLDocument2, _bstr_t bstrFileName)
{
	try {
		IPersistFilePtr spPersistFile = spIHTMLDocument2;
		if (!spPersistFile) {
			return E_FAIL;
		}
		return spPersistFile->Load(bstrFileName, STGM_READ);
	}
	catch (...) {
		return E_FAIL;
	}
	return E_FAIL;
}

enum HTMLTAGTYPE {
	HTMLTAG_ERROR,
	HTMLTAG_UNKNOWN,
	HTMLTAG_A,
	HTMLTAG_DEL, // 取り消し線
	HTMLTAG_DIV,
	HTMLTAG_EM,
	HTMLTAG_FORM,
	HTMLTAG_H1,
	HTMLTAG_H2,
	HTMLTAG_H3,
	HTMLTAG_H4,
	HTMLTAG_H5,
	HTMLTAG_H6,
	HTMLTAG_IMG,
	HTMLTAG_P,
	HTMLTAG_RUBY,
	HTMLTAG_RB,
	HTMLTAG_RP,
	HTMLTAG_RT,
	HTMLTAG_SCRIPT,
	HTMLTAG_SPAN,
	HTMLTAG_STYLE,
	HTMLTAG_SUB,
	HTMLTAG_SUP,
	HTMLTAG_BR,
	HTMLTAG_PBR,
	HTMLTAG_TEXTNODE,
};
static struct NameTypeMap {
	LPCTSTR name;
	int type;
} l_htmltag_map[] = {
	// 予め name で ソートしておくこと
	{_T("#text"),HTMLTAG_TEXTNODE},
	{_T("A"),HTMLTAG_A       },
	{_T("BR"),HTMLTAG_BR      },
	{_T("DEL"),HTMLTAG_DEL     },
	{_T("DIV"),HTMLTAG_DIV     },
	{_T("EM"),HTMLTAG_EM      },
	{_T("FORM"),HTMLTAG_FORM    },
	{_T("H1"),HTMLTAG_H1      },
	{_T("H2"),HTMLTAG_H2      },
	{_T("H3"),HTMLTAG_H3      },
	{_T("H4"),HTMLTAG_H4      },
	{_T("H5"),HTMLTAG_H5      },
	{_T("H6"),HTMLTAG_H6      },
	{_T("IMG"),HTMLTAG_IMG     },
	{_T("P"),HTMLTAG_P       },
	{_T("PBR"),HTMLTAG_PBR     },
	{_T("RB"),HTMLTAG_RB      },
	{_T("RP"),HTMLTAG_RP      },
	{_T("RT"),HTMLTAG_RT      },
	{_T("RUBY"),HTMLTAG_RUBY    },
	{_T("S"),HTMLTAG_DEL     },
	{_T("SCRIPT"),HTMLTAG_SCRIPT  },
	{_T("SPAN"),HTMLTAG_SPAN    },
	{_T("STRIKE"),HTMLTAG_DEL     },
	{_T("STYLE"),HTMLTAG_STYLE   },
	{_T("SUB"),HTMLTAG_SUB     },
	{_T("SUP"),HTMLTAG_SUP     },
}, l_texttype_map[] = {
	// 予め name で ソートしておくこと
	{_T("black_circle"),TxtMiru::TT_ROUND_DOT         },
	{_T("black_up-pointing_triangle"),TxtMiru::TT_BLACK_TRIANGLE_DOT},
	{_T("bullseye"),TxtMiru::TT_DOUBLE_ROUND_DOT  },
	{_T("caption"),TxtMiru::TT_CAPTION           },
	{_T("fisheye"),TxtMiru::TT_BULLS_EYE_DOT     },
	{_T("keigakomi"),TxtMiru::TT_LINE_BOX_START    },
	{_T("overline_solid"),TxtMiru::TT_UNDER_LINE        },
	{_T("saltire"),TxtMiru::TT_SALTIRE_DOT       },
	{_T("sesame_dot"),TxtMiru::TT_DOT               },
	{_T("underline_dashed"),TxtMiru::TT_SHORT_DASHED_LINE },
	{_T("underline_dotted"),TxtMiru::TT_DOT_LINE          },
	{_T("underline_double"),TxtMiru::TT_DOUBLE_LINE       },
	{_T("underline_solid"),TxtMiru::TT_LINE              },
	{_T("underline_wave"),TxtMiru::TT_WAVE_LINE         },
	{_T("white_circle"),TxtMiru::TT_WHITE_ROUND_DOT   },
	{_T("white_sesame_dot"),TxtMiru::TT_WHITE_DOT         },
	{_T("white_up-pointing_triangle"),TxtMiru::TT_WHITE_TRIANGLE_DOT},
	{_T("yokogumi"),TxtMiru::TT_HORZ_START},
}, l_fontsize_map[] = {
	{_T("dai1"),  1},
	{_T("dai2"),  2},
	{_T("dai3"),  3},
	{_T("dai4"),  4},
	{_T("dai5"),  5},
	{_T("sho1"), -1},
	{_T("sho2"), -2},
	{_T("sho3"), -3},
	{_T("sho4"), -4},
	{_T("sho5"), -5},
};

static void SysFreeStringSetNull(BSTR &bstr)
{
	SysFreeString(bstr);
	bstr = nullptr;
}

static const TCHAR *bstr2char(const BSTR &bstr)
{
	if ((const TCHAR*)bstr) {
		return (const TCHAR*)bstr;
	}
	return _T("");
}
static __declspec(noinline) void moveBSTR2tstring(BSTR &bstr, std::tstring &out_str)
{
	out_str = bstr2char(bstr);
	SysFreeStringSetNull(bstr);
}

static int NameTypeMapCompare(const void *key, const void *pdata)
{
	auto name  = static_cast<LPCTSTR>(key);
	auto *pntm = static_cast<const NameTypeMap*>(pdata);
	return _tcsicmp(name, pntm->name);
}
static HTMLTAGTYPE GetHtmlTagType(MSHTML::IHTMLDOMNodePtr pchild_node)
{
	BSTR bstr = nullptr;
	if(!pchild_node || FAILED(pchild_node->get_nodeName(&bstr))){
		return HTMLTAG_ERROR;
	}
	auto ht = HTMLTAG_UNKNOWN;
	auto *pntm = static_cast<NameTypeMap*>(bsearch(bstr2char(bstr), l_htmltag_map, sizeof(l_htmltag_map)/sizeof(NameTypeMap), sizeof(NameTypeMap), NameTypeMapCompare));
	if(pntm){
		ht = static_cast<HTMLTAGTYPE>(pntm->type);
	}
	SysFreeString(bstr);
	return ht;
}
static bool getInnerText(MSHTML::IHTMLElement *element, std::tstring &out_str)
{
	BSTR bstr = nullptr;
	if (element && SUCCEEDED(element->get_innerText(&bstr))) {
		moveBSTR2tstring(bstr, out_str);
		return true;
	}
	return false;
}

static TxtMiru::TextType GetTextType(LPCTSTR str)
{
	auto *pntm = static_cast<NameTypeMap*>(bsearch(str, l_texttype_map, sizeof(l_texttype_map)/sizeof(NameTypeMap), sizeof(NameTypeMap), NameTypeMapCompare));
	if(pntm){
		return static_cast<TxtMiru::TextType>(pntm->type);
	}
	return TxtMiru::TT_MaxNum;
}

static int GetFontSize(LPCTSTR str)
{
	auto *pntm = static_cast<NameTypeMap*>(bsearch(str, l_fontsize_map, sizeof(l_fontsize_map)/sizeof(NameTypeMap), sizeof(NameTypeMap), NameTypeMapCompare));
	if(pntm){
		return pntm->type;
	}
	return 0;
}
/*
 lpwUrlで指定されたURIのキャッシュの情報を取得します。
 取得後、ファイル名をlpszwFileNameに対して書き込みます。
 */
static bool GetUrlCacheFileName(LPCWSTR lpwUrl, LPWSTR lpszwFileName, DWORD dwFileNameInBytes)
{
	if (CGrText::isMatchChar(lpwUrl, _T("file:///"))) {
		if (lstrcpynW(lpszwFileName, &(lpwUrl[sizeof(_T("file:///")) / sizeof(TCHAR) - 1]), dwFileNameInBytes)) {
			;
		}
		return true;
	}
	DWORD dwCbCacheInfo = 1024;
	auto lpCacheInfo = reinterpret_cast<LPINTERNET_CACHE_ENTRY_INFO>(new char[dwCbCacheInfo]);
	if (!lpCacheInfo) {
		return false;
	}
	bool bret = true;

	do {
		ZeroMemory(lpCacheInfo, dwCbCacheInfo);
		if(!GetUrlCacheEntryInfo(lpwUrl, lpCacheInfo, &dwCbCacheInfo)){
			switch(GetLastError()){
			case ERROR_FILE_NOT_FOUND:
				if(S_OK == URLDownloadToCacheFile(nullptr, lpwUrl, lpszwFileName, dwFileNameInBytes, 0, nullptr)){
					delete [] lpCacheInfo;
					return true;
				}
				bret = false;
				break;
			case ERROR_INSUFFICIENT_BUFFER:
				delete [] lpCacheInfo;
				lpCacheInfo = reinterpret_cast<LPINTERNET_CACHE_ENTRY_INFO>(new char[dwCbCacheInfo]);
				continue;
			default:
				bret = false;
				break;
			}
		}
		break;
	} while (true);
	if(bret && FAILED(_tcscpy_s(lpszwFileName, dwFileNameInBytes, lpCacheInfo->lpszLocalFileName))){
		bret = false;
	}
	delete [] lpCacheInfo;

	if(bret){
		// gzip圧縮のファイル場合、内容が 1F 8B 08 になるhtmファイルが取得されるので URLDownloadToCacheFileでファイルを取得しなおす
		const auto &param = CGrTxtMiru::theApp().Param();
		auto *lpExt = CGrShell::GetFileExt(lpszwFileName);
		if(lpExt && param.GetFileType(lpExt) == CGrTxtParam::FT_Html){
			if(S_OK == URLDownloadToCacheFile(nullptr, lpwUrl, lpszwFileName, dwFileNameInBytes, 0, nullptr)){
				return true;
			}
		}
	}
	return bret;
}

// AssocCreate が無くても動くように 動的にロードする
using PFNAssocCreate = HRESULT (WINAPI * )(CLSID, REFIID, LPVOID);
static bool getAssocTypeName(LPCTSTR lpExt, std::tstring &typeName)
{
	bool bret = false;
	IQueryAssociations *pQueryAssociations = nullptr;
	HMODULE hshlwapi_ll = NULL;
	do {
		auto hshlwapi = ::GetModuleHandle(_T("shlwapi.dll")); // OnMemoryにdllがあればそちらを使用する
		if(!hshlwapi){
			hshlwapi = hshlwapi_ll = ::LoadLibrary(_T("shlwapi.dll"));
		}
		if(!hshlwapi){
			break;
		}
		auto pfnAssocCreate = reinterpret_cast<PFNAssocCreate>(GetProcAddress(hshlwapi, "AssocCreate"));
		if(FAILED(pfnAssocCreate/*AssocCreate*/(CLSID_QueryAssociations, IID_IQueryAssociations, reinterpret_cast<PVOID*>(&pQueryAssociations)))){
			break;
		}
		if(!pQueryAssociations){
			break;
		}
		pQueryAssociations->Init(ASSOCF_INIT_DEFAULTTOSTAR, lpExt, NULL, NULL);
		DWORD dwSize = 0;
		pQueryAssociations->GetString(ASSOCF_NOTRUNCATE, ASSOCSTR_FRIENDLYDOCNAME, nullptr, nullptr, &dwSize);
		if(dwSize <= 0){
			break;
		}
		typeName.resize(dwSize);
		if(!typeName.empty()){
			pQueryAssociations->GetString(ASSOCF_NOTRUNCATE, ASSOCSTR_FRIENDLYDOCNAME, nullptr, &(typeName[0]), &dwSize);
		}
		bret = true;
	} while(0);
	if(hshlwapi_ll){
		::FreeLibrary(hshlwapi_ll);
	}
	return bret;
}

static INTERNET_SCHEME GetScheme(LPCTSTR szURL)
{
	if (szURL) {
		DWORD dwFlags = 0;
		URL_COMPONENTS urlComponents = { sizeof(URL_COMPONENTS) };
		if (InternetCrackUrl(szURL, 0, dwFlags, &urlComponents)) {
			return urlComponents.nScheme;
		}
	}
	return INTERNET_SCHEME_UNKNOWN;
}

static int hexToDec(TCHAR ch)
{
	if (ch >= _T('0') && ch <= _T('9')) {
		return ch - _T('0');
	}
	if (ch >= _T('A') && ch <= _T('F')) {
		return ch - _T('A') + 10;
	}
	if (ch >= _T('a') && ch <= _T('f')) {
		return ch - _T('a') + 10;
	}
	return -1;
}

static void url_decode(LPCTSTR lpSrc, std::tstring& dist)
{
	if (CGrText::isMatchChar(lpSrc, _T("file:///"))) {
		lpSrc += sizeof(_T("file:///")) / sizeof(TCHAR) - 1;
	}
	else {
		dist = lpSrc;
		return;
	}
	for (; *lpSrc; ++lpSrc) {
		if (*lpSrc == _T('+')) {
			dist += _T(' ');
		}
		else {
			int d1 = 0;
			int d2 = 0;
			if (*lpSrc == _T('%')
				&& (d1 = hexToDec(*(lpSrc + 1))) >= 0
				&& (d2 = hexToDec(*(lpSrc + 2))) >= 0) {
				std::tstring buf;
				BYTE utf8[2] = {};
				utf8[0] = (d1 << 4) + d2;
				if (CGrText::MultiByteToTString(CP_UTF8, (LPCSTR)utf8, 1, buf)) {
					dist += buf;
					lpSrc += 2;
				}
			}
			else {
				dist += *lpSrc;
			}
		}
	}
}
///////////////////////////////////////////////////////
class CGrHTMLTxtNodeParser :
	public IPropertyNotifySink, IOleClientSite, IDispatch, IOleWindow
{
private:
	static std::tstring TypeNameTxt;

	MSHTML::IHTMLDocument2Ptr m_spIHTMLDocument2;
	std::tstring &m_title;
	std::tstring &m_author;
	std::tstring &m_info;
	std::tstring &m_lastWriteTime;
	CGrTxtBuffer &m_buffer;
	TxtMiru::LineInfo m_line_info;
	TxtMiru::TextInfoList m_text_info_after;
	std::vector<TxtMiru::TextInfo> m_border_list;
	size_t m_border_current = 0;
	bool m_bBorder = false;
	int m_current_subtitle_no = 0;
	int m_iLine = 0;
	int m_iRubyPos = -1;
	enum BoldState {
		BS_INIT,
		BS_NONE,
		BS_BOLD
	} m_curBoldState = BS_INIT;
	struct IndentState {
		int i1st = 0;
		int i2nd = 0;
		IndentState() {}
		bool isIndent() { return i1st != 0 || i2nd != 0; }
		bool operator !=(const IndentState& i) const { return i1st != i.i1st || i2nd != i.i2nd; }
	};
	IndentState m_curIndentState;
	IndentState m_curRIndentState;

	UINT m_iIEOption = DLCTL_NO_FRAMEDOWNLOAD | DLCTL_NO_CLIENTPULL | DLCTL_DLIMAGES /*| DLCTL_DOWNLOADONLY*/ | DLCTL_SILENT | DLCTL_NO_SCRIPTS | DLCTL_NO_JAVA | DLCTL_NO_DLACTIVEXCTLS | DLCTL_NO_RUNACTIVEXCTLS;

	bool m_bEndofContents = false;
	bool m_bUseOverlapChar = true;
	bool m_bAutoDetectPictureType = true;
	int m_iLinkNum = 0;
	int m_iFontSize = 0;
	HANDLE m_hEvent = 0;
	LONG m_cRef = 0;
	DWORD m_dwCookie = 0;
	LPCONNECTIONPOINT m_pCP = nullptr;
	HRESULT m_hrConnected = CONNECT_E_CANNOTCONNECT;
	CGrWebFilter webFilter;
public:
	CGrHTMLTxtNodeParser(CGrTxtBuffer &buffer, std::tstring &title, std::tstring &author, std::tstring &info, std::tstring &lastWriteTime)
	: m_buffer(buffer), m_title(title), m_author(author), m_info(info), m_lastWriteTime(lastWriteTime)
	{
		const auto &param = CGrTxtMiru::theApp().Param();
		m_bUseOverlapChar = param.GetBoolean(CGrTxtParam::UseOverlapChar);
		int ieOptions[2] = {};
		param.GetPoints(CGrTxtParam::IEOption, ieOptions, sizeof(ieOptions)/sizeof(int));
		UINT idlctl_silent     = (ieOptions[0] == 1) ? DLCTL_SILENT     : 0;
		UINT idlctl_no_scripts = (ieOptions[1] == 1) ? DLCTL_NO_SCRIPTS : 0;
		m_iIEOption = DLCTL_NO_FRAMEDOWNLOAD | DLCTL_NO_CLIENTPULL | DLCTL_DLIMAGES | idlctl_silent | idlctl_no_scripts | DLCTL_NO_JAVA | DLCTL_NO_DLACTIVEXCTLS | DLCTL_NO_RUNACTIVEXCTLS;
	}
	virtual ~CGrHTMLTxtNodeParser()
	{
		Destory();
	}
	bool Parse(LPCTSTR lpFileName)
	{
		if (!Open(lpFileName) || !m_spIHTMLDocument2) {
			return false;
		}
		Destory();
		if (TypeNameTxt.empty()) {
			if (!getAssocTypeName(_T(".txt"), TypeNameTxt)) {
				TypeNameTxt = _T("xNG");
				TypeNameTxt[0] = '\0';
			}
		}
		MSHTML::IHTMLElementPtr pbody = nullptr;
		if (FAILED(m_spIHTMLDocument2->get_body(&pbody))) {
			return false;
		}
		if (!pbody) {
			return false;
		}
		{
			BSTR bstr = nullptr;
			if (m_lastWriteTime.empty() && SUCCEEDED(m_spIHTMLDocument2->get_lastModified(&bstr))) {
				moveBSTR2tstring(bstr, m_lastWriteTime);
			}
		}
		do {
			// テキストファイルは、CGrAozoraTxtParser で処理
			BSTR bstr = nullptr;
			if (FAILED(m_spIHTMLDocument2->get_mimeType(&bstr))) {
				break;
			}
			std::tstring mimeType;
			moveBSTR2tstring(bstr, mimeType);
			if (_tcsicmp(TypeNameTxt.c_str(), mimeType.c_str()) != 0) {
				auto ext = CGrShell::GetFileExtConst(lpFileName);
				if(!ext || 0 != _tcsicmp(_T("txt"), ext)){
					break;
				}
			}
			std::tstring innerText;
			CGrAozoraTxtParser aozora;
			if (getInnerText(pbody, innerText) && aozora.ReadBuffer(innerText.c_str(), m_buffer)) {
				m_title = aozora.GetTitle();
				m_author = aozora.GetAuthor();
				m_info = aozora.GetInfo();
				m_lastWriteTime = aozora.GetLastWriteTime();
				{
					std::vector<TCHAR> baseUrl;
					baseUrl.resize(lstrlen(lpFileName) + 2);
					CGrShell::GetParentDir(const_cast<TCHAR*>(lpFileName), &baseUrl[0]);
					for (auto&& ll : m_buffer.GetLineList()) {
						for (auto&& tl : ll.text_list) {
							switch(tl.textType){
							case TxtMiru::TT_LINK:
								break;
							case TxtMiru::TT_FILE:
								break;
							case TxtMiru::TT_PICTURE_FULL_PAGE:
							case TxtMiru::TT_PICTURE_HALF_PAGE:
							case TxtMiru::TT_PICTURE_LAYOUT:
								{
									if (tl.str.find(L':') == std::tstring::npos) {
										std::tstring url(&baseUrl[0]);
										url += tl.str;
										TCHAR cacheFileName[MAX_PATH] = {};
										if (GetUrlCacheFileName(url.c_str(), cacheFileName, _countof(cacheFileName))) {
											std::vector<std::tstring> param_list;
											std::tstring buf;
											for (auto c : tl.str) {
												if (c == L'\0') {
													param_list.push_back(buf);
												} else {
													buf += c;
												}
											}
											param_list.push_back(buf);
											param_list[0] = cacheFileName;
											tl.str.clear();
											for (auto& item : param_list) {
												tl.str += item;
												tl.str.append(1, L'\0');
											}
										}
									}
								}
								break;
							case TxtMiru::TT_TXTMIRU:
								break;
							}
						}
					}
				}
				return true;
			}
			return false;
		} while (0);
		bool ret = parse(MSHTML::IHTMLElementPtr(pbody, false));
		return ret;
	}
	void Destory()
	{
		if (m_spIHTMLDocument2) {
			if (SUCCEEDED(m_hrConnected)) {
				m_pCP->Unadvise(m_dwCookie);
				m_hrConnected = CONNECT_E_CANNOTCONNECT;
				m_dwCookie = 0;
			}
			if (m_pCP) {
				m_pCP->Release();
				m_pCP = nullptr;
			}
			bool ret = false;
			do {
				HRESULT hr;
				LPOLEOBJECT pOleObject = nullptr;
				if(FAILED(hr = m_spIHTMLDocument2->QueryInterface(IID_IOleObject, reinterpret_cast<LPVOID*>(&pOleObject)))){
					break;
				}
				hr = pOleObject->SetClientSite(nullptr);
				pOleObject->Release();
				if(FAILED(hr)){
					break;
				}
				ret = true;
			} while (0);
		}
	}
private:
	void preparse(LPCTSTR url)
	{
		MSHTML::IHTMLElementPtr pbody = nullptr;
		if (FAILED(m_spIHTMLDocument2->get_body(&pbody))) {
			return;
		}
		if (!pbody) {
			return;
		}
		CGrJScript script;
		if (::LoadPreParser(script, _T("html"))) {
			BSTR innerHTML = nullptr;
			if (SUCCEEDED(pbody->get_innerHTML(&innerHTML))) {
				std::tstring ret;
				std::tstring html_str;
				moveBSTR2tstring(innerHTML, html_str);
				{
					BSTR bstr;
					if (SUCCEEDED(m_spIHTMLDocument2->get_title(&bstr))) {
						std::tstring title;
						moveBSTR2tstring(bstr, title);
						if (title.size() > 0) {
							std::tstring head;
							head = _T("<head><title>");
							head += title;
							head += _T("</title></head>");
							html_str = head + html_str;
						}
					}
				}
				if (::PreParse(script, ret, url, html_str.c_str())) {
					BSTR html = SysAllocString(ret.c_str());
					pbody->put_innerHTML(html);
					SysFreeString(html);
				}
			}
		}
	}
	bool parse(MSHTML::DispHTMLBodyPtr pBody)
	{
		BSTR bstr_result = nullptr;
		if(FAILED(m_spIHTMLDocument2->get_url(&bstr_result))){
			return false;
		}
		std::tstring url;
		moveBSTR2tstring(bstr_result, url);
		addContents(url.c_str());
		//
		preparse(url.c_str());
		TxtMiru::LineInfo li;
		m_buffer.AddLineInfo(li);
		++m_iLine;
		if(pBody){
			IDispatchPtr pchildNodes = nullptr;
			MSHTML::IHTMLDOMNodePtr pBodyNode(pBody);
			if(FAILED(pBodyNode->get_childNodes(&pchildNodes))){
				return false;
			}
			if(!pchildNodes){
				return false;
			}
			parseNode(pBodyNode, pchildNodes);
			if(!m_line_info.text_list.empty()){
				addLineInfo();
			}
			if(m_curBoldState == BS_BOLD){
				m_line_info.text_list.push_back(TextInfoSpec(TxtMiru::TT_BOLD_END, 0));
				addLineInfo();
			}
			if(m_title.empty()){
				BSTR bstr = nullptr;
				if(SUCCEEDED(m_spIHTMLDocument2->get_title(&bstr))){
					moveBSTR2tstring(bstr, m_title);
				}
				m_buffer.AddLineInfo(m_line_info);
			}
			m_buffer.ForEachAll(static_cast<CGrTxtBuffer::CGrTypeFunc&&>(BoldType(m_buffer)));
		} else {
			return false;
		}
		return true;
	}
	void addContents(LPCTSTR lpStr)
	{
		m_info += lpStr;
		m_info += _T("\r\n");
	}
	void addLineInfo()
	{
		if(!m_text_info_after.empty()){
			for(auto &&item : m_text_info_after){
				m_line_info.text_list.push_back(item);
			}
			m_text_info_after.clear();
		}
		if(m_bBorder){
			AddBorderContinue();
		}
		m_buffer.AddLineInfo(m_line_info);
		m_line_info.text_list.clear();
		++m_iLine;
	}
	void nextLine()
	{
		if(m_iRubyPos < 0){
			// ruby内での改行は無視
			addLineInfo();
		}
	}
	int styleEmSize(const _variant_t &v_size, int def = 0)
	{
		std::tstring str_size(bstr2char(v_size.bstrVal));
		if(CGrText::isMatchLast(str_size.c_str(), _T("em"))){
			return CGrText::toInt(str_size);
		}
		return def;
	}
	bool setIndent(IndentState &indentState)
	{
		if(m_curIndentState != indentState){
			if(m_curIndentState.isIndent()){
				if(!m_line_info.text_list.empty()){
					addLineInfo();
				}
				m_line_info.text_list.push_back(TextInfoSpec(TxtMiru::TT_INDENT_END, 0));
			}
			if(indentState.isIndent()){
				if(!m_line_info.text_list.empty()){
					addLineInfo();
				}
				m_line_info.text_list.push_back(TextInfoSpec(TxtMiru::TT_INDENT_START, indentState.i1st, indentState.i2nd));
			}
			m_curIndentState = indentState;
			return true;
		}
		return false;
	}
	bool setRIndent(IndentState &rIndentState)
	{
		if(m_curRIndentState != rIndentState){
			if(m_curRIndentState.isIndent()){
				if (!m_line_info.text_list.empty()) {
					addLineInfo();
				}
				m_line_info.text_list.push_back(TextInfoSpec(TxtMiru::TT_RINDENT_END, 0));
			}
			if(rIndentState.isIndent()){
				if (!m_line_info.text_list.empty()) {
					addLineInfo();
				}
				m_line_info.text_list.push_back(TextInfoSpec(TxtMiru::TT_RINDENT_START, rIndentState.i1st, rIndentState.i2nd));
			}
			m_curRIndentState = rIndentState;
			return true;
		}
		return false;
	}
	bool hasAnchor(MSHTML::IHTMLDOMChildrenCollectionPtr childrenCollection)
	{
		if(!childrenCollection){
			return false;
		}
		long lLength = 0;
		if(FAILED(childrenCollection->get_length(&lLength))){
			return false;
		}
		for(long i = 0; i < lLength; i++){
			IDispatchPtr pnode = nullptr;
			if(FAILED(childrenCollection->item(_variant_t(i), &pnode))){
				continue;
			}
			MSHTML::IHTMLDOMNodePtr child_node(pnode);
			if(!child_node){
				continue;
			}
			if(HTMLTAG_A == GetHtmlTagType(child_node)){
				return true;
			}
			if(FAILED(child_node->get_childNodes(&pnode))){
				continue;
			}
			if(hasAnchor(pnode)){
				return true;
			}
		}
		return false;
	}
	void pushAnchor(int tl_idx, TxtMiru::LineInfo &li, const std::tstring &href, const std::tstring &comment, int order = 0)
	{
		if(!href.empty()){
			int tl_max = li.text_list.size();
			int tl_idx_init = tl_idx;
			for(; tl_idx<tl_max; ++tl_idx){
				const auto &ti = li.text_list[tl_idx];
				if(ti.textType == TxtMiru::TT_TEXT){
					break;
				}
			}
			li.text_list.push_back(TxtMiru::TextInfo(href.c_str(), m_iLinkNum, tl_idx, 0, tl_max, -1, TxtMiru::TT_LINK));
			if(order != 0){
				li.text_list.push_back(TxtMiru::TextInfo(comment.c_str(), order, tl_idx, 0, tl_max, -1, TxtMiru::TT_FILE));
			}
			for(tl_idx=tl_idx_init; tl_idx<tl_max; ++tl_idx){
				auto &&ti = li.text_list[tl_idx];
				if(TxtMiruType::isPicture(ti.textType)){
					ti.chrType = m_iLinkNum;
					ti.tpEnd.iIndex = tl_max;
				}
			}
		}
	}
	void addSubTitle(TxtMiru::TextType t, int iLine, int iIndex)
	{
		if(m_iLine != iLine){
			auto &&ll = m_buffer.GetLineList();
			for(; iLine < m_iLine; ++iLine){
				ll[iLine].text_list.push_back(TextInfoSpec(_T(""), t, iIndex, 0, ll[iLine].text_list.size(), -1));
				iIndex = 0;
			}
		}
		m_line_info.text_list.push_back(TextInfoSpec(_T(""), t, iIndex, 0, m_line_info.text_list.size(), -1));
	}
	void convertNoteChar(std::tstring &note)
	{
		auto lpNote = note.c_str();
		auto lpFindSrcTmp = CGrText::GetFirstShortMatchPos(lpNote, _T("※("), _T(")"));
		if(lpFindSrcTmp){
			auto lpSrc = CGrText::CharNextN(note.c_str(), 2);
			auto lpConvertStr = ConvertNoteChar(lpSrc);
			if(lpConvertStr){
				note = lpConvertStr;
				return;
			}
			if(m_bEndofContents){
				addContents(note.c_str());
				return;
			}
			std::tstring target(lpSrc, lpFindSrcTmp);
			auto lpFindSrcTmp2    = CGrText::Find(lpSrc, _T("「"));
			auto lpFindSrcEndTmp2 = CGrText::Find(lpSrc, _T("」"));
			if(lpFindSrcTmp2 && lpFindSrcEndTmp2 && lpFindSrcEndTmp2 > lpFindSrcTmp2){
				target = std::tstring(CGrText::CharNext(lpFindSrcTmp2), lpFindSrcEndTmp2);
			}
			addText(_T("※"), 0);
			auto &&text_list = m_line_info.text_list;
			int iIndex = text_list.size() - 1;
			text_list[iIndex].textType = TxtMiru::TT_OTHER;
			text_list[iIndex].str      = target;
			text_list[iIndex].tpBegin  = TxtMiru::TextListPos{iIndex, 0};
			note.resize(0);
		}
	}
	void setTextTypeRange(TxtMiru::TextType t, int iIndex)
	{
		if(!m_line_info.text_list.empty()){
			for(int i=m_line_info.text_list.size()-1; i>=iIndex; --i){
				if(m_line_info.text_list[i].textType == TxtMiru::TT_TEXT){
					m_line_info.text_list[i].textType = t;
				}
			}
		}
	}
	bool getAttribute(MSHTML::IHTMLElementPtr element, std::tstring &attr, LPCTSTR lpName)
	{
		bool bRet = false;
		VARIANT var = {};
		VariantInit(&var);
		auto bstrAttribute = SysAllocString(lpName);
		if(SUCCEEDED(element->getAttribute(bstrAttribute, 0, &var))){
			attr = bstr2char(var.bstrVal);
			bRet = true;
		}
		VariantClear(&var);
		SysFreeString(bstrAttribute);
		return bRet;
	}
	int AddBeginBoder()
	{
		m_bBorder = true;
		TxtMiru::TextInfo ti(
			_T("A"),
			static_cast<WORD>(m_border_list.size()), //           chrType         に 罫囲み連番
			m_curIndentState .i1st  , // 無理やり：tpBegin.iIndex  に TOP
			m_curRIndentState.i1st  , //           tpBegin.iPos    に BOTTOM
			0                       ,
			0                       , /* 開始=0、途中=1, 終了=2 */
			TxtMiru::TT_LINE_BOX_START
			);
		m_line_info.text_list.push_back(ti);
		m_border_list.push_back(ti);
		return m_border_list.size();
	}
	void AddBorderContinue()
	{
		int count = m_border_list.size();
		for(int i=m_border_current; i<count; ++i){
			auto &&ti = m_border_list[i];
			if(ti.tpEnd.iPos == 2/*終了*/ || ti.chrType < 0){
				continue;
			}
			ti.tpEnd.iPos = 1; // 途中
			m_line_info.text_list.push_back(ti);
		}
	}
	void AddEndBorder(size_t borderNo)
	{
		for(auto &&ti : m_border_list){
			if(ti.tpEnd.iPos == 2/*終了*/ || ti.chrType < 0 || ti.chrType > borderNo){
				continue;
			}
			ti.tpEnd.iIndex = m_line_info.text_list.size();
			ti.tpEnd.iPos = 2; // 終了
			ti.textType = TxtMiru::TT_LINE_BOX_END;
			m_line_info.text_list.push_back(ti);
		}
		if(m_border_current < borderNo){
			m_border_current = borderNo;
		}
		if(m_border_current >= m_border_list.size()){
			m_bBorder = false;
		}
	}
	//
	void parseNode(MSHTML::IHTMLDOMNodePtr pParent, MSHTML::IHTMLDOMChildrenCollectionPtr childrenCollection, int level = 0)
	{
		if(!childrenCollection){
			return;
		}
		long lLength = 0;
		if(FAILED(childrenCollection->get_length(&lLength))){
			return;
		}
		int iFontSize = m_iFontSize;
		for(long i = 0; i < lLength; i++){
			const int current_subtitle_no = m_current_subtitle_no;
			int iLine = m_iLine;
			int iIndex = m_line_info.text_list.size();
			bool bBlock = false;
			bool bCenter = false;
			auto preBoldState = m_curBoldState;
			auto preIndentState  = m_curIndentState ;
			auto preRIndentState = m_curRIndentState;
			IDispatchPtr pnode = nullptr;
			if(FAILED(childrenCollection->item(_variant_t(i), &pnode))){
				continue;
			}
			MSHTML::IHTMLDOMNodePtr current_node(pnode);
			if(!current_node){
				continue;
			}
			// <ul><li>A</li><span><li>B</li></span></ul>のようなHTMLがあるとBがIEのNodeTreeでは２回出てくるので親子関係をチェックする
			{
				MSHTML::IHTMLDOMNodePtr parentNode;
				current_node->get_parentNode(&parentNode);
				if(parentNode != pParent){
					continue;
				}
			}
			auto tag_type = GetHtmlTagType(current_node);
			if(tag_type == HTMLTAG_RP || tag_type == HTMLTAG_FORM || tag_type == HTMLTAG_ERROR){
				continue;
			}
			//
			int margin_top    = 0;
			int margin_bottom = 0;
			//
			int borderNo = -1;
			bool bTateChuYoko = false;
			bool bHorz = false;
			BSTR bstr = nullptr;
			if(m_iRubyPos < 0){
				switch(tag_type){
				case HTMLTAG_RUBY    : ; break;
				case HTMLTAG_RB      : ; break; // ルビを振る文字
				case HTMLTAG_RT      : ; break; // ルビ
				case HTMLTAG_BR      : ; break;
				case HTMLTAG_PBR     : ; break;
				case HTMLTAG_TEXTNODE: ; break;
				default:
					{
						MSHTML::IHTMLElementPtr element(current_node);
						MSHTML::IHTMLElement2Ptr element2(current_node);
						if(!element || !element2){
							break;
						}
						MSHTML::IHTMLCurrentStylePtr pcurrentStyle = nullptr;
						if(FAILED(element2->get_currentStyle(&pcurrentStyle))){
							break;
						}
						MSHTML::IHTMLCurrentStylePtr style(pcurrentStyle);
						if(!style){
							break;
						}
						std::tstring className;
						if(SUCCEEDED(element->get_className(&bstr))){
							moveBSTR2tstring(bstr, className);
						}
						if(!className.empty()){
							switch(GetTextType(className.c_str())){
							case TxtMiru::TT_CAPTION:
								{
									std::tstring innerText;
									if(getInnerText(element, innerText)){
										auto *pti = CGrTxtParser::GetPrevTextInfoPicture(m_buffer.GetLineList(), m_line_info.text_list);
										if(pti){
											if(m_bAutoDetectPictureType){
												pti->textType = TxtMiru::TT_PICTURE_HALF_PAGE;
											}
											CGrTxtParser::SetCaption(pti->str, innerText.c_str());
										}
										continue;
									}
								}
								break;
							case TxtMiru::TT_LINE_BOX_START:
								if(tag_type == HTMLTAG_DIV){
									::SysFreeStringSetNull(bstr);
									if(SUCCEEDED(style->get_borderWidth(&bstr))){
										borderNo = 0;
									}
									::SysFreeStringSetNull(bstr);
								}
								break;
							case TxtMiru::TT_HORZ_START:
								bHorz = true;
								break;
							}
						}
						//
						if(tag_type == HTMLTAG_SPAN){
							::SysFreeStringSetNull(bstr);
							if(SUCCEEDED(element2->get_dir(&bstr))){
								// 明示的に ltrになっている場合には、縦中横として処理
								if(_tcsicmp(bstr2char(bstr), _T("ltr")) == 0){
									bTateChuYoko = true;
								}
							}
						} else if(tag_type == HTMLTAG_DIV && m_author.empty()){
							getAttribute(element, m_author, _T("Author"));
						}
						//
						::SysFreeStringSetNull(bstr);
						if(SUCCEEDED(style->get_display(&bstr))){
							if(_tcsicmp(bstr2char(bstr), _T("block")) == 0){
								bBlock = true;
							}
						}
						::SysFreeStringSetNull(bstr);
						if(tag_type != HTMLTAG_IMG){
							VARIANT verticalAlign;
							VariantInit(&verticalAlign);
							if(SUCCEEDED(style->get_verticalAlign(&verticalAlign))){
								if(_tcsicmp(bstr2char(verticalAlign.bstrVal), _T("middle")) == 0){
									m_line_info.text_list.push_back(TextInfoSpec(TxtMiru::TT_CENTER, 0));
									nextLine();
									bCenter = true;
								}
							}
							VariantClear(&verticalAlign);
						}
						//
						if(SUCCEEDED(element->get_id(&bstr))){
							std::tstring id;
							moveBSTR2tstring(bstr, id);
							std::tstring innerText;
							if(id == _T("contents") && getInnerText(element, innerText)){
								if(CGrText::isMatchChar(innerText.c_str(), _T("+目次"))){
									continue;
								}
							}
						}
						//
						BoldState boldState = BS_INIT;
						VARIANT fontWeight;
						VariantInit(&fontWeight);
						if(SUCCEEDED(style->get_fontWeight(&fontWeight))){
							if(static_cast<long>(_variant_t(fontWeight, false)) > 400){
								boldState = BS_BOLD;
							}
						}
						VariantClear(&fontWeight);
						if(m_curBoldState != boldState){
							if(boldState == BS_BOLD){
								m_line_info.text_list.push_back(TextInfoSpec(TxtMiru::TT_BOLD_START, 0));
							} else if(m_curBoldState != BS_INIT){
								m_line_info.text_list.push_back(TextInfoSpec(TxtMiru::TT_BOLD_END, 0));
							}
						}
						m_curBoldState = boldState;
						//
						auto indentState     = m_curIndentState ;
						auto rIndentState    = m_curRIndentState;
						//
						VARIANT margin;
						VARIANT padding;
						// Left
						VariantInit(&margin);
						VariantInit(&padding);
						style->get_marginLeft(&margin);
						style->get_paddingLeft(&padding);
						int left  = styleEmSize(margin, -1000) + styleEmSize(padding);
						VariantClear(&margin);
						VariantClear(&padding);
						// Right
						VariantInit(&margin);
						VariantInit(&padding);
						style->get_marginRight(&margin);
						style->get_paddingRight(&padding);
						int right = styleEmSize(margin, -1000) + styleEmSize(padding);
						VariantClear(&margin);
						VariantClear(&padding);
						if(tag_type == HTMLTAG_P){
							// Top
							VariantInit(&margin);
							style->get_marginTop(&margin);
							margin_top = styleEmSize(margin);
							std::tstring str_size(bstr2char(margin.bstrVal));
							VariantClear(&margin);
							// Bottom
							VariantInit(&margin);
							style->get_marginBottom(&margin);
							margin_bottom = styleEmSize(margin);
							VariantClear(&margin);
						}
						//
						VARIANT textIndent;
						VariantInit(&textIndent);
						style->get_textIndent(&textIndent);
						int indent = styleEmSize(textIndent);
						VariantClear(&textIndent);
						//
						if(left >= 0 && (indent + left) >= 0){
							indentState.i1st = left + indent;
							indentState.i2nd = left;
						}
						if(right > 0){
							rIndentState.i1st = right;
							rIndentState.i2nd = right;
						}
						auto bIndent = setIndent(indentState);
						auto bRIndent = setRIndent(rIndentState);
						if(bIndent || bRIndent){
							addLineInfo();
						}
						// 文字サイズ
						if(!className.empty()){
							int fontSize = GetFontSize(className.c_str());
							if(fontSize != 0){
								m_iFontSize = fontSize;
								TxtMiru::TextInfo ti_size;
								ti_size.textType = TxtMiru::TT_TEXT_SIZE;
								ti_size.chrType  = m_iFontSize;
								m_line_info.text_list.push_back(ti_size);
							}
						}
						if(borderNo >= 0){
							borderNo = AddBeginBoder();
						}
					}
					break;
				}
			}
			IDispatchPtr child_nodes = nullptr;
			if(SUCCEEDED(current_node->get_childNodes(&child_nodes))){
				;
			}
			// 開始
			switch(tag_type){
			case HTMLTAG_A       :
				if(m_iLine != iLine){
					const auto &ll = m_buffer.GetLineList();
					if(static_cast<int>(ll.size()) > m_iLine){
						const auto &li = ll[m_iLine];
						iIndex = li.text_list.size();
						iLine = m_iLine;
					}
				}
				{
					MSHTML::IHTMLAnchorElementPtr element(current_node);
					if(!element){
						break;
					}
					//
					if(!child_nodes || !hasAnchor(child_nodes)){
						std::tstring str_id;
						// ID取得
						::SysFreeStringSetNull(bstr);
						{
							MSHTML::IHTMLElementPtr html_element(current_node);
							if(html_element){
								if(SUCCEEDED(html_element->get_id(&bstr))){
									std::tstring str = bstr2char(bstr);
									if(!str.empty()){
										str_id = str;
									}
									::SysFreeStringSetNull(bstr);
								}
							}
						}
						// Name取得
						::SysFreeStringSetNull(bstr);
						if(!str_id.empty()){
							if(SUCCEEDED(element->get_name(&bstr))){
								std::tstring str = bstr2char(bstr);
								if(!str.empty()){
									str_id = str;
								}
								::SysFreeStringSetNull(bstr);
							}
						}
						// Aタグが子にAタグを持っている場合は無視
						::SysFreeStringSetNull(bstr);
						if(SUCCEEDED(element->get_href(&bstr))){
							std::tstring link_str = bstr2char(bstr);
							url_decode(bstr2char(bstr), link_str);
							m_line_info.text_list.push_back(TxtMiru::TextInfo(link_str.c_str(), m_iLinkNum, -1, -1, -1, -1, TxtMiru::TT_LINK));
							if(!str_id.empty()){
								m_line_info.text_list.push_back(TxtMiru::TextInfo(str_id.c_str(), 0, -1, -1, -1, -1, TxtMiru::TT_ID));
							}
						}
						::SysFreeStringSetNull(bstr);
					}
				}
				break;
			case HTMLTAG_P       :
				if(margin_top > 0 || !m_line_info.text_list.empty()){
					nextLine();
				}
				break;
				// ruby rb  rp rt     rp
				//      漢字 ( かんじ )
			case HTMLTAG_RUBY    : m_iRubyPos = iIndex; break;
			case HTMLTAG_RB      : // ルビを振る文字
				bBlock = false;
				// ルビ開始位置には常に [TT_RUBY_SEP]を入れる // □RUBY長対応
				m_line_info.text_list.push_back(TextInfoSpec(TxtMiru::TT_RUBY_SEP, 0));
				++iIndex;
				m_iRubyPos = iIndex;
				break;
			case HTMLTAG_BR      : nextLine(); break;
			case HTMLTAG_PBR     :
				bCenter = false;
				if(!m_line_info.text_list.empty()){
					nextLine();
				}
				m_line_info.text_list.push_back(TextInfoSpec(TxtMiru::TT_NEXT_PAGE, 0));
				nextLine();
				break;
			case HTMLTAG_H1      :
			case HTMLTAG_H2      :
			case HTMLTAG_H3      :
			case HTMLTAG_H4      :
			case HTMLTAG_H5      :
			case HTMLTAG_H6      :
				{
					m_iFontSize = 5 - (tag_type-HTMLTAG_H1);
					TxtMiru::TextInfo ti_size;
					ti_size.textType = TxtMiru::TT_TEXT_SIZE;
					ti_size.chrType  = m_iFontSize;
					m_line_info.text_list.push_back(ti_size);
					m_current_subtitle_no = (tag_type-HTMLTAG_H1);
					break;
				}
			case HTMLTAG_TEXTNODE:
				{
					MSHTML::IHTMLDOMTextNodePtr textNode(current_node);
					::SysFreeStringSetNull(bstr);
					if(FAILED(textNode->get_data(&bstr))){
						break;
					}
					std::tstring text;
					moveBSTR2tstring(bstr, text);
					auto lpBuffer = text.c_str();
					bool bTrans = false;
					bool cr = false;
					auto lpStr = const_cast<LPTSTR>(lpBuffer), lpLine = const_cast<LPTSTR>(lpBuffer);
					for(;*lpStr; ++lpStr){
						if(*lpStr == _T('\n') || cr){
							if(*lpStr == _T('\n')){
								*lpStr = _T('\0');
								addLine(lpLine, lpStr, bTrans);
								lpLine = lpStr + 1;
							} else {
								addLine(lpLine, lpStr, bTrans);
								lpLine = lpStr;
							}
							nextLine();
							bTrans = false;
						}
						if(*lpStr == _T('\r')){
							cr = true;
							*lpStr = _T('\0');
						} else if(*lpStr == _T('〔')){
							bTrans = true;
						} else {
							cr = false;
						}
					}
					if(lpLine){
						addLine(lpLine, lpStr, bTrans);
					}
				}
				break;
			case HTMLTAG_SCRIPT:
			case HTMLTAG_STYLE:
				child_nodes = nullptr;
				break;
			default:
				break;
			}
			if (bHorz) {
				m_line_info.text_list.push_back(TxtMiru::TextInfo(_T(""), 1, 0, 0, 0, 0, TxtMiru::TT_HORZ_START));
			}
			if(child_nodes){
				parseNode(current_node, child_nodes, level+1);
			}
			if(bTateChuYoko){
				// 縦中横
				setTextTypeRange(TxtMiru::TT_ROTATE_NUM, iIndex);
			}
			if (bHorz) {
				m_line_info.text_list.push_back(TxtMiru::TextInfo(_T(""),1,0,0,0,0,TxtMiru::TT_HORZ_END));
			}
			const auto &ad = CGrAozoraData::Data();
			const auto *test_type_name = ad.GetTT();
			// 終了
			if(borderNo > 0){
				AddEndBorder(borderNo);
			}
			switch(tag_type){
			case HTMLTAG_A       :
				{
					MSHTML::IHTMLAnchorElementPtr element(current_node);
					if(!element
					   || m_iLine - iLine > 5                     // 5行以上、リンクがまたぐ場合はタグの整合性が取れていない可能性があるので無視する
					   || (child_nodes && hasAnchor(child_nodes)) // Aタグが子にAタグを持っている場合は無視
					   ){
						break;
					}
					::SysFreeStringSetNull(bstr);
					if(FAILED(element->get_href(&bstr))){
						break;
					}
					std::tstring href_buf;
					moveBSTR2tstring(bstr, href_buf);
					std::tstring href;
					url_decode(href_buf.c_str(), href);
					int iOrder = 0;
					{
						MSHTML::IHTMLElementPtr html_element(current_node);
						std::tstring str;
						if(getAttribute(html_element, str, _T("LinkOrder"))){
							iOrder = _ttoi(str.c_str());
						}
					}
					std::tstring a_comment;
					{
						MSHTML::IHTMLElementPtr html_element(current_node);
						getAttribute(html_element, a_comment, _T("LinkComment"));
					}
					if(!m_line_info.text_list.empty()){
						if(m_iLine != iLine){
							auto &&ll = m_buffer.GetLineList();
							int ll_max = ll.size();
							if(iLine < ll_max){
								pushAnchor(iIndex, ll[iLine], href, a_comment, iOrder);
								int ll_idx = iLine + 1;
								for(; ll_idx<ll_max; ++ll_idx){
									pushAnchor(0, ll[iLine], href, a_comment, iOrder);
								}
							}
							pushAnchor(0, m_line_info, href, a_comment, iOrder);
						} else {
							pushAnchor(iIndex, m_line_info, href, a_comment, iOrder);
							::SysFreeStringSetNull(bstr);
							if(SUCCEEDED(element->get_name(&bstr))){
								std::tstring name_str = bstr2char(bstr);
								if(!name_str.empty()){
									m_line_info.text_list.push_back(TxtMiru::TextInfo(name_str.c_str(), 0, -1, -1, -1, -1, TxtMiru::TT_ID));
								}
								const auto &text_list = m_line_info.text_list;
								if(m_current_subtitle_no == 0){
									for(int i=text_list.size()-1; i>=iIndex; --i){
										if(CGrTxtParser::isText(text_list[i].textType)){
											if(name_str.empty()){
												m_line_info.text_list.push_back(TextInfoSpec(_T(""), TxtMiru::TT_ID, iIndex, 0, m_line_info.text_list.size(), -1));
											} else {
												m_line_info.text_list.push_back(TextInfoSpec(_T(""), TxtMiru::TT_SUBTITLE3, iIndex, 0, m_line_info.text_list.size(), -1));
											}
											break;
										}
									}
								}
							}
							::SysFreeStringSetNull(bstr);
						}
					} else if(m_iLine != iLine){
						auto &&ll = m_buffer.GetLineList();
						int ll_max = ll.size();
						if(iLine < ll_max){
							pushAnchor(iIndex, ll[iLine], href, a_comment, iOrder);
							int ll_idx = iLine + 1;
							for(; ll_idx<ll_max; ++ll_idx){
								pushAnchor(0, ll[iLine], href, a_comment, iOrder);
							}
						}
					}
				}
				++m_iLinkNum;
				break;
			case HTMLTAG_DEL     :
				m_line_info.text_list.push_back(TextInfoSpec(static_cast<const TCHAR*>(test_type_name[TxtMiru::TT_DEL_LINE]), TxtMiru::TT_DEL_LINE, iIndex, 0, m_line_info.text_list.size(), -1));
				break;
			case HTMLTAG_EM      :
				{
					MSHTML::IHTMLElementPtr element(current_node);
					if(!element){
						break;
					}
					::SysFreeStringSetNull(bstr);
					if(FAILED(element->get_className(&bstr))){
						break;
					}
					auto text_type = GetTextType(bstr2char(bstr));
					if(text_type != TxtMiru::TT_MaxNum && !m_line_info.text_list.empty()){
						m_line_info.text_list.push_back(TextInfoSpec(static_cast<const TCHAR*>(test_type_name[text_type]), text_type, iIndex, 0, m_line_info.text_list.size(), -1));
					}
					::SysFreeStringSetNull(bstr);
				}
				break;
			case HTMLTAG_H1      :
				{
					bBlock = false;
					addLineInfo();
					MSHTML::IHTMLElementPtr element(current_node);
					if(!element){
						break;
					}
					::SysFreeStringSetNull(bstr);
					std::tstring str;
					if(getAttribute(element, str, _T("title")) && str.size() > 0){
						m_title = str;
						break;
					} else if(m_iLine == 1 && m_title.empty()){
						;
					} else if(SUCCEEDED(element->get_className(&bstr))){
						if(_tcsicmp(bstr2char(bstr), _T("title")) != 0){
							::SysFreeStringSetNull(bstr);
							break;
						}
						::SysFreeStringSetNull(bstr);
					} else {
						break;
					}
					getInnerText(element, m_title);
				}
				break;
			case HTMLTAG_H2      :
				{
					bBlock = false;
					addLineInfo();
					MSHTML::IHTMLElementPtr element(current_node);
					if(!element){
						break;
					}
					::SysFreeStringSetNull(bstr);
					if(m_iLine == 2 && !m_title.empty() && m_author.empty()){
						if(m_curBoldState != preBoldState && m_curBoldState == BS_BOLD){
							m_line_info.text_list.push_back(TextInfoSpec(TxtMiru::TT_BOLD_END, 0));
							m_curBoldState = preBoldState;
						}
						m_line_info.text_list.push_back(TextInfoSpec(TxtMiru::TT_NEXT_PAGE, 0));
						addLineInfo();
					} else if(SUCCEEDED(element->get_className(&bstr))){
						std::tstring classname = bstr2char(bstr);
						::SysFreeStringSetNull(bstr);
						if (m_title.empty() && _tcsicmp(classname.c_str(), _T("title")) != 0) {
							getInnerText(element, m_title);
							break;
						} else if (_tcsicmp(classname.c_str(), _T("author")) != 0) {
							break;
						}
					} else {
						break;
					}
					getInnerText(element, m_author);
				}
				break;
			case HTMLTAG_H3      :
				{
					if (m_iLine == 2 && !m_author.empty() && !m_title.empty()) {
						::SysFreeStringSetNull(bstr);
						MSHTML::IHTMLElementPtr element(current_node);
						if (!element) {
							break;
						}
						if (SUCCEEDED(element->get_className(&bstr))) {
							std::tstring classname = bstr2char(bstr);
							::SysFreeStringSetNull(bstr);
							if (_tcsicmp(classname.c_str(), _T("author")) == 0) {
								break;
							}
						}
					}
					addSubTitle(TxtMiru::TT_SUBTITLE1, iLine, iIndex);
					break;
				}
			case HTMLTAG_H4      : addSubTitle(TxtMiru::TT_SUBTITLE2, iLine, iIndex); break;
			case HTMLTAG_H5      :
			case HTMLTAG_H6      : addSubTitle(TxtMiru::TT_SUBTITLE3, iLine, iIndex); break;
			case HTMLTAG_IMG:
				{
					MSHTML::DispHTMLImgPtr elementdisp(current_node);
					if(!elementdisp){
						break;
					}
					MSHTML::IHTMLImgElementPtr img_element(current_node);
					if(!img_element){
						break;
					}
					::SysFreeStringSetNull(bstr);
					if(SUCCEEDED(((MSHTML::IHTMLElementPtr)elementdisp)->get_className(&bstr))){
						if(_tcsicmp(bstr2char(bstr), _T("gaiji")) == 0){
							::SysFreeStringSetNull(bstr);
							if(SUCCEEDED(img_element->get_alt(&bstr))){
								std::tstring text;
								moveBSTR2tstring(bstr, text);
								convertNoteChar(text);
								addText(text.c_str(), 0);
								break;
							}
						}
					}
					::SysFreeStringSetNull(bstr);
					if(FAILED(img_element->get_href(&bstr))){
						break;
					}
					std::tstring href;
					moveBSTR2tstring(bstr, href);
					TCHAR fileName[MAX_PATH] = {};
					if(GetUrlCacheFileName(href.c_str(), fileName, sizeof(fileName)/sizeof(TCHAR))){
						const auto &param = CGrTxtMiru::theApp().Param();
						SIZE size = {};
						param.GetPoints(CGrTxtParam::SkipHTMLImgSize, (int*)&size, 2);
						long naturalWidth = 0;
						long naturalHeight = 0;
						MSHTML::IHTMLImgElement4Ptr img4(elementdisp); // Windows Vista with SP1, Windows 7 [desktop apps only]
						if(img4){
							if(FAILED(img4->get_naturalWidth(&naturalWidth))){
								naturalWidth = 0;
							}
							if(FAILED(img4->get_naturalHeight(&naturalHeight))){
								naturalHeight = 0;
							}
						} else {
							if(FAILED(img_element->get_width(&naturalWidth))){
								naturalWidth = 0;
							}
							if(FAILED(img_element->get_height(&naturalHeight))){
								naturalHeight = 0;
							}
						}
						if(naturalHeight == 0 && naturalWidth == 0){
							// imgのサイズがわからない場合は、一旦画像を取得して取得
							CGrPictOleRenderer por;
							CGrBitmap bmp;
							if(por.Open(fileName, &bmp)){
								naturalHeight = bmp.Height();
								naturalWidth  = bmp.Width();
							}
						}
						if((size.cx > 0 && naturalWidth < size.cx) || (size.cy > 0 && naturalHeight < size.cy)){
							break;
						}
						TxtMiru::TextListPos lastTLP;
						std::tstring tt_str;
						tt_str  = fileName  ; tt_str.append(1, '\0');
						::SysFreeStringSetNull(bstr);
						if(SUCCEEDED(img_element->get_alt(&bstr))){
							tt_str += bstr2char(bstr);
						}
						::SysFreeStringSetNull(bstr);
						/* alt            */; tt_str.append(1, '\0');
						tt_str += _T("Size"); tt_str.append(1, '\0');
						tt_str += _T("Pos" ); tt_str.append(1, '\0');
						tt_str.append(1, '\0');
						if(m_line_info.text_list.size() > 0){ // 画像を同一行では扱わない <img>text<img>の時 textが表示されないので
							const auto &pre_ti = m_line_info.text_list.back();
							if(pre_ti.textType == TxtMiru::TT_TEXT){
								if(pre_ti.str.size() == 0 || pre_ti.str == _T(" ")){
									; // タグ間に改行があると 空白文字が入るので無視する
								} else {
									nextLine();
								}
							}
						}
						m_bAutoDetectPictureType = true;
						TxtMiru::TextType tt;
						if(naturalWidth > 0 && naturalHeight > 0 && 10 * naturalWidth / naturalHeight > 13){
							tt = TxtMiru::TT_PICTURE_FULL_PAGE;
						} else {
							tt = TxtMiru::TT_PICTURE_HALF_PAGE;
						}
						do {
							MSHTML::IHTMLElement2Ptr element2(current_node);
							if(!element2){
								break;
							}
							MSHTML::IHTMLCurrentStylePtr pcurrentStyle = nullptr;
							if(FAILED(element2->get_currentStyle(&pcurrentStyle))){
								break;
							}
							MSHTML::IHTMLCurrentStylePtr style(pcurrentStyle);
							if(!style){
								break;
							}
							VARIANT var = {0};
							VariantInit(&var);
							style->get_width(&var);
							std::tstring width_str;
							width_str = bstr2char(var.bstrVal);
							VariantClear(&var);
							if(_tcsicmp(width_str.c_str(), L"100%") == 0){
								tt = TxtMiru::TT_PICTURE_FULL_PAGE;
							} else if(_tcsicmp(width_str.c_str(), L"50%") == 0){
								tt = TxtMiru::TT_PICTURE_HALF_PAGE;
							} else if(_tcsicmp(width_str.c_str(), L"25%") == 0){
								tt = TxtMiru::TT_PICTURE_LAYOUT;
							} else {
								break;
							}
							m_bAutoDetectPictureType = false;
						} while(0);
						m_line_info.text_list.push_back(TextInfoSpec(static_cast<std::tstring&&>(tt_str), tt, lastTLP));
						nextLine(); // 画像を同一行では扱わない
					}
				}
				break;
			case HTMLTAG_P       :
				bBlock = false;
				if(!m_line_info.text_list.empty()){
					nextLine();
					if(margin_bottom > 0){
						nextLine();
					}
				}
				break;
			case HTMLTAG_RUBY    : bBlock = false; break;
			case HTMLTAG_RT      : // ルビ
				bBlock = false;
				if(iLine == m_iLine){
					auto &&text_list = m_line_info.text_list;
					int len = text_list.size();
					if(m_iRubyPos >= 0){
						if(len > 0 && iIndex > 0 && iIndex < len){
							std::tstring ruby(text_list[iIndex].str);
							text_list[iIndex].textType = TxtMiru::TT_RUBY;
							text_list[iIndex].tpBegin  = TxtMiru::TextListPos{m_iRubyPos, 0};
							text_list[iIndex].tpEnd    = TxtMiru::TextListPos{iIndex  -1, static_cast<signed int>(text_list[iIndex-1].str.size())-1};
							for(int j=iIndex+1; j<len; ++j){
								text_list[j].textType = TxtMiru::TT_COMMENT;
								ruby += text_list[j].str;
							}
							text_list[iIndex].str = ruby;
						}
					} else {
						for(int j=iIndex; j<len; ++j){
							text_list[j].textType = TxtMiru::TT_COMMENT;
						}
					}
				}
				m_iRubyPos = -1;
				break;
			case HTMLTAG_SUB     : setTextTypeRange(TxtMiru::TT_SUB_NOTE, iIndex); break;
			case HTMLTAG_SUP     : setTextTypeRange(TxtMiru::TT_SUP_NOTE, iIndex); break;
			default:
				break;
			}
			if(m_iFontSize !=  iFontSize){
				m_iFontSize = iFontSize;
				TxtMiru::TextInfo ti_size;
				ti_size.textType = TxtMiru::TT_TEXT_SIZE;
				ti_size.chrType  = iFontSize;
				m_line_info.text_list.push_back(ti_size);
			}
			if(m_iRubyPos < 0){
				auto bIndent = setIndent(preIndentState);
				auto bRIndent = setRIndent(preRIndentState);
				if(bIndent || bRIndent || bBlock){
					addLineInfo();
				}
				if(m_curBoldState != preBoldState){
					if(preBoldState == BS_BOLD){
						m_line_info.text_list.push_back(TextInfoSpec(TxtMiru::TT_BOLD_START, 0));
					} else {
						m_line_info.text_list.push_back(TextInfoSpec(TxtMiru::TT_BOLD_END, 0));
					}
					m_curBoldState = preBoldState;
				}
			}
			if(bCenter){
				if(!m_line_info.text_list.empty()){
					nextLine();
				}
				m_line_info.text_list.push_back(TextInfoSpec(TxtMiru::TT_NEXT_PAGE, 0));
				nextLine();
			}
			m_current_subtitle_no = current_subtitle_no;
		}
	}
	WORD getCharType(LPCTSTR lpSrc, LPCTSTR lpNextSrc)
	{
		return CGrText::GetStringTypeEx(lpSrc, lpNextSrc-lpSrc);
	}
	void addLine(LPCTSTR lpSrc, LPCTSTR lpEnd, bool bTrans)
	{
		std::tstring line(lpSrc, lpEnd);
		if(bTrans){
			// アクセント文字をUTFに変換
			ConvertAccentChar(line);
		}
		addText(line.c_str());
	}
	void addText(LPCTSTR lpSrc, int it_sl_len = -1)
	{
		if(!lpSrc || m_bEndofContents){
			addContents(lpSrc);
			return;
		}
		auto &&text_list = m_line_info.text_list;
		if(CGrText::isMatchChar(lpSrc, _T("［＃"))){
			if(CGrText::isMatchChar(lpSrc, _T("［＃改ページ］"))){
				if(!text_list.empty()){
					// 行の最中に 改段・改頁・改丁が見つかった場合、リストを分割
					addLineInfo();
				}
				text_list.push_back(TextInfoSpec(TxtMiru::TT_NEXT_PAGE, 0));
				addLineInfo();
			} else if(CGrText::isMatchChar(lpSrc, _T("［＃改丁］"))){
				if(!text_list.empty()){
					// 行の最中に 改段・改頁・改丁が見つかった場合、リストを分割
					addLineInfo();
				}
				text_list.push_back(TextInfoSpec(TxtMiru::TT_NEXT_PAPER, 0));
				addLineInfo();
			}
			return;
		}
		const auto &ad = CGrAozoraData::Data();
		const auto *it_slb = ad.GetTTList();
		if(it_sl_len < 0){
			it_sl_len = ad.GetTTListLength();
		}
		//
		auto lpTextStart = lpSrc;
		auto lpNextSrc   = CGrText::CharNext(lpSrc);
		bool bInsert = false;
		WORD chrType = 0xffff;
		for(; *lpSrc; lpSrc = lpNextSrc){
			bInsert = false;
			lpNextSrc = CGrText::CharNext(lpSrc);
			//
			if(CGrText::isMatchChar(lpSrc, _T("底本："))){
				addLineInfo();
				text_list.push_back(TextInfoHalfChar(_T(""), TxtMiru::TT_ENDOFCONTENTS));
				text_list.push_back(TextInfoSpec(TxtMiru::TT_COMMENT_BEGIN, 0));
				m_bEndofContents = true;
				addContents(lpSrc);
				break;
			}
			const auto *it_ps=it_slb;
			for(int it_ps_idx=it_sl_len; it_ps_idx>0; --it_ps_idx, ++it_ps){
				// LINEとかの特殊文字は、text_listを追加する(strに連続していれない)
				const auto &ms = (*it_ps);
				const auto textType = ms.type();
				if(textType == TxtMiru::TT_RUBY_SEP || (textType == TxtMiru::TT_OVERLAP_CHAR && !m_bUseOverlapChar)){
					continue;
				}
				if(CGrText::isMatchChar(lpSrc, ms)){
					if(lpTextStart != lpSrc){
						text_list.push_back(TextInfoLPTSTR(lpTextStart, lpSrc-1, chrType));
					}
					TxtMiru::TextListPos lastTLP;
					CGrTxtParser::GetRFindTextType(lastTLP, text_list, TxtMiru::TT_TEXT);
					text_list.push_back(TxtMiru::TextInfo(static_cast<const TCHAR*>(ms), getCharType(lpSrc, lpNextSrc), textType, lastTLP));
					lpTextStart = lpNextSrc = CGrText::CharNextN(lpSrc, ms.len());
					chrType = 0xffff;
					bInsert = true;
					break;
				}
			}
			if(bInsert){
				continue;
			}
			//
			auto curChrType = getCharType(lpSrc, lpNextSrc);
			if(chrType == 0xffff){
				chrType = curChrType;
			} else if(curChrType != chrType){
				text_list.push_back(TextInfoLPTSTR(lpTextStart, lpSrc-1, chrType));
				lpTextStart = lpSrc;
				chrType     = curChrType;
			}
		}
		if(!bInsert){
			text_list.push_back(TextInfoLPTSTR(lpTextStart, lpNextSrc-1, chrType));
		}
	}
	bool Create()
	{
		bool ret = false;
		LPCONNECTIONPOINTCONTAINER pCPC = nullptr;
		do {
			HRESULT hr;
			LPOLEOBJECT pOleObject = nullptr;
			LPOLECONTROL pOleControl = nullptr;
			hr = m_spIHTMLDocument2.CreateInstance(MSHTML::CLSID_HTMLDocument);
			if (FAILED(hr)) {
				break;
			}
			if(FAILED(hr = m_spIHTMLDocument2->QueryInterface(IID_IOleObject, reinterpret_cast<LPVOID*>(&pOleObject)))){
				break;
			}
			hr = pOleObject->SetClientSite(static_cast<IOleClientSite*>(this));
			pOleObject->Release();
			if (FAILED(hr)) {
				break;
			}
			if(FAILED(hr = m_spIHTMLDocument2->QueryInterface(IID_IOleControl, reinterpret_cast<LPVOID*>(&pOleControl)))){
				break;
			}
			hr = pOleControl->OnAmbientPropertyChange(DISPID_AMBIENT_DLCONTROL);
			pOleControl->Release();
			if (FAILED(hr)) {
				break;
			}
			if(FAILED(hr = m_spIHTMLDocument2->QueryInterface(IID_IConnectionPointContainer, reinterpret_cast<LPVOID*>(&pCPC)))){
				break;
			}
			if(FAILED(hr = pCPC->FindConnectionPoint(IID_IPropertyNotifySink, &m_pCP))){
				break;
			}
			m_hrConnected = m_pCP->Advise(static_cast<LPUNKNOWN>(static_cast<IPropertyNotifySink*>(this)), &m_dwCookie);

			ret = true;
		} while (0);
		if (pCPC) {
			pCPC->Release();
		}
		return ret;
	}
	bool Open(LPCTSTR lpUrl)
	{
		if(!Create()){
			return false;
		}
		const auto &param = CGrTxtMiru::theApp().Param();
		{
			auto lpCookie = param.GetCookie(lpUrl);
			if(lpCookie){
				InternetSetCookieEx(lpUrl,
									nullptr, lpCookie,
									0, 0);
			}
		}
		char *charProxyServer    = nullptr;
		char *charNoProxyAddress = nullptr;
		do {
			INTERNET_PROXY_INFO proxy = {0};
			if(param.GetBoolean(CGrTxtParam::UseIESetting)){
				proxy.dwAccessType = INTERNET_OPEN_TYPE_PRECONFIG;
			} else if(param.GetBoolean(CGrTxtParam::UseProxy)){
				//プロクシの変更
				std::tstring strProxyServer, strNoProxyAddress;
				param.GetText(CGrTxtParam::ProxyServer   , strProxyServer   );
				param.GetText(CGrTxtParam::NoProxyAddress, strNoProxyAddress);
				int lenStrProxyServer    = strProxyServer   .size() * 3;
				int lenStrNoProxyAddress = strNoProxyAddress.size() * 3;
				charProxyServer    = new char[lenStrProxyServer    + 1];
				charNoProxyAddress = new char[lenStrNoProxyAddress + 1];
				if(!charProxyServer || !charNoProxyAddress){
					break;
				}
				memset(charProxyServer   , 0x00, lenStrProxyServer   +1);
				memset(charNoProxyAddress, 0x00, lenStrNoProxyAddress+1);
				if(lenStrProxyServer > 0){
					sprintf_s(charProxyServer   , lenStrProxyServer   , "%S", strProxyServer   .c_str());
				}
				if(lenStrNoProxyAddress > 0){
					sprintf_s(charNoProxyAddress, lenStrNoProxyAddress, "%S", strNoProxyAddress.c_str());
				}
				proxy.dwAccessType    = INTERNET_OPEN_TYPE_PROXY   ;
				proxy.lpszProxy       = reinterpret_cast<LPCTSTR>(charProxyServer   );//"localhost:8080";
				proxy.lpszProxyBypass = reinterpret_cast<LPCTSTR>(charNoProxyAddress);//"localhost:8080";
			} else {
				proxy.dwAccessType = INTERNET_OPEN_TYPE_DIRECT;
			}
			UrlMkSetSessionOption(INTERNET_OPTION_PROXY, &proxy, sizeof(proxy) , 0);
		} while(0);
		if(charProxyServer   ){ delete [] charProxyServer   ; }
		if(charNoProxyAddress){ delete [] charNoProxyAddress; }
		//
		HRESULT hr = E_FAIL;
		switch(GetScheme(lpUrl)){
		case INTERNET_SCHEME_HTTP  :
		case INTERNET_SCHEME_FTP   :
		case INTERNET_SCHEME_GOPHER:
		case INTERNET_SCHEME_HTTPS :
		case INTERNET_SCHEME_FILE  :
			hr = ::LoadURLFromMoniker(/*this, */m_spIHTMLDocument2, lpUrl);
			break;
		case INTERNET_SCHEME_NEWS  :
		case INTERNET_SCHEME_MAILTO:
		case INTERNET_SCHEME_SOCKS :
			break;
		default:
			hr = ::LoadURLFromFile(m_spIHTMLDocument2, lpUrl);
			break;
		}
		MSG msg = {0};
		while(::GetMessage(&msg, NULL, 0, 0)){
			if (msg.message == WM_DESTROY) {
				break;
			}
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
		return SUCCEEDED(hr);
	}
public:
	// IUnknown methods
	STDMETHODIMP QueryInterface(REFIID riid, LPVOID* ppvObject)
	{
		*ppvObject = nullptr;

		if(IsEqualIID(IID_IUnknown, riid) || IsEqualIID(IID_IPropertyNotifySink, riid)){
			*ppvObject = static_cast<IPropertyNotifySink*>(this);
		} else if(IsEqualIID(IID_IOleClientSite, riid)){
			*ppvObject = static_cast<IOleClientSite*>(this);
		} else if(IsEqualIID(IID_IDispatch, riid)){
			*ppvObject = static_cast<IDispatch*>(this);
		} else if(IsEqualIID(IID_IOleWindow, riid)){
			*ppvObject = static_cast<IOleWindow*>(this);
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
		if(InterlockedDecrement(&m_cRef) == 0){
			return 0;
		}
		return m_cRef;
	}
	//
	// IOleWindow/*IOleInPlaceSite*/ methods
	STDMETHODIMP GetWindow(HWND *phWnd){
		if(phWnd && *phWnd == NULL){
			*phWnd = CGrStdWin::GetWnd();
		}
		return NOERROR;
	}
	STDMETHODIMP ContextSensitiveHelp(BOOL b){ return NOERROR; }
	// IPropertyNotifySink methods
	STDMETHODIMP OnChanged(DISPID dispID)
	{
		if(DISPID_READYSTATE == dispID){
			VARIANT vResult = {0};
			EXCEPINFO excepInfo;
			UINT uArgErr;
			DISPPARAMS dp = {NULL, NULL, 0, 0};
			if(SUCCEEDED(m_spIHTMLDocument2->Invoke(DISPID_READYSTATE, IID_NULL, LOCALE_SYSTEM_DEFAULT,
													DISPATCH_PROPERTYGET, &dp, &vResult, &excepInfo, &uArgErr))){
				auto lReadyState = static_cast<READYSTATE>(V_I4(&vResult));
				switch(lReadyState){
				case READYSTATE_UNINITIALIZED: break;
				case READYSTATE_LOADING      : break;
				case READYSTATE_LOADED       : break;
				case READYSTATE_INTERACTIVE  : break;
				case READYSTATE_COMPLETE     :
					PostThreadMessage(GetCurrentThreadId(), WM_DESTROY, 0, 0);
					break;
				}
				VariantClear(&vResult);
			} else {
				PostThreadMessage(GetCurrentThreadId(), WM_DESTROY, 0, 0);
			}
		}
		return NOERROR;
	}
	STDMETHODIMP OnRequestEdit(DISPID dispID){ return NOERROR; }
	// IOleClientSite methods
	STDMETHODIMP SaveObject(){ return E_NOTIMPL; }
	STDMETHODIMP GetMoniker(DWORD dwAssign, DWORD dwWhichMoniker, IMoniker** ppmk){ return E_NOTIMPL; }
	STDMETHODIMP GetContainer(IOleContainer** ppContainer){ return E_NOTIMPL; }
	STDMETHODIMP ShowObject(){ return E_NOTIMPL; }
	STDMETHODIMP OnShowWindow(BOOL fShow){ return E_NOTIMPL; }
	STDMETHODIMP RequestNewObjectLayout(){ return E_NOTIMPL; }
	// IDispatch method
	STDMETHODIMP GetTypeInfoCount(UINT* pctinfo){ return E_NOTIMPL; }
	STDMETHODIMP GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo** ppTInfo){ return E_NOTIMPL; }
	STDMETHODIMP GetIDsOfNames(REFIID riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId){ return E_NOTIMPL; }
	STDMETHODIMP Invoke(DISPID dispIdMember,
						REFIID riid,
						LCID lcid,
						WORD wFlags,
						DISPPARAMS __RPC_FAR *pDispParams,
						VARIANT __RPC_FAR *pVarResult,
						EXCEPINFO __RPC_FAR *pExcepInfo,
						UINT __RPC_FAR *puArgErr)
	{
		if(!pVarResult){
			return E_POINTER;
		}
		switch(dispIdMember){
		case DISPID_AMBIENT_DLCONTROL:
			V_VT(pVarResult) = VT_I4;
			V_I4(pVarResult) = m_iIEOption;
			break;
		default:
			return DISP_E_MEMBERNOTFOUND;
		}
		return NOERROR;
	}
};
std::tstring CGrHTMLTxtNodeParser::TypeNameTxt;

bool CGrHTMLTxtParser::ReadFile(LPCTSTR lpFileName, CGrTxtBuffer &buffer)
{
	CGrHTMLTxtNodeParser htpd(buffer, m_title, m_author, m_info, m_lastWriteTime);
	return htpd.Parse(lpFileName);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CGrHTMLTxtParser::ReadBuffer(LPCTSTR lpBuffer, CGrTxtBuffer &buffer)
{
	return false;
}
