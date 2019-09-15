#include <process.h>
#include "TxtDocument.h"
#include "text.h"
#include "shell.h"
#include "stlutil.h"
#include "TxtMapper.h"
#include "TxtParser.h"
#include "resource.h"
#include "CurrentDirectory.h"
#include "stdwin.h"
#include "MessageBox.h"
#include "HTMLPropertyDlg.h"
#include "JScript.h"
#include "TxtFuncBookmark.h"
#include "Win32Wrap.h"

#include "TxtMiru.h"
#include "TxtParam.h"
#include "Archive.h"

//#define __DBG__
#include "Debug.h"
static std::tstring::size_type getArcfileSplitPos(const std::tstring &filename)
{
	std::tstring::size_type pos = 0;
	return filename.find(ARCFILE_SPLIT_CHAR, pos);
}
static bool getArchivePath(LPTSTR out_filename,  int max_len, LPCTSTR arc_in_filename)
{
	const auto lpWorkPath = CGrTxtMiru::GetWorkPath();
	if(!lpWorkPath){
		return false;
	}
	_stprintf_s(out_filename, max_len, _T("%sArchive/%s"), lpWorkPath, arc_in_filename);
	return true;
}
static void getArcFullPathName(std::tstring &out_path, LPCTSTR arc_filename, LPCTSTR arc_in_filename)
{
	TCHAR buf[MAX_PATH+1] = {0};
	_stprintf_s(buf, _T("%s") ARCFILE_SPLIT_CHAR _T("%s"), arc_filename, arc_in_filename);
	CGrShell::ToPrettyFileName(buf, out_path);
}
static void deleteTempDir()
{
	TCHAR curTmpDir[_MAX_PATH+1] = {0};
	if(!getArchivePath(curTmpDir, sizeof(curTmpDir)/sizeof(TCHAR), _T(""))){
		return;
	}
	auto lpchr = curTmpDir;
	for(; *lpchr; ++lpchr){
		if(*lpchr == _T('/')){
			*lpchr = _T('\\');
		}
	}
	if(CGrShell::IsBackSlash(*(lpchr-1))){
		*(lpchr-1) = '\0';
	}
	*(lpchr+1) = '\0'; // 末尾を \0\0にする

	SHFILEOPSTRUCT sfo = {};
	sfo.fFlags = FOF_SILENT | FOF_NOCONFIRMATION | FOF_NOERRORUI;
	sfo.wFunc = FO_DELETE;
	sfo.pFrom = curTmpDir;
	SHFileOperation(&sfo);
}

CGrTxtDocument::CGrTxtDocument()
{
	m_bookMark.Attach(this);
	m_subTitle.Attach(this);
	m_search.Attach(this);
	m_bOpenBookmark = false;
	m_maxPage = 1;
}

CGrTxtDocument::~CGrTxtDocument()
{
}

void CGrTxtDocument::clear()
{
	m_bOpenFile = false;
	m_maxPage = 1;
	m_subTitle.ClearLeaf();
	m_txtBuffer.Clear();
	while(!m_pageForwardHistory.empty()){
		m_pageForwardHistory.pop();
	}
	while(!m_pageBackHistory.empty()){
		m_pageBackHistory.pop();
	}
}

bool CGrTxtDocument::OpenText(LPCTSTR lpText)
{
	CGrTxtParserMgr parser;
	do {
		m_fileName.clear();
		m_bookMark.Clear();
		m_bOpenBookmark = false;
		clear();
		if(!parser.ReadBuffer(lpText, m_txtBuffer)){
			break;
		}
	} while(0);
	if(m_txtBuffer.GetTitle().empty()){
		std::tstring str;
		CGrText::LoadString(IDS_DOC_TEXT, str);
		m_txtBuffer.SetTitle(str.c_str());
	}
	CGrTxtMapper::CreateIndex(*this);
	return true;
}

// DetectInputCodepage
// mlang.dll
// ConvertINetString

bool CGrTxtDocument::OpenClipboard()
{
	HANDLE hMem  = NULL;
	bool   bopen = false;

	CGrTxtParserMgr parser;
	do {
		m_fileName.clear();
		m_bookMark.Clear();
		m_bOpenBookmark = false;
		clear();
		// クリップボードを開きます。
		if(!::OpenClipboard(NULL)){
			break;
		}
		bopen = true;
#ifdef UNICODE
		if(!::IsClipboardFormatAvailable(CF_UNICODETEXT)){
			break;
		}
		hMem = ::GetClipboardData(CF_UNICODETEXT);
#else
		if(::IsClipboardFormatAvailable(CF_UNICODETEXT)){
		}
		hMem = ::GetClipboardData(CF_TEXT);
#endif
		if(!hMem){
			break;
		}
		auto lpBuffer = static_cast<LPTSTR>(::GlobalLock(hMem));
		if(!lpBuffer){
			break;
		}
		if(!parser.ReadBuffer(lpBuffer, m_txtBuffer)){
			break;
		}
	} while(0);

	if(hMem){
		::GlobalUnlock(hMem);
	}
	if(bopen){
		// クリップボードを閉じます。
		::CloseClipboard();
	}
	if(m_txtBuffer.GetTitle().empty()){
		std::tstring str;
		CGrText::LoadString(IDS_DOC_CLIPBOARD, str);
		m_txtBuffer.SetTitle(str.c_str());
	}
	CGrTxtMapper::CreateIndex(*this);
	m_bookMark.RebuildPageNo();
	m_subTitle.RebuildPageNo();
	return true;
}

