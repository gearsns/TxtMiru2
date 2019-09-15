#pragma warning( disable : 4786 )

#include <windows.h>
#include "stlutil.h"

namespace std
{
	tstring& replace(tstring& str, LPCTSTR src, LPCTSTR dst)
	{
		if(str.empty() || src == 0 || *src == 0 || dst == 0 || _tcscmp(src, dst) == 0){
			return str;
		}
		const auto src_len(_tcslen(src));
		const auto dst_len(_tcslen(dst));

		for(tstring::size_type pos(0); (pos = str.find(src, pos, src_len)) != tstring::npos; pos += dst_len){
			str.replace(pos, src_len, dst, dst_len);
		}
		return str;
	}
	void replaceCRLF(std::tstring &str, LPCTSTR repstr)
	{
		std::replace(str, _T("\r\n"), repstr);
		std::replace(str, _T("\r"), repstr);
		std::replace(str, _T("\n"), repstr);
	}
	bool utf82w(LPCSTR c, std::tstring &w)
	{
		if(c[0]==0xEF && c[1]==0xBB && c[2]==0xBF){ //BOM check
			c+=3;
		}
		bool result = false;

		int wlen = ::MultiByteToWideChar(CP_UTF8, 0, c, -1, nullptr, 0);
		if(wlen == 0){
			return false;
		}
		auto* buff = new WCHAR[wlen + 1];
		if(::MultiByteToWideChar(CP_UTF8, 0, c, -1, buff, wlen)){
			result = true;
			buff[wlen] = L'\0';
			w = buff;
		}
		delete[] buff;
		return result;
	}
	bool w2utf8(LPCTSTR w, std::string &c, UINT iCodePage/* = CP_UTF8*/)
	{
		bool result = false;

		auto clen = ::WideCharToMultiByte(iCodePage, 0, w, -1, nullptr, 0, nullptr, nullptr);
		if(clen == 0){
			return false;
		}
		auto* buff = new char[clen + 1];
		if(::WideCharToMultiByte(iCodePage, 0, w, -1, buff, clen, nullptr, nullptr)){
			result = true;
			buff[clen] = L'\0';
			c = buff;
		}
		delete[] buff;
		return result;
	}
};
