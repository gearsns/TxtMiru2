#include <windows.h>
#include <windowsx.h>
#include <wininet.h>
#include <shlwapi.h>
#include <shlobj.h>
#include <commctrl.h>
#include "resource.h"
#include "BookMarkHtml.h"
#include "Text.h"
#include "TxtParam.h"
#include "TxtMiru.h"
#include "Shell.h"
#include "stdwin.h"
#include "Mlang.h"

#include <algorithm>
std::wstring toUtf16(const std::string &str)
{
	std::vector<wchar_t> wtext;
	do {
		int nLenWide = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
		if(nLenWide <= 0){
			break;
		}
		wtext.resize(nLenWide + 1);
		if(MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wtext.data(), nLenWide) != nLenWide){
			// Error
			wtext.clear();
		}
		return std::wstring(wtext.begin(), wtext.end());
	} while(0);
	return std::wstring();
}

void toLower(std::string &str){ std::transform(str.begin(), str.end(), str.begin(), ::tolower); }

template <size_t size>
int str_compare(const char *str, const char (&val)[size])
{
	return strncmp(str, val, size-1);
};

#define TagStartComment "<!--"
#define TagEndComment "-->"
#define EndTag "</"

static void Parse(const char *in_html, BookmarkDB::BookList &out_book_list)
{
	struct TagInfo {
		std::string name;
		std::map<std::string,std::string> attrs;
		std::vector<TagInfo*> child;
		virtual ~TagInfo()
		{
			for(auto item : child){
				delete item;
			}
			child.clear();
		}
		bool hasElement(const char *name0)
		{
			for(auto item : child){
				if(item->name == name0 || item->hasElement(name0)){
					return true;
				}
			}
			return false;
		}
		void innerText(std::string &text)
		{
			for(auto item : child){
				if(item->name == "text"){
					text += item->attrs["text"];
				} else {
					item->innerText(text);
				}
			}
		}
	};
	TagInfo taginfo;
	taginfo.name = "html";
	std::vector<TagInfo*> taginfo_list;
	taginfo_list.push_back(&taginfo);
	auto *pcur_taginfo = &taginfo;
	const char *start = in_html;
	const char *text = nullptr;
	std::map<std::string,std::string> escape_char_map = {
		{"&quot;", "\x22"},
		{"&amp;" , "\x26"},
		{"&lt;"  , "\x3C"},
		{"&gt;"  , "\x3E"},
		{"&nbsp;", "\xA0"},
		{"&copy;", "\xA9"},
	};
	auto addText = [&](const char *end_text)
	{
		if(text && text < end_text){
			auto *ctaginfo = new TagInfo;
			ctaginfo->name = "text";
			std::vector<char> attr_text;
			attr_text.resize(end_text-text+1);
			char *p = attr_text.data();
			for(; text<end_text; ++text){
				if(*text == '&'){
					bool bFind = false;
					for(const auto &ecm_item : escape_char_map){
						if(0 == strncmp(text, ecm_item.first.c_str(), ecm_item.first.size()-1)){
							text += ecm_item.first.size()-1;
							for(const char tch : ecm_item.second){
								bFind = true;
								*p = tch;
								++p;
							}
							break;
						}
					}
					if(bFind){
						--p;
					} else {
						*p = *text;
					}
				} else {
					*p = *text;
				}
				++p;
			}
			*p = '\0';
			ctaginfo->attrs["text"] = attr_text.data();
			if(pcur_taginfo){
				pcur_taginfo->child.push_back(ctaginfo);
			}
		}
		text = nullptr;
	};
	auto isTagStartChar = [](const char ch)
	{
		return (/**/(ch >= 'a' && ch <= 'z')
				||  (ch >= 'A' && ch <= 'Z')
				||  (ch >= '0' && ch <= '9')
				||   ch == '-' || ch == '_');
	};
	auto isSpaceChar = [](const char ch){ return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n'; };
	for(const auto *phtml=in_html; *phtml; ++phtml)
	{
		start = phtml;
		if(*start != '<'){
			continue;
		}
		// Comment
		if(0 == str_compare(start, TagStartComment)){
			start += _countof(TagStartComment)-1;
			for(; *start; ++start){
				if(*start == '-' && 0 == str_compare(start, TagEndComment)){
					start += _countof(TagEndComment)-1;
					break;
				}
			}
			phtml = start - 1;
			text = phtml + 1;
			continue;
		}
		// End Tag
		if(0 == str_compare(start, EndTag)){
			start += _countof(EndTag)-1;
			if(isTagStartChar(*start)){
				const char *start_tagname = start;
				std::string tagname;
				for(; *start && *start != '>'; ++start){
					if(tagname.empty() && isSpaceChar(*start)){
						tagname = std::string(start_tagname, start);
					}
				}
				if(tagname.empty()){
					tagname = std::string(start_tagname, start);
				}
				toLower(tagname);
				addText(phtml);
				// 閉じタグと対応するタグまで閉じる
				if(taginfo_list.size() > 0){
					auto rend = taginfo_list.rend();
					auto rit=taginfo_list.rbegin();
					for(; rit != rend; ++rit){
						const auto *ptaginfo = *rit;
						taginfo_list.pop_back();
						if(ptaginfo->name == tagname){
							break;
						}
					}
					if(taginfo_list.size() == 0){
						taginfo_list.push_back(&taginfo);
					}
					pcur_taginfo = taginfo_list.back();
				}
				if(!pcur_taginfo){
					pcur_taginfo = &taginfo;
				}
				phtml = start;
				text = phtml + 1;
				continue;
			}
		}
		start = phtml;
		// Start Tag
		start += 1;
		const char *tagname = start;
		for(; *start; ++start){
			if(!isTagStartChar(*start)){
				break;
			}
		}
		if(start != tagname){
			addText(phtml);
			auto *ctaginfo = new TagInfo;
			ctaginfo->name = std::string(tagname, start);
			toLower(ctaginfo->name);
			if(ctaginfo->name == "dt" || ctaginfo->name == "dd"){
				auto rend = taginfo_list.rend();
				for(auto rit=taginfo_list.rbegin(); rit != rend; ++rit){
					const auto *ptaginfo = *rit;
					if(ptaginfo->name == "dt" || ptaginfo->name == "dd"){
						if(taginfo_list.size() > 0){
							taginfo_list.pop_back();
							pcur_taginfo = taginfo_list.back();
						}
						if(!pcur_taginfo){
							pcur_taginfo = &taginfo;
						}
						break;
					}
				}
			}
			if(pcur_taginfo){
				pcur_taginfo->child.push_back(ctaginfo);
			}
			if(ctaginfo->name == "hr" || ctaginfo->name == "meta"){
			} else {
				taginfo_list.push_back(ctaginfo);
				pcur_taginfo = ctaginfo;
			}
			// Attributes
			char ch_quote = 0;
			std::string attrname;
			char ch_value = 0;
			const char *attr = nullptr;
			for(; *start; ++start){
				if(ch_quote){
					if(ch_quote == *start && '\\' != *(start-1)){
						ch_quote = 0;
					}
				} else if(*start == '>'){
					break;
				} else if(*start == '"' || *start == '\''){
					ch_quote = *start;
				} else if(isSpaceChar(*start)){
					if(attr && start > attr){
						if(ch_value == '='){
							if(*attr == *(start-1) && *attr == '"' || *attr == '\''){
								ctaginfo->attrs[attrname] = std::string(attr+1, start-1);
							} else {
								ctaginfo->attrs[attrname] = std::string(attr, start);
							}
						} else {
							attrname = std::string(attr, start);
							toLower(attrname);
							ctaginfo->attrs[attrname] = "";
						}
						ch_value = 0;
						attr = nullptr;
					}
					if(ch_value == 0){
						attr = start + 1;
					}
				} else if(*start == '='){
					if(attr && start > attr){
						attrname = std::string(attr, start);
						toLower(attrname);
						ctaginfo->attrs[attrname] = "";
					}
					attr = start + 1;
					ch_value = *start;
				} else if(!attr){
					attr = start;
				}
			}
			if(attr && start > attr){
				if(ch_value == '='){
					if(*attr == *(start-1) && *attr == '"' || *attr == '\''){
						ctaginfo->attrs[attrname] = std::string(attr+1, start-1);
					} else {
						ctaginfo->attrs[attrname] = std::string(attr, start);
					}
				} else {
					attrname = std::string(attr, start);
					toLower(attrname);
					ctaginfo->attrs[attrname] = "";
				}
			}
			phtml = start;
			text = phtml + 1;
			continue;
		}
	}
	//
	struct BookmarkTree
	{
		std::map<int,int> m_parent_map;
		BookmarkDB::BookList &m_book_list;
		BookmarkTree(BookmarkDB::BookList &b) : m_book_list(b){}

		void Create(TagInfo &tagInfo, int level = 0){
			if(tagInfo.name == "dl"){
				level += 1;
			} else if(/**/tagInfo.name == "h1"
					  ||  tagInfo.name == "h2"
					  ||  tagInfo.name == "h3"
					  ||  tagInfo.name == "h4"
					  ||  tagInfo.name == "h5"
					  ||  tagInfo.name == "h6"){
				if(!tagInfo.hasElement("a")){
					std::string text;
					tagInfo.innerText(text);
					// Folder push
					int parent = -1;
					auto it = m_parent_map.find(level);
					if(it != m_parent_map.end()){
						parent = it->second;
					}
					auto *pbook = new BookmarkDB::Book;
					pbook->id          = m_book_list.size();
					pbook->place_id    = -1;
					pbook->parent      = parent;
					pbook->type        = 1; // 0:ITEM, 1:FOLDER
					pbook->position    = 0;
					pbook->title       = toUtf16(text);
					m_book_list.push_back(pbook);
					m_parent_map[level+1] = pbook->id;
				}
			} else if(tagInfo.name == "a"){
				if(!tagInfo.hasElement("a") && !tagInfo.attrs["href"].empty()
				   && tagInfo.attrs["href"].compare(0,  6, "place:"     ) != 0
				   && tagInfo.attrs["href"].compare(0, 11, "javascript:") != 0
				   ){
					std::string text;
					tagInfo.innerText(text);
					// Item push
					int parent = -1;
					auto it = m_parent_map.find(level);
					if(it != m_parent_map.end()){
						parent = it->second;
					}
					auto *pbook = new BookmarkDB::Book;
					pbook->id          = m_book_list.size();
					pbook->place_id    = -1;
					pbook->parent      = parent;
					pbook->type        = 0; // 0:ITEM, 1:FOLDER
					pbook->position    = 0;
					pbook->base_url    = toUtf16(tagInfo.attrs["href"]);
					pbook->title       = toUtf16(text);
					m_book_list.push_back(pbook);
				}
			}
			for(auto item : tagInfo.child){
				Create(*item, level);
			}
		}
	};
	BookmarkTree bt(out_book_list);
	bt.Create(taginfo);
}

static bool isUTF16(BYTE *p, int len)
{
	while(--len > 0){
		if(*p == '\0'){
			return true;
		}
		++p;
	}
	return false;
}

CGrBookMarkHtml::CGrBookMarkHtml()
{
}

CGrBookMarkHtml::~CGrBookMarkHtml()
{
}

static const int l_DetectInputCodepageMaxChar = 512;

bool CGrBookMarkHtml::ReadFile(LPCTSTR lpFileName, BookmarkDB::BookList &out_book_list)
{
	bool bret = false;
	do {
		auto hFile = ::CreateFile(lpFileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, 0);
		if(hFile == INVALID_HANDLE_VALUE){
			return false;
		}
		DWORD sizeHigh;
		// サイズを取得して領域を確保します。
		auto fsize = ::GetFileSize(hFile, &sizeHigh);
		if(fsize > 0){
			auto hHeap = ::HeapCreate(0, 0, 0);
			auto *lpData = reinterpret_cast<BYTE*>(::HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sizeof(BYTE)*(fsize+1)));
			DWORD dw;
			do {
				if(!lpData){
					bret = false;
					break;
				}
				if(0 == ::ReadFile(hFile, lpData, fsize, &dw, nullptr)){
					bret = false;
					break;
				}
				lpData[dw] = _T('\0');
				DetectEncodingInfo detectEnc = {};
				auto *beginData = lpData;
				if(fsize >= 2){
					// BOM でチェック
					if(lpData[0] == 0xff && lpData[1] == 0xfe){
						beginData = &lpData[2];
						detectEnc.nCodePage =  1200;/* UTF-16LE    */
					} else if(lpData[0] == 0xfe && lpData[1] == 0xff){
						detectEnc.nCodePage =  1201;/* UTF-16BE    */
						beginData = &lpData[2];
					} else if(fsize >= 3 && lpData[0] == 0xef && lpData[1] == 0xbb && lpData[2] == 0xbf){
						detectEnc.nCodePage = 65001;/* utf-8       */
						beginData = &lpData[3];
					}
				}
				IMultiLanguage2 *pIML2 = nullptr;
				if (S_OK != CoCreateInstance(CLSID_CMultiLanguage, nullptr, CLSCTX_INPROC_SERVER, IID_IMultiLanguage2, reinterpret_cast<void**>(&pIML2))) {
					break;
				}
				if(!pIML2){
					bret = false;
					break;
				}
				if(detectEnc.nCodePage == 0){
					//
					if(detectEnc.nCodePage == 0){
						int srcSize = fsize;
						int detectEncCount = 1;
						int srcCheckSize = min(srcSize, l_DetectInputCodepageMaxChar);
						pIML2->DetectInputCodepage(MLDETECTCP_DBCS|MLDETECTCP_HTML, 0, reinterpret_cast<CHAR*>(beginData), &srcCheckSize, &detectEnc, &detectEncCount);
						if(/**/detectEnc.nCodePage !=   932/* sjis        */
						   &&  detectEnc.nCodePage !=  1200/* UTF-16LE    */
						   &&  detectEnc.nCodePage != 12000/* utf-32      */
						   &&  detectEnc.nCodePage != 20932/* EUC-JP      */
						   &&  detectEnc.nCodePage != 50220/* iso-2022-jp */
						   &&  detectEnc.nCodePage != 50221/* csISO2022JP */
						   &&  detectEnc.nCodePage != 50222/* iso-2022-jp */
						   &&  detectEnc.nCodePage != 51932/* euc-jp      */
						   &&  detectEnc.nCodePage != 65000/* utf-7       */
						   &&  detectEnc.nCodePage != 65001/* utf-8       */){
							if(S_OK != pIML2->DetectInputCodepage(MLDETECTCP_NONE, 0, reinterpret_cast<CHAR*>(beginData), &srcCheckSize, &detectEnc, &detectEncCount)){
								detectEnc.nCodePage = 932/* sjis */;
							}
						}
						if(detectEnc.nCodePage == 1252/* 変換エラー? 西欧? */){
							detectEnc.nCodePage = 932/* sjis */;
						}
					}
					if(detectEnc.nCodePage == 932/* sjis */ && ::isUTF16(beginData, fsize)){
						detectEnc.nCodePage = 1200/* UTF-16LE */;
					}
				}
				if(detectEnc.nCodePage != 65001/* UTF-8 */){
					DWORD mode = 0;
					auto wlen = static_cast<UINT>(fsize);

					auto utfhHeap = ::HeapCreate(0, 0, 0);
					if (utfhHeap) {
						auto* lpUtfData = reinterpret_cast<WCHAR*>(::HeapAlloc(utfhHeap, HEAP_ZERO_MEMORY, sizeof(WCHAR) * (wlen + 1)));
						if (lpUtfData) {
							auto file_size = static_cast<UINT>(fsize);
							if (S_OK == pIML2->ConvertString(&mode, detectEnc.nCodePage, CP_UTF8, reinterpret_cast<BYTE*>(beginData), &file_size, reinterpret_cast<BYTE*>(lpUtfData), &wlen)) {
								::HeapFree(hHeap, 0, lpData);
								::HeapDestroy(hHeap);
								hHeap = utfhHeap;
								lpUtfData[wlen] = '\0';
								lpData = reinterpret_cast<LPBYTE>(lpUtfData);
							}
							else {
								::HeapFree(utfhHeap, 0, lpUtfData);
								::HeapDestroy(utfhHeap);
							}
						}
						else {
							::HeapDestroy(utfhHeap);
						}
					}
				}
				pIML2->Release();
			} while(0);
			Parse(reinterpret_cast<char*>(lpData), out_book_list);
			::HeapFree(hHeap, 0, lpData);
			::HeapDestroy(hHeap);
		}
		::CloseHandle(hFile);
	} while(0);
	return bret;
}