// ファイルのあるフォルダにカレントフォルダを移動
static void setCurrentDirectoryFromFilename(LPCTSTR lpfilename)
{
	TCHAR tmp_filedir[MAX_PATH+1];
	_tcscpy_s(tmp_filedir, lpfilename);
	if((GetFileAttributes(lpfilename) & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY){
		::SetCurrentDirectory(lpfilename);
	} else if(CGrShell::RemoveFileName(tmp_filedir)){
		::SetCurrentDirectory(tmp_filedir);
	}
}

static CGrTxtDocument::TXTMIRU_FT getFileType(LPCTSTR lpext, CGrTxtDocument::TXTMIRU_FT nomatched_ft=CGrTxtDocument::TXTMIRU_FT_NONE)
{
	const auto &param = CGrTxtMiru::theApp().Param();
	switch(param.GetFileType(lpext)){
	case CGrTxtParam::FT_Html  :
		return CGrTxtDocument::TXTMIRU_FT_TEXT;
	case CGrTxtParam::FT_Text  :
		return CGrTxtDocument::TXTMIRU_FT_TEXT;
	case CGrTxtParam::FT_Link  :
		return CGrTxtDocument::TXTMIRU_FT_LINK;
	case CGrTxtParam::FT_Siori :
		return CGrTxtDocument::TXTMIRU_FT_SIORI;
	case CGrTxtParam::FT_Arc7z :
	case CGrTxtParam::FT_ArcCab:
	case CGrTxtParam::FT_ArcLzh:
	case CGrTxtParam::FT_ArcRar:
	case CGrTxtParam::FT_ArcZip:
		return CGrTxtDocument::TXTMIRU_FT_ARCHIVE;
	}
	return nomatched_ft;
}
CGrTxtDocument::TXTMIRU_FT CGrTxtDocument::GetFileType(LPCTSTR lpFileName)
{
	if(getArcfileSplitPos(lpFileName) != std::tstring::npos){
		return TXTMIRU_FT_ARCHIVE;
	}
	if(CGrShell::IsURI(lpFileName)){
		return TXTMIRU_FT_TEXT;
	}
	return getFileType(CGrShell::GetFileExtConst(lpFileName), TXTMIRU_FT_TEXT);
}

bool CGrTxtDocument::openFile(LPCTSTR lpFileName)
{
	TCHAR tmp_filename[MAX_PATH+1];
	TCHAR in_filename[MAX_PATH+1];

	if(lpFileName && lpFileName[0] == _T('"')){
		// 引数で、ブランクが含まれている場合 「"」で括られてくるので CGrCSVTextを使って 「"」を外す
		CGrCSVText csv(lpFileName);
		_tcscpy_s(in_filename, csv.GetText(0,0));
		lpFileName = in_filename;
	}
	if(!lpFileName){
		lpFileName = m_fileName.c_str(); // 再読み込み対応(再読み込みの場合は、既存のファイル名を使用)
	}
	{
		CGrCurrentDirectory cur;
		CGrTxtMiru::MoveDataDir(); // Application Dataにカレントを移す
		if(m_arcFileName.empty()){
			TCHAR *lpfilepart;
			if((
				lpFileName[0] == '.'
				|| (!CGrText::Find(lpFileName, _T(":"))
					&& (!CGrText::Find(lpFileName, _T("\\\\")) && !CGrText::Find(lpFileName, _T("//"))) // UNC,URIは対象外
					)
				)
			   && !m_fileName.empty()){
				// 相対パスなら絶対パスに変更
				setCurrentDirectoryFromFilename(m_fileName.c_str());
				::GetFullPathName(lpFileName, sizeof(tmp_filename)/sizeof(TCHAR), tmp_filename, &lpfilepart);
				lpFileName = tmp_filename;
			} else if(!CGrShell::IsURI(lpFileName)){
				setCurrentDirectoryFromFilename(lpFileName);
				::GetFullPathName(lpFileName, sizeof(tmp_filename)/sizeof(TCHAR), tmp_filename, &lpfilepart);
				lpFileName = tmp_filename;
			} /*else {} *//* URI */
			CGrShell::ToPrettyFileName(lpFileName, m_fileName);
		} else {
			// 書庫ファイルの場合、実際のファイル名を取得
			m_fileName = lpFileName;
			std::tstring tmp_arc_filename;
			CGrShell::ToPrettyFileName(lpFileName, tmp_arc_filename);
			LPCTSTR arc_in_filename = nullptr;
			auto pos = getArcfileSplitPos(tmp_arc_filename);
			if(pos != std::tstring::npos){
				tmp_arc_filename[pos] = _T('\0');
				arc_in_filename = &(tmp_arc_filename[pos+1]);
			}
			if(!arc_in_filename || !getArchivePath(tmp_filename, sizeof(tmp_filename)/sizeof(TCHAR), arc_in_filename)){
				return false;
			}
			lpFileName = tmp_filename;
		}
		if(!m_bOpenBookmark){
			if(m_bookMark.Open(m_fileName.c_str())){
				if(m_arcFileName.empty()){ // 書庫ファイルの栞は無視
					// 指定されたファイル名が栞ファイルの時
					m_bookMark.GetTextFileName(m_fileName);
					lpFileName = m_fileName.c_str();
				}
				m_bOpenBookmark = true;
				// 栞ファイルのリンク先が書庫の場合は、再度 書庫ファイルで開きなおす
				if(getArcfileSplitPos(lpFileName) != std::tstring::npos){
					return Open(lpFileName);
				}
			} else {
				std::tstring filename;
				GetFileName(filename);
				m_bookMark.SetTextFileName(filename);
			}
		}
	}
	clear();
	CGrTxtParserMgr parser;
	if (!parser.ReadFile(lpFileName, m_txtBuffer)) {
		m_fileName.clear();
		m_arcFileName.clear();
		return false;
	}
	m_bOpenFile = true;
	// カレントフォルダを、ファイルのあるパスに変更
	setCurrentDirectoryFromFilename(lpFileName);
	// アウトライン
	m_subTitle.AddLeaf(m_txtBuffer);
	//
	CGrTxtMapper::CreateIndex(*this);
	//
	m_bookMark.SetLastWriteTime(m_txtBuffer.GetLastWriteTime());
	m_bookMark.RebuildPageNo();
	m_subTitle.RebuildPageNo();
	/**/
	auto lpJumpTag = CGrText::Find(lpFileName, _T("#"));
	if(lpJumpTag && GetLastPage() <= 1){
		++lpJumpTag;
		int page = m_bookMark.GetPageById(lpJumpTag);
		if(page <= 0){
			page = m_subTitle.GetPageById(lpJumpTag);
		}
		if(page > 1){
			m_bookMark.SetLastPage(page);
		}
	}
	const auto &param = CGrTxtMiru::theApp().Param();
	if(param.GetBoolean(CGrTxtParam::BookMarkAutoSave)){
		Save();
	}
	/**/
	return true;
}
#define MAX_HITMAP_CHARCOUNT (400*50) // チェックする文字は、最大で400字詰めで50ページ
#include <algorithm>
struct HitMap : public CGrTxtBuffer::CGrCharFunc {
	using hit_map = std::map<std::tstring,int>;
	static bool compare(const hit_map::const_iterator& l, const hit_map::const_iterator& r){
		return (l->second > r->second);
	}
	hit_map hit;
	std::tstring &out_str;
	const CGrTxtBuffer &buffer;
	int num;
	int char_count = 0;
	HitMap(std::tstring &s, const CGrTxtBuffer &b, int n) : out_str(s), buffer(b), num(n){}
	~HitMap()
	{
		std::vector<hit_map::const_iterator> f;
		auto hitit=hit.begin(), hite=hit.end();
		for(;hitit != hite; ++hitit){
			f.push_back(hitit);
		}
		out_str.resize(0);

		int len=f.size();
		if(len > num){ len=num; }
		if(len > 0){
			std::partial_sort(f.begin(), f.begin()+len, f.end(), compare); // 使用頻度でソート
			out_str.reserve(len);
			auto *fit=&f[0];
			for(; len>0; --len, ++fit){
				out_str += (*fit)->first;
			}
		}
	}
	virtual bool IsValid(TxtMiru::TextType tt){ return (tt == TxtMiru::TT_TEXT || tt == TxtMiru::TT_OVERLAP_CHAR); }
	virtual bool SetChar(const TxtMiru::TextPoint &cur, LPCTSTR lpSrc, LPCTSTR lpEnd){
		if(char_count > MAX_HITMAP_CHARCOUNT){
			return false;
		}
		++char_count;
		++hit[std::tstring(lpSrc, lpEnd)];
		return true;
	};
};

void CGrTxtDocument::GetUsedTopN(std::tstring &out_str, int num)
{
	m_txtBuffer.ForEachAll(HitMap(out_str, m_txtBuffer, num));
}

TxtMiru::LineInfo &CGrTxtDocument::GetLineInfo(int row)
{
	if(row >= (int)GetLineList().size()){
		throw err_out_of_range;
	}
	return GetLineList()[row];
}

void CGrTxtDocument::UpdateLayout()
{
	const auto &param = CGrTxtMiru::theApp().Param();
	std::tstring filename;
	param.GetText(CGrTxtParam::LayoutFile, filename);
	if(filename.empty()){
		m_layout.SetInitialize();
	} else {
		m_layout.Open(filename.c_str());
	}
	CGrTxtMapper::CreateIndex(*this);
	m_bookMark.RebuildPageNo();
	m_subTitle.RebuildPageNo();
}

#include "LoadDllFunc.h"
int CGrTxtDocument::ConfigurationDlg(HWND hWnd)
{
	CGrLoadDllFunc func(_T("TxtFuncConfig.dll"));
	bool (cdecl *fConfig)(HWND hWnd, LPCTSTR lpDataDir, LPCTSTR type, LPCTSTR name, LPCTSTR lpFileName, CGrTxtFuncIParam *pParam);
	SetProcPtr(fConfig, func.GetProcAddress("TxtFuncLayoutConfig", hWnd));
	if(fConfig){
		auto &&param = CGrTxtMiru::theApp().Param();
		std::tstring filename;
		m_layout.GetFileName(filename);
		fConfig(hWnd, CGrTxtMiru::GetDataPath(), m_layout.LayoutType(), m_layout.OpenLayoutType(), filename.c_str(), &param);
	}
	return 0;
}

bool CGrTxtDocument::SaveBookmark(LPCTSTR lpFileName)
{
	if(m_bOpenFile){
		m_bOpenBookmark = true;
		std::tstring filename;
		CGrShell::ToPrettyFileName(lpFileName, filename);
		m_bookMark.SetFileName(filename);
		m_bookMark.SetLastWriteTime(m_txtBuffer.GetLastWriteTime());
		return m_bookMark.SaveAs(filename.c_str());
	}
	return true;
}

bool CGrTxtDocument::Save(LPCTSTR lpFileName)
{
	if(m_bOpenFile){
		if(!m_bOpenBookmark && !lpFileName){
			std::tstring filename;
			GetFileName(filename);
			m_bookMark.SetTextFileName(filename);
			m_bookMark.SetLastWriteTime(m_txtBuffer.GetLastWriteTime());
		}
		return m_bookMark.Save();
	}
	return true;
}

// 栞追加
void CGrTxtDocument::AddBookmark(int page)
{
	m_bookMark.AddPage(page);
}
int CGrTxtDocument::GetBookmarkPage(int idx)
{
	return m_bookMark.GetPage(idx);
}

bool CGrTxtDocument::Search(LPCTSTR lpSrc, TxtMiru::TextPoint &pos, bool bLoop, bool bUseRegExp, bool bDown)
{
	return m_search.Search(pos, lpSrc, bLoop, bUseRegExp, bDown);
}

const bool CGrTxtDocument::IsBold(const TxtMiru::TextPoint &tp) const
{
	return m_txtBuffer.IsBold(tp);
}

void CGrTxtDocument::ToString(std::tstring &out_str, const TxtMiru::TextPoint &tpb, const TxtMiru::TextPoint &tpe, int max_length) const
{
	m_txtBuffer.ToString(out_str, tpb, tpe, max_length);
	if(out_str.empty()){
		const auto &ppm = m_txtBuffer.GetConstPictPosMap();
		if(ppm.find(tpb) != ppm.end()){
			const auto lpNote = m_txtBuffer.GetChar(tpb);
			out_str = lpNote + _tcslen(lpNote) + 1;
		}
	}
}

void CGrTxtDocument::ToStringRuby(std::tstring &out_str, const TxtMiru::TextPoint &tpb, const TxtMiru::TextPoint &tpe, int max_length) const
{
	m_txtBuffer.ToStringRuby(out_str, tpb, tpe, max_length);
	if(out_str.empty()){
		const auto &ppm = m_txtBuffer.GetConstPictPosMap();
		if(ppm.find(tpb) != ppm.end()){
			const auto lpNote = m_txtBuffer.GetChar(tpb);
			out_str = lpNote + _tcslen(lpNote) + 1;
		}
	}
}

void CGrTxtDocument::CalcTotalPage()
{
	if(m_txtBuffer.GetConstLineList().empty()){
		m_maxPage = 1;
	} else {
		m_maxPage = LineToPage(m_txtBuffer.GetConstLineList().back().iEndCol) | 0x01;
		const auto &textLayout = m_layout.GetConstLayoutList(CGrTxtLayout::LLT_Text);
		int total_layout_h = (textLayout.size() / 2);
		for(const auto &item : m_txtBuffer.GetConstPictPosMap()){
			int page = (item.second.iStartLayout-1) / total_layout_h | 0x01;
			if(m_maxPage < page){
				m_maxPage = page;
			}
		}
	}
}

int CGrTxtDocument::LineToPage(int line/*論理行*/) const
{
	int half_page_line = m_layout.GetMaxLine() / 2;
	if(line < half_page_line){
		return 0;
	}
	return line / max(half_page_line, 1);
}

int CGrTxtDocument::GetTotalPage() const
{
	return m_maxPage;
}

bool CGrTxtDocument::StartPageToTextPoint(TxtMiru::TextPoint &tp, int start_page) const
{
	const auto &textLayout = m_layout.GetConstLayoutList(CGrTxtLayout::LLT_Text);
	const int iLayoutFrom = start_page * textLayout.size() / 2 + 1;
	const int iLayoutTo   = iLayoutFrom + textLayout.size() - 1;
	for(const auto &item : m_txtBuffer.GetConstPictPosMap()){
		const auto &pp = item.second;
		if(iLayoutFrom <= pp.iStartLayout && pp.iStartLayout <= iLayoutTo){
			tp = item.first;
			return true;
		}
	}
	int start_line = m_layout.GetMaxLine() * (start_page/2);
	bool bRet = false;
	bool bRet1st = false;
	auto tp1st = tp;
	for(int iCount=m_layout.GetMaxLine(); iCount>0 && !bRet; --iCount, ++start_line){
		// iLineが改ページなどで、開始位置にない場合があるので 1ページの行内で データが無いかチェック
		bRet = m_txtBuffer.ToTextPoint(tp, start_line);
		if(bRet && start_page != TextPointToPage(tp)){
			bRet = false;
			if(!bRet1st){
				bRet1st = true;
				tp1st = tp;
			}
		}
	}
	if(!bRet && bRet1st){
		tp = tp1st;
		bRet = true;
	}
	return bRet;
}

int CGrTxtDocument::TextPointToPage(const TxtMiru::TextPoint &pos) const
{
	const auto &ppm = m_txtBuffer.GetConstPictPosMap();
	auto it=ppm.find(pos);
	if(ppm.end() != it){
		const auto &textLayout = m_layout.GetConstLayoutList(CGrTxtLayout::LLT_Text);
		return (it->second.iStartLayout-1) / (textLayout.size() / 2);
	}
	const auto &lineList = m_txtBuffer.GetConstLineList();
	if((int)lineList.size() <= pos.iLine){
		return GetTotalPage();
	}
	TxtMiru::TextTurnPos ttp{pos.iIndex, pos.iPos};
	const auto &li = lineList[pos.iLine];
	int col = li.iStartCol-1; // 一件目は、必ず先頭(iIndex=0,iPos=0)のデータが入っているので無視する(posがこれより小さいものはないはず、あれば一つ前の colが返る ※iPos=-1とかの時)
	for(const auto &item : li.tlpTurnList){
		if(ttp < item){
			return LineToPage(col);
		}
		++col;
	}
	return LineToPage(col);
}

const std::tstring &CGrTxtDocument::GetTitle() const
{
	return m_txtBuffer.GetTitle();
}

const std::tstring &CGrTxtDocument::GetAuthor() const
{
	return m_txtBuffer.GetAuthor();
}

const std::tstring &CGrTxtDocument::GetDocInfo() const
{
	return m_txtBuffer.GetDocInfo();
}

bool CGrTxtDocument::SearchNext(TxtMiru::TextPoint &pos)
{
	return m_search.Next(pos);
}

bool CGrTxtDocument::SearchPrev(TxtMiru::TextPoint &pos)
{
	return m_search.Prev(pos);
}

struct NextPosFunc : public CGrTxtBuffer::CGrCharFunc {
	int num;
	TxtMiru::TextPoint &last_point;
	NextPosFunc(TxtMiru::TextPoint &tp, int n) : last_point(tp), num(n){}
	virtual bool IsValid(TxtMiru::TextType tt){ return (tt == TxtMiru::TT_TEXT); }
	virtual bool SetChar(const TxtMiru::TextPoint &cur, LPCTSTR lpSrc, LPCTSTR lpEnd){
		last_point = cur;
		--num;
		if(num < 0){
			return false;
		}
		return true;
	};
};

bool CGrTxtDocument::TextPointNextN(TxtMiru::TextPoint &out_pos, const TxtMiru::TextPoint &pos, int n) const
{
	NextPosFunc npf(out_pos, n);
	m_txtBuffer.ForEach(pos, TxtMiru::TextPoint{pos.iLine + 1, -1, -1}, static_cast<CGrTxtBuffer::CGrCharFunc &&>(npf));
	return true;
}

int CGrTxtDocument::ForwardPage()
{
	if(m_pageForwardHistory.empty()){
		return -1;
	}
	auto tp = m_pageForwardHistory.top();
	m_pageForwardHistory.pop();
	m_pageBackHistory.push(tp);
	return TextPointToPage(tp);
}

int CGrTxtDocument::BackPage()
{
	if(m_pageBackHistory.empty()){
		return -1;
	}
	auto tp = m_pageBackHistory.top();
	m_pageBackHistory.pop();
	m_pageForwardHistory.push(tp);
	return TextPointToPage(tp);
}

void CGrTxtDocument::ShowPropertyDialog(HWND hWnd) const
{
	if(!m_fileName.empty()){
		const auto lpFileName = m_fileName.c_str();
		if(CGrShell::IsURI(lpFileName) || CGrText::Find(lpFileName, _T("|"))/* アーカイブファイル */){
			CGrHTMLPropertyDlg dlg(this);
			dlg.DoModal(hWnd);
		} else {
			SHELLEXECUTEINFO si = { sizeof(SHELLEXECUTEINFO) };
			si.fMask   = SEE_MASK_INVOKEIDLIST | SEE_MASK_FLAG_NO_UI;
			si.hwnd    = hWnd;
			si.lpVerb  = _T("properties";);
			si.lpFile  = lpFileName;
			::ShellExecuteEx(&si);
		}
	}
}
// 画面、栞などに使用するファイル名
// ※書庫の場合にのみ、実ファイルと異なる。
// 通常、実ファイル名と同じものが入る。
const void CGrTxtDocument::GetFileName(std::tstring &str) const
{
	CGrShell::ToPrettyFileName(m_fileName.c_str(), str);
}
const void CGrTxtDocument::GetRealFileName(std::tstring &str) const
{
	if(m_arcFileName.empty()){
		CGrShell::ToPrettyFileName(m_fileName.c_str(), str);
	} else {
		CGrShell::ToPrettyFileName(m_arcFileName.c_str(), str);
		auto pos = getArcfileSplitPos(str);
		if(pos != std::tstring::npos){
			str[pos] = _T('\0');
		}
	}
}

const void CGrTxtDocument::GetCurDir(std::tstring &dir) const
{
	TCHAR buf[_MAX_PATH+1] = {0};
	auto filename = m_fileName;
	if(!m_arcFileName.empty()){
		auto pos = getArcfileSplitPos(filename);
		if(pos != std::tstring::npos){
			const auto arc_in_filename = &(filename[pos+1]);
			getArchivePath(buf, sizeof(buf)/sizeof(TCHAR), arc_in_filename);
			filename = buf;
		}
	}
	CGrShell::GetParentDir(const_cast<TCHAR*>(filename.c_str()), buf);
	CGrShell::ToPrettyFileName(buf, dir);
}

struct FileInfo {
	std::tstring filename;
	std::tstring folder;
	bool operator <(const FileInfo& fi) const{
		int ret = _tcsicmp(folder.c_str(), fi.folder.c_str());
		if(ret == 0){
			ret = _tcsicmp(filename.c_str(), fi.filename.c_str());
		}
		return ret < 0;
	}
};
static void makeFileInfoList(std::vector<FileInfo> &out_fi_list, const std::vector<std::tstring> &in_flist)
{
	out_fi_list.clear();
	out_fi_list.shrink_to_fit();
	for(const auto &item : in_flist){
		auto lpFilename = item.c_str();
		if(CGrTxtDocument::TXTMIRU_FT_TEXT == getFileType(CGrShell::GetFileExtConst(lpFilename))){
			for(; *lpFilename; ++lpFilename){
				if(*lpFilename != '.' && *lpFilename != '/' && *lpFilename != '\\'){
					break;
				}
			}
			if(!(*lpFilename)){
				continue;
			}
			std::tstring str;
			CGrShell::ToPrettyFileName(lpFilename, str);
			lpFilename = str.c_str();
			TCHAR buf[_MAX_PATH+1] = {0};
			FileInfo fi{lpFilename};
			if(CGrShell::GetParentDir(const_cast<TCHAR*>(lpFilename), buf)){
				fi.folder = buf;
			}
			out_fi_list.push_back(fi);
		}
	}
	std::sort(std::begin(out_fi_list), std::end(out_fi_list));
}

bool CGrTxtDocument::Open(LPCTSTR lpFileName)
{
	bool ret = true;
	do {
		if(lpFileName){
			m_bOpenBookmark = false;
			m_bookMark.Clear();
		}
		std::tstring tmp_filename;
		if(!lpFileName && !m_arcFileName.empty()){
			tmp_filename = m_fileName;
			lpFileName = tmp_filename.c_str();
			m_arcFileName.clear();
		}
		if(!lpFileName || !(*lpFileName)){
			ret = false;
			break;
		}
		switch(GetFileType(lpFileName)){
		case TXTMIRU_FT_LINK:
			{
				std::tstring filename;
				if(CGrShell::GetPathFromLink(lpFileName, filename)){
					return Open(filename.c_str());
				}
			}
			break;
		case TXTMIRU_FT_ARCHIVE:
			{
				bool bExtracted = false;
				std::tstring in_filename(lpFileName);
				LPCTSTR arc_in_filename = nullptr;
				{
					// 書庫ファイル展開済みかどうかをチェック
					auto pos = getArcfileSplitPos(in_filename);
					if(pos != std::tstring::npos){
						in_filename[pos] = _T('\0');
						arc_in_filename = &(in_filename[pos+1]);
					}
					if(_tcsicmp(in_filename.c_str(), m_arcFileName.c_str()) == 0){
						bExtracted = true;
					}
				}
				std::tstring curDir;
				{
					TCHAR buf[_MAX_PATH+1] = {0};
					if(!getArchivePath(buf, sizeof(buf)/sizeof(TCHAR), _T(""))){
						return false;
					}
					CGrShell::ToPrettyFileName(buf, curDir);
				}

				CGrCurrentDirectory cur;
				CGrTxtMiru::MoveDataDir(); // Application Dataにカレントを移す

				if(!bExtracted){
					UINT err_no = 0;
					CGrArchive ar;
					//
					deleteTempDir();
					//
					m_arcFileName = in_filename;
					int arcmaxfilesize = 0;
					const auto &param = CGrTxtMiru::theApp().Param();
					param.GetPoints(CGrTxtParam::ArcMaxFileSize, &arcmaxfilesize, 1);
					//
					auto at = CGrArchive::ARTYPE_NONE;
					switch(param.GetFileType(CGrShell::GetFileExtConst(m_arcFileName.c_str()))){
					case CGrTxtParam::FT_Arc7z : at = CGrArchive::ARTYPE_7Z ; break;
					case CGrTxtParam::FT_ArcCab: at = CGrArchive::ARTYPE_CAB; break;
					case CGrTxtParam::FT_ArcLzh: at = CGrArchive::ARTYPE_LZH; break;
					case CGrTxtParam::FT_ArcRar: at = CGrArchive::ARTYPE_RAR; break;
					case CGrTxtParam::FT_ArcZip: at = CGrArchive::ARTYPE_ZIP; break;
					}
					switch(ar.ExtractFull(at, m_arcFileName.c_str(), curDir.c_str(), arcmaxfilesize)){
					case CGrArchive::ERCODE_SUCCESS         : bExtracted = true; break;
					case CGrArchive::ERCODE_ERROR           : err_no = IDS_ERROR_OPEN_ARCFILE ; break;
					case CGrArchive::ERCODE_NOSUPPORT       : err_no = IDS_ERROR_ARC_NOSUPPORT; break;
					case CGrArchive::ERCODE_FILENOTFOUND    : err_no = IDS_ERROR_FILENOTFOUND ; break;
					case CGrArchive::ERCODE_ARCHIVERNOTFOUND: err_no = IDS_ERROR_ARC_NOTFOUND ; break;
					case CGrArchive::ERCODE_ARCHIVERRUNNING : err_no = IDS_ERROR_ARC_RUNNING  ; break;
					default                                 : err_no = IDS_ERROR_ARC          ; break;
					}
					if(err_no != 0){
						std::tstring mess;
						CGrText::FormatMessage(mess, err_no, m_arcFileName.c_str(), ar.GetLastErrorArchiverName());
						CGrMessageBox::Show(GetMainWnd(), mess.c_str(), CGrTxtMiru::AppName());
						ret = false;
						m_arcFileName.clear();
					}
				}
				if(bExtracted){
					ret = false;
					cur.SetRestore(false);
					::SetCurrentDirectory(curDir.c_str());
					std::vector<std::tstring> flist;
					CGrShell::GetFullPathList(flist, _T("."), _T("*.*"));
					std::vector<FileInfo> fi_list;
					makeFileInfoList(fi_list, flist);
					int len = fi_list.size();

					if(arc_in_filename && ::GetFileAttributes(arc_in_filename) != -1){
						;
					} else {
						arc_in_filename = nullptr;
						if(len > 0){
							arc_in_filename = fi_list[0].filename.c_str();
						}
					}
					if(arc_in_filename){
						std::tstring filename;
						::getArcFullPathName(filename, m_arcFileName.c_str(), arc_in_filename);
						ret = openFile(filename.c_str());
					} else {
						std::tstring mess;
						CGrText::FormatMessage(mess, IDS_ERROR_ARC_NOENTRY, m_arcFileName.c_str());
						CGrMessageBox::Show(GetMainWnd(), mess.c_str(), CGrTxtMiru::AppName());
					}
					if(len > 0 && ret){
						int order = 0;
						{
							for(const auto &item : fi_list){
								--order;
								if(arc_in_filename && _tcsicmp(arc_in_filename, item.filename.c_str()) == 0){
									break;
								}
							}
						}
						for(const auto &item : fi_list){
							++order;
							std::tstring filename;
							::getArcFullPathName(filename, m_arcFileName.c_str(), item.filename.c_str());
							//
							std::tstring::size_type str_pos = 0;
							str_pos = filename.find(ARCFILE_SPLIT_CHAR, str_pos);
							if(str_pos == std::tstring::npos){
								m_subTitle.AddLeaf(order, filename.c_str(), filename.c_str());
							} else {
								std::tstring name = filename.c_str() + str_pos + 1;
								m_subTitle.AddLeaf(order, name.c_str(), filename.c_str());
							}
						}
					}
				}
				return ret;
			}
			break;
		case TXTMIRU_FT_SIORI  : break;
		case TXTMIRU_FT_TEXT   : break;
		}
	} while(0);
	//
	m_arcFileName.clear();
	ret = openFile(lpFileName);

	return ret;
}

int CGrTxtDocument::GetPageByLinkNo(int iLinkNo) const
{
	if(iLinkNo >= 0 && iLinkNo < m_subTitle.Count()){
		return m_subTitle.GetConstLeaf(iLinkNo)->GetPage();
	}
	return -1;
}

LPCTSTR CGrTxtDocument::GetLinkFileNameByOrder(int order) const
{
	int len = m_subTitle.Count();
	for(int idx=0; idx<len; ++idx){
		const auto *pLeaf = m_subTitle.GetConstLeaf(idx);
		if(pLeaf->GetLevel() < 0 && pLeaf->GetOrder() == order){
			return pLeaf->GetFileName();
		}
	}
	return nullptr;
}

// TxtBufferへのアクセス
const TxtMiru::TextOffsetMap &CGrTxtDocument::GetConstTextOffsetMap() const { return m_txtBuffer.GetConstTextOffsetMap(); }
const TxtMiru::PictPosMap    &CGrTxtDocument::GetConstPictPosMap   () const { return m_txtBuffer.GetConstPictPosMap   (); }
TxtMiru::TextOffsetMap &CGrTxtDocument::GetTextOffsetMap(){ return m_txtBuffer.GetTextOffsetMap();}
TxtMiru::PictPosMap    &CGrTxtDocument::GetPictPosMap   (){ return m_txtBuffer.GetPictPosMap   ();}

void CGrTxtDocument::OpenError()
{
	CGrTxtParserMgr parser;
	auto filename = m_fileName;
	do {
		m_fileName.clear();
		m_bookMark.Clear();
		m_bOpenBookmark = false;
		clear();
		if(!parser.ReadBuffer(_T("Open Error"), m_txtBuffer)){
			break;
		}
	} while(0);
	m_txtBuffer.SetTitle(filename.c_str());
	CGrTxtMapper::CreateIndex(*this);
	m_fileName = filename;
}

void CGrTxtDocument::SetMainWnd(HWND hWnd)
{
	if(hWnd){
		m_hMainWnd = hWnd;
	} else {
		m_hMainWnd = CGrStdWin::GetWnd();
	}
}
HWND CGrTxtDocument::GetMainWnd()
{
	if(m_hMainWnd){
		return m_hMainWnd;
	} else {
		return CGrStdWin::GetWnd();
	}
}

///////////////////
// スレッドで開く際のパラメータ
struct OpenInfo {
	HWND hWnd;
	CGrTxtDocument *pdoc;
	std::tstring filename;
	CGrDocOpenProcCallbak callback;
	LPARAM lParam;
};

unsigned __stdcall CGrTxtDocument::OpenProc(void *lpParam)
{
	if (CoInitializeEx(NULL, COINIT_APARTMENTTHREADED) == S_OK) {
		;
	}
	if (OleInitialize(NULL) == S_OK) {
		;
	}
	auto *pinfo = static_cast<OpenInfo*>(lpParam);
	auto *pdoc = pinfo->pdoc;
	if(!pinfo->pdoc){
		pdoc = new CGrTxtDocument();
		pdoc->GetBookmark().SetParamInteger(_T("IsUpdateCheck"), 1);
	}
	pdoc->SetMainWnd(pinfo->hWnd);
	auto callback = pinfo->callback;
	bool ret = pdoc->Open(pinfo->filename.c_str());
	if(callback){
		callback(ret ? 1 : 0, pinfo->lParam);
	}
	if(!pinfo->pdoc){
		delete pdoc;
	}
	delete pinfo;
	CoUninitialize();
	OleUninitialize();
	return 0;
}

#include "OpenUrlDlg.h"
HANDLE CGrTxtDocument::OpenThread(HWND hWnd, LPCTSTR lpFilename, CGrDocOpenProcCallbak callback)
{
	auto *pinfo = new OpenInfo{
		hWnd, this, lpFilename, callback, 0
		};
	CGrOpenURLDlg dlg;
	dlg.Show(hWnd, lpFilename);
	unsigned int threadid = 0;
	auto h = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0 , OpenProc, pinfo, 0, &threadid));
	if (h) {
		bool bret = true;
		DWORD ret = 0;
		do {
			ret = MsgWaitForMultipleObjects(1, &h, FALSE, INFINITE, QS_ALLEVENTS);
			if (ret == (WAIT_OBJECT_0 + 1)) {
				MSG msg;
				while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
					if (msg.message == WM_QUIT) {
						break;
					}
					PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
					if (msg.message == WM_COMMAND || CGrTxtMiru::theApp().IsDialogMessage(msg.hwnd, msg)) {
						continue;
					}
					TranslateMessage(&msg);
					DispatchMessage(&msg);
					if (CGrOpenURLDlg::IsWaitting()) {
						continue;
					}
					else {
						PostThreadMessage(threadid, WM_DESTROY, 0, 0);
						break;
					}
				}
			}
		} while (ret != WAIT_OBJECT_0);
		CloseHandle(h);
	}
	return reinterpret_cast<HANDLE>(1);
}

#include <wininet.h>
extern "C" {
__declspec(dllexport) HANDLE TxtMiruFunc_UpdateCheck(HWND hWnd, LPCTSTR lpFilename, CGrDocOpenProcCallbak callback, LPARAM lParam, unsigned int *threadid)
{
	auto *pinfo = new OpenInfo{
		hWnd, nullptr, lpFilename, callback, lParam
		};
	if(lpFilename){
		DeleteUrlCacheEntry(lpFilename);
	}
	return reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0 , CGrTxtDocument::OpenProc, pinfo, 0, threadid));
}

using CGrDocSearchProcCallbak = void (__stdcall *)(int page, LPCTSTR lpSrc, LPARAM lParam);
class CGrTxtMiruFuncSearchData
{
private:
	std::tstring m_search_word;
	bool m_bUseRegExp;
	LPARAM m_lParam;
	CGrTxtDocument m_doc;
	CGrDocSearchProcCallbak m_pFunc;
public:
	CGrTxtMiruFuncSearchData(LPCTSTR lpStr, bool bUseRegExp, CGrDocSearchProcCallbak func, LPARAM lParam)
	: m_search_word(lpStr), m_bUseRegExp(bUseRegExp), m_pFunc(func), m_lParam(lParam)
	{
	}
	virtual ~CGrTxtMiruFuncSearchData(){}
	CGrTxtDocument &GetDocment(){ return m_doc; }
	static void __stdcall DocOpenProcCallbak(int ret, LPARAM lParam)
	{
		auto *lpSearch = reinterpret_cast<CGrTxtMiruFuncSearchData*>(lParam);
		if(ret){
			auto &&doc = lpSearch->m_doc;
			TxtMiru::TextPoint pos;
			bool bSearch = doc.Search(lpSearch->m_search_word.c_str(), pos, false, lpSearch->m_bUseRegExp, true);
			while(bSearch){
				std::tstring line_str;
				auto next_pos = pos;
				++next_pos.iLine;
				doc.ToString(line_str, pos, next_pos, 100);
				int page_no = doc.TextPointToPage(pos);
				lpSearch->m_pFunc(page_no, line_str.c_str(), lpSearch->m_lParam);
				bSearch = doc.SearchNext(pos);
			};
		}
		lpSearch->m_pFunc(-1, nullptr, lpSearch->m_lParam);
		delete lpSearch;
	}
};

__declspec(dllexport) HANDLE TxtMiruFunc_Search(HWND hWnd, LPCTSTR lpFilename, LPCTSTR lpSrc, bool bUseRegExp, CGrDocSearchProcCallbak func, LPARAM lParam, unsigned int *threadid)
{
	auto *lpData = new CGrTxtMiruFuncSearchData(lpSrc, bUseRegExp, func, lParam);
	auto *pinfo = new OpenInfo{
		hWnd, &(lpData->GetDocment()), lpFilename, CGrTxtMiruFuncSearchData::DocOpenProcCallbak, reinterpret_cast<LPARAM>(lpData)
		};
	if(lpFilename){
		DeleteUrlCacheEntry(lpFilename);
	}
	return reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0 , CGrTxtDocument::OpenProc, pinfo, 0, threadid));
}

__declspec(dllexport) bool TxtMiruFunc_SearchCurrent(void *pDocPtr, LPCTSTR lpSrc, TxtMiru::TextPoint &pos, bool bLoop, bool bUseRegExp, bool bDown)
{
	auto *pDoc = static_cast<CGrTxtDocument*>(pDocPtr);
	return pDoc->Search(lpSrc, pos, bLoop, bUseRegExp, bDown);
}
class cdecl CGrTxtMiruFunc_EachAllTool : public CGrTxtFuncIEachAllTool
{
private:
	CGrTxtDocument *m_pDoc = nullptr;
	std::tstring m_buf;
public:
	const TxtMiru::TextPoint *mp_cur = nullptr;
	const TxtMiru::TextInfo *mp_text_info = nullptr;
public:
	CGrTxtMiruFunc_EachAllTool(CGrTxtDocument *pDoc) : m_pDoc(pDoc){}
	virtual ~CGrTxtMiruFunc_EachAllTool(){}

	virtual const TxtMiru::TextPoint *CurTextPoint(){ return mp_cur; }
	virtual LPCTSTR String(){ return mp_text_info->str.c_str(); }
	virtual size_t StringLength(){ return mp_text_info->str.size(); }
	virtual WORD CharType(){ return mp_text_info->chrType; }
	virtual WORD TextType(){ return mp_text_info->textType; }
	virtual TxtMiru::TextListPos BeginTextListPos(){ return mp_text_info->tpBegin; }
	virtual TxtMiru::TextListPos EndTextListPos(){ return mp_text_info->tpEnd; }
	virtual int Page(){ return m_pDoc->TextPointToPage(TxtMiru::TextPoint{mp_cur->iLine, mp_text_info->tpBegin.iIndex, mp_text_info->tpBegin.iPos}); }
	virtual LPCTSTR RangeString()
	{
		const auto &textBuffer = m_pDoc->GetTxtBuffer();
		textBuffer.ToString(m_buf,
							TxtMiru::TextPoint{mp_cur->iLine, mp_text_info->tpBegin.iIndex, mp_text_info->tpBegin.iPos},
							TxtMiru::TextPoint{mp_cur->iLine, mp_text_info->tpEnd  .iIndex, mp_text_info->tpEnd  .iPos},
							-1);
		return m_buf.c_str();
	}
};
using CGrDocEachAllProcCallbak = void (__stdcall *)(CGrTxtFuncIEachAllTool *pTool, LPARAM lParam);
__declspec(dllexport) bool TxtMiruFunc_EachAll(void *pDocParam, CGrDocEachAllProcCallbak func, LPARAM lParam)
{
	if(!pDocParam){
		return false;
	}
	auto *pDoc = static_cast<CGrTxtDocument*>(pDocParam);
	const auto &textBuffer = pDoc->GetTxtBuffer();
	struct CGrTypeFuncAll : public CGrTxtBuffer::CGrTypeFunc, CGrTxtMiruFunc_EachAllTool
	{
		CGrDocEachAllProcCallbak func;
		LPARAM lParam;
		CGrTypeFuncAll(CGrTxtDocument *pDoc, CGrDocEachAllProcCallbak f, LPARAM l) : CGrTxtMiruFunc_EachAllTool(pDoc), func(f), lParam(l)
		{
		}
		virtual bool IsValid(TxtMiru::TextType tt){
			return true;
		}
		virtual bool SetType(const TxtMiru::TextPoint &cur, const TxtMiru::TextInfo &text_info){
			mp_cur = &cur;
			mp_text_info = &text_info;
			func(this, lParam);
			return true;
		};
	};
	textBuffer.ForEachAll(CGrTypeFuncAll(pDoc, func, lParam));
	return true;
}
}
