#pragma warning( disable : 4786 )

#include "AozoraTxtParser.h"
#include "Mlang.h"
#include "shell.h"
#include "TxtMiru.h"
#include "reverse_iterate.h"
#include "TxtMiruTextType.h"
#include "Win32Wrap.h"

//#define __DBG__
#include "Debug.h"

static const int l_DetectInputCodepageMaxChar = 512;

#include "AozoraData.h"

static LPCTSTR l_lpTagStartStr = _T("�m��");
static LPCTSTR l_PictureType[] = { _T("�}"), _T("�n�}"), _T("�G"), _T("�}�G"), _T("�\"), _T("�ʐ^"), _T("�A�C�R��") };
struct PicturePostion {
	LPCTSTR name;
	TxtMiru::TextType textType;
};
static PicturePostion l_PicturePositionType[] = {
	{_T("���"), TxtMiru::TT_PICTURE_LAYOUT   },
	{_T("����"  ), TxtMiru::TT_PICTURE_LAYOUT   },
	{_T("����"), TxtMiru::TT_PICTURE_LAYOUT   },
	{_T("�i"    ), TxtMiru::TT_PICTURE_LAYOUT   },
	{_T("��"    ), TxtMiru::TT_PICTURE_HALF_PAGE},
	{_T("���J��"), TxtMiru::TT_PICTURE_FULL_PAGE},
};

CGrAozoraTxtParser::CGrAozoraTxtParser()
{
	const auto &param = CGrTxtMiru::theApp().Param();
	m_bUseOverlapChar = param.GetBoolean(CGrTxtParam::UseOverlapChar);
}

CGrAozoraTxtParser::~CGrAozoraTxtParser()
{
}

static int GetPrevIndexTexts(const TxtMiru::TextInfoList &text_list, int iBeginIndex)
{
	for(int iIndex=text_list.size()-1; iIndex>=iBeginIndex; --iIndex){
		if(TxtMiruType::isTextOrRotateNumOrKuChar(text_list[iIndex].textType)){
			return iIndex;
		}
	}
	return iBeginIndex;
}

bool CGrAozoraTxtParser::convertNoteStr(std::tstring &note, LPCTSTR lpNote)
{
	if(!lpNote){
		lpNote = note.c_str();
	}
	if(CGrText::Find(lpNote, l_lpTagStartStr)){
		CGrTxtBuffer buffer;
		TxtMiru::TextInfoList note_text_list;
		CGrAozoraTxtParser atp;
		atp.parse(buffer, note_text_list, lpNote, lpNote + _tcslen(lpNote));
		if(note_text_list.size() > 0){
			note.clear();
			for(const auto &ti : note_text_list){
				note += ti.str;
			}
			return true;
		}
	}
	return false;
}

void CGrAozoraTxtParser::pushNote(TxtMiru::TextInfoList &text_list, LPCTSTR target, LPCTSTR note, TxtMiru::TextType textType)
{
	TxtMiru::TextListPos tpBegin;
	std::tstring tmp_note;
	if(convertNoteStr(tmp_note, note)){
		note = tmp_note.c_str();
	}
	if(CGrText::Find(target, l_lpTagStartStr)){
		CGrTxtBuffer buffer;
		TxtMiru::TextInfoList target_text_list;
		CGrAozoraTxtParser atp;
		atp.parse(buffer, target_text_list, target, target + _tcslen(target));
		int taeget_text_list_len = target_text_list.size();
		int line_text_list_len   = text_list.size();
		if(line_text_list_len >= taeget_text_list_len && taeget_text_list_len > 0){
			if(taeget_text_list_len == 1){
				TxtMiru::TextListPos tpEnd;
				tpEnd.iIndex = -1;
				const auto *target_it = &target_text_list[0];
				for(int line_idx = line_text_list_len-1; line_idx >= taeget_text_list_len; --line_idx){
					const auto *it = &text_list[line_idx];
					if(it->textType == target_it->textType){
						auto lpSrc = it->str.c_str();
						auto lpFind = CGrText::RFind(lpSrc, lpSrc + CGrText::CharLen(lpSrc), target_it->str.c_str());
						if(lpFind){
							tpBegin.iIndex = line_idx;
							tpBegin.iPos   = CGrText::CharLen(lpFind) - CGrText::CharLen(lpSrc);
							tpEnd.iIndex   = line_idx;
							tpEnd.iPos     = tpBegin.iPos + CGrText::CharLen(target_it->str.c_str());
							break;
						}
					}
				}
				if(tpEnd.iIndex >= 0){
					text_list.push_back(TextInfoSpec(note, textType, tpBegin.iIndex, tpBegin.iPos, tpEnd.iIndex, tpEnd.iPos));
				}
			} else {
				// ������v�̈ʒu���擾
				for(; line_text_list_len >= taeget_text_list_len; --line_text_list_len){
					const auto *target_it = &target_text_list[taeget_text_list_len-1];
					const auto *it = &text_list[line_text_list_len-1];
					if(it->textType == target_it->textType && CGrText::isMatchChar(it->str.c_str(), target_it->str.c_str())){
						bool bMatched0 = false;
						--target_it;
						TxtMiru::TextListPos tpEnd;
						tpEnd.iIndex = line_text_list_len-1;
						tpEnd.iPos   = CGrText::CharLen(target_it->str.c_str());
						int line_idx = line_text_list_len-1-1;
						for(int note_idx = taeget_text_list_len-1-1; note_idx>=0; --note_idx, --target_it, --line_idx){
							bool bMatched1 = false;
							for(; line_idx>=0; --line_idx){
								it = &text_list[line_idx];
								if(it->textType == TxtMiru::TT_TEXT || it->textType == TxtMiru::TT_OTHER){
									if(note_idx == 0){
										if(it->textType == target_it->textType && CGrText::isMatchLast(it->str.c_str(), target_it->str.c_str())){
											bMatched1 = true;
											tpBegin.iIndex = line_idx;
											tpBegin.iPos   = CGrText::CharLen(it->str.c_str()) - CGrText::CharLen(target_it->str.c_str());
										}
									} else {
										if(it->textType == target_it->textType && CGrText::isMatchChar(target_it->str.c_str(), it->str.c_str())){
											bMatched1 = true;
										}
									}
									break;
								}
							}
							if(bMatched1){
								bMatched0 = true;
							} else {
								bMatched0 = false;
								break;
							}
						}
						if(bMatched0){
							text_list.push_back(TextInfoSpec(note, textType, tpBegin.iIndex, tpBegin.iPos, tpEnd.iIndex, tpEnd.iPos));
							break;
						}
					}
				}
			}
			return;
		}
	}
	if(!GetRFindText(tpBegin, text_list, target)){
		return;
	}
	int offset = GetTextListOffset(tpBegin, text_list) + CGrText::CharLen(target) -1;
	auto tpEnd = GetTextListPos(offset, text_list);
	if(tpEnd.iIndex >= 0){
		text_list.push_back(TextInfoSpec(note, textType, tpBegin.iIndex, tpBegin.iPos, tpEnd.iIndex, tpEnd.iPos));
	}
}

static int Str2Int(std::tstring &str)
{
	auto dnumber = _T("�X�W�V�U�T�S�R�Q�P�O");
	std::tstring buf;
	auto lpNextSrc = str.c_str();
	while(*lpNextSrc){
		auto lpSrc = lpNextSrc;
		lpNextSrc = CGrText::CharNext(lpSrc);
		std::tstring ch(lpSrc, lpNextSrc);
		auto lpFindStr = CGrText::Find(dnumber, ch.c_str());
		if(lpFindStr){
			buf.append(1, '0' + CGrText::CharLen(lpFindStr) - 1);
		} else {
			buf += ch;
		}
	}
	return _ttol(buf.c_str());
}
#include "JScript.h"

bool CGrAozoraTxtParser::ReadFile(LPCTSTR lpFileName, CGrTxtBuffer &buffer)
{
	bool bret = true;
	m_lastWriteTime.clear();
	auto hFile = ::CreateFile(lpFileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, 0);
	if(hFile == INVALID_HANDLE_VALUE){
		return false;
	}

	DWORD sizeHigh;
	// �T�C�Y���擾���ė̈���m�ۂ��܂��B
	auto fsize = ::GetFileSize(hFile, &sizeHigh);
	if(fsize > 0){
		HANDLE hHeap = INVALID_HANDLE_VALUE;
		BYTE *data = nullptr;
		do {
			DWORD dw;
			hHeap = ::HeapCreate(0, 0, 0);
			if (hHeap) {
				data = static_cast<BYTE*>(::HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sizeof(BYTE) * (fsize + 1)));
				if (0 == ::ReadFile(hFile, data, fsize, &dw, nullptr)) {
					break;
				}
				bret = ReadDataBuffer(data, fsize, buffer, lpFileName);
			}
		} while(0);
		if(hHeap && hHeap != INVALID_HANDLE_VALUE){
			::HeapFree(hHeap, 0, data);
			::HeapDestroy(hHeap);
		}
	}
	::CloseHandle(hFile);

	WIN32_FIND_DATA wfd;
	if(CGrShell::getFileInfo(lpFileName, wfd)){
		SYSTEMTIME stFileTime;
		FILETIME ftLocalFileTime;
		FileTimeToLocalFileTime(&wfd.ftLastWriteTime, &ftLocalFileTime);
		FileTimeToSystemTime(&ftLocalFileTime, &stFileTime);
		TCHAR buf[512];
		_stprintf_s(buf, _T("%04d/%02d/%02d %02d:%02d:%02d"),
					stFileTime.wYear , stFileTime.wMonth ,
					stFileTime.wDay , stFileTime.wHour ,
					stFileTime.wMinute , stFileTime.wSecond);
		m_lastWriteTime = buf;
	}
	return bret;
}

void CGrAozoraTxtParser::addLine(CGrTxtBuffer &buffer, LPCTSTR lpSrc, LPCTSTR lpEnd, bool bTrans, int iFileLineNo/* = -1*/)
{
	if(m_csComment == CSTAT_SKIP_LINE){
		m_csComment = CSTAT_NO_MORE;
		// �y�e�L�X�g���Ɍ����L���ɂ��āz�̌��
		// �P�s�J���āA�{��������
		if(lpSrc == lpEnd-1){
			return;
		}
	}
	TxtMiru::LineInfo li;
	std::tstring line;
	if(bTrans){
		line = std::tstring(lpSrc, lpEnd);
		// �A�N�Z���g������UTF�ɕϊ�
		ConvertAccentChar(line);
		lpSrc = line.c_str();
		lpEnd = lpSrc + line.size();
	}
	li.iFileLineNo = iFileLineNo;
	li.text_list.reserve((lpEnd - lpSrc)/2);// ��Ƀ��������m�ۂ��đ��x���グ��
	if(parse(buffer, li.text_list, lpSrc, lpEnd)){
		buffer.AddLineInfo(li);
	}
	if(m_bIndentReset && !m_Indent.empty()){
		m_bIndentReset = false;
		m_Indent.pop_back();
		if(!m_Indent.empty()){
			const auto &indent = m_Indent.back();
			buffer.MoveBackLineInfo(
				TxtMiru::LineInfo {
					iFileLineNo, 0,0,0,0,0,
					{TextInfoSpec(TxtMiru::TT_INDENT_START, indent.iIndent1st, indent.iIndent2nd)}
				}
				);
		}
	}
	if(m_bRIndentReset && !m_RIndent.empty()){
		m_bRIndentReset = false;
		m_RIndent.pop_back();
		if(!m_RIndent.empty()){
			const auto &indent = m_RIndent.back();
			buffer.MoveBackLineInfo(
				TxtMiru::LineInfo {
					iFileLineNo, 0,0,0,0,0,
					{TextInfoSpec(TxtMiru::TT_RINDENT_START, indent.iIndent1st, indent.iIndent2nd)}
				}
				);
		}
	}
}

static bool isCommentLine(LPCTSTR lpNextSrc)
{
	if(*lpNextSrc != '-'){
		return false;
	}
	int iCnt = 0;
	for(;*lpNextSrc; lpNextSrc=CGrText::CharNext(lpNextSrc)){
		if(*lpNextSrc != '-'){
			return (*lpNextSrc == '\r' || *lpNextSrc == '\n');
		}
		++iCnt;
	}
	return iCnt > 5;
}

// �}�b�`���Ȃ������ꍇ�A'�i','�j'�������p�^�[�����`�F�b�N(�ŏ��̋󔒂ŋ�؂�)�m���}10 fig42154_01.png�n
static bool isPictureOther(LPCTSTR lpSrc)
{
	std::tstring str;
	for(const auto p : l_PictureType){
		auto lpFindSrcTmp = CGrText::GetShortMatchStr(str, lpSrc, p, _T(" "));
		if(lpFindSrcTmp && CGrText::Find(lpFindSrcTmp, _T("."))){
			return true;
		}
	}
	return false;
}
// �m�����V��̐}�ifig42154_01.png�A��321�~�c123�j����n
// �m���u��ꎵ���@�����S���d�m���������䗝���i���ʁv�̃L���v�V�����t���̐}�ifig4990_07.png�A��321�~�c123�j����n
// �m���}�ifig42154_01.png�j����n
// �m���}�P�ifig42154_01.png�j����n
// �m���}10�ifig42154_01.png�j����n
static bool isPicture(LPCTSTR lpSrc)
{
	std::tstring str;
	for(const auto p : l_PictureType){
		auto lpFindSrcTmp = CGrText::GetShortMatchStr(str, lpSrc, p, _T("�i"));
		if(lpFindSrcTmp){
			if(CGrText::GetShortMatchStr(str, lpFindSrcTmp, _T("�i"), _T("�j"))){
				return true;
			}
		}
	}
	// �ixxxxx�Axxxx�~xxxxx�j �̎����摜�Ƃ��ď���
	auto lpBeginPict = CGrText::Find(lpSrc, _T("�i"));
	if(lpBeginPict){
		auto lpNextPCStr = CGrText::Find(lpBeginPict, _T("�A"));
		if(lpNextPCStr){
			if(CGrText::Find(lpNextPCStr, _T("�~"))){
				return true;
			}
		}
	}
	if(CGrText::GetShortMatchStr(str, lpSrc, _T("�i"), _T("�j"))){
		std::transform(str.begin(), str.end(), str.begin(), ::tolower);
		for(const auto ftype : {_T(".png"), _T(".jpg"), _T(".jpeg"), _T(".gif"), _T(".bmp")}){
			if(CGrText::Find(str.c_str(), ftype)){
				return true;
			}
		}
	}
	return isPictureOther(lpSrc);
}

static void incTextListIndexPos(int &ruby_pos_r, TxtMiru::TextInfoList &text_list, const int check_iIndex, int inc)
{
	if(ruby_pos_r >= check_iIndex){
		ruby_pos_r += inc;
	}
	int tmp_iIndex_len = text_list.size();
	for(int tmp_iIndex=check_iIndex; tmp_iIndex<tmp_iIndex_len; ++tmp_iIndex){
		if(text_list[tmp_iIndex].tpBegin.iIndex >= check_iIndex){
			text_list[tmp_iIndex].tpBegin.iIndex += inc;
		}
		if(text_list[tmp_iIndex].tpEnd.iIndex >= check_iIndex){
			text_list[tmp_iIndex].tpEnd.iIndex += inc;
		}
	}
}

static const int max_info_blank_line = 3;
bool CGrAozoraTxtParser::parse(CGrTxtBuffer &buffer, TxtMiru::TextInfoList &text_list, LPCTSTR lpLine, LPCTSTR lpEnd)
{
	auto &line_list = buffer.GetLineList();
	// �{�����I�����Ă���ꍇ�A�ȍ~���ׂĖ���
	if(m_bEndofContents){
		text_list.push_back(TextInfoHalfChar(lpLine, TxtMiru::TT_TEXT));
		return true;
	} else if(line_list.size() > max_info_blank_line && CGrText::isMatchChar(lpLine, _T("��{�F"))){
		// ��{���菈��
		int iLine = line_list.size() - 1;
		auto *pll = &line_list[iLine];
		int iLineNum;
		for(iLineNum = max_info_blank_line; iLineNum>0; --iLineNum, --pll){
			const auto &tl = pll->text_list;
			if(tl.empty() || tl[0].str.empty()){
				continue;
			}
			break;
		}
		if(iLineNum == 0){
			++pll;
			if(pll->text_list.empty()){
				pll->text_list.push_back(TextInfoHalfChar(_T(""), TxtMiru::TT_ENDOFCONTENTS));
			} else {
				pll->text_list[0].textType = TxtMiru::TT_ENDOFCONTENTS;
			}
			text_list.push_back(TextInfoHalfChar(lpLine, TxtMiru::TT_TEXT));
			m_bEndofContents = true;
			return true;
		}
	}
	// �R�����g�`�F�b�N
	// �u---------------------------------------�v�Ǝ��̍s��
	// �u�y�e�L�X�g���Ɍ����L���ɂ��āz�v�Ƒ����ŏ��̑g�ݍ��킹����
	// �u---------------------------------------�v���͈̔͂��R�����g�Ƃ݂Ȃ�
	//
	// Contents>---------------------------------------    <- ���̍s�� �y�e�L�X�g���Ɍ����L���ɂ��āz ��������Ζ{���Ƃ݂Ȃ�
	// Comment >---------------------------------------
	// Comment >�y�e�L�X�g���Ɍ����L���ɂ��āz
	// Comment >
	// Comment >---------------------------------------
	// Contents>
	// Contents>---------------------------------------    <- �Q��ڂ͖���
	// Contents>�y�e�L�X�g���Ɍ����L���ɂ��āz
	// Contents>
	// Contents>---------------------------------------
	//
	//   ��s�S�� '-' �̎��ɂ́A�R�����g�Ƃ��Ĕ���
	if(::isCommentLine(lpLine)){
		if(m_csComment == CSTAT_CONTENTS){
			m_csComment = CSTAT_BEGIN_COMMENT;
			text_list.push_back(TextInfoHalfChar(lpLine, TxtMiru::TT_COMMENT_BEGIN));
			return true;
		} else if(m_csComment == CSTAT_COMMENT){
			m_csComment = CSTAT_SKIP_LINE; // �Q��ڂ͖���
			text_list.push_back(TextInfoHalfChar(lpLine, TxtMiru::TT_COMMENT_END));
			return true;
		}
	}
	if(m_csComment == CSTAT_BEGIN_COMMENT){
		if(CGrText::Find(lpLine, _T("�y�e�L�X�g���Ɍ����L���ɂ��āz"))){
			m_csComment = CSTAT_COMMENT; // �R�����g�J�n
		} else {
			// �O��̃R�����g�J�n�́A�R�����g�Ƃ݂͂Ȃ��Ȃ��B
			int idx = line_list.size()-1;
			if(idx >= 0 && line_list[idx].text_list.size() == 1 && line_list[idx].text_list[0].textType == TxtMiru::TT_COMMENT_BEGIN){
				auto &&ti = line_list[idx].text_list[0];
				ti.textType = TxtMiru::TT_TEXT;
				ti.chrType  = CGrText::GetStringTypeEx(_T("-"), 1);
			}
			if(::isCommentLine(lpLine)){
				text_list.push_back(TextInfoHalfChar(lpLine, TxtMiru::TT_COMMENT_BEGIN));
				return true;
			} else {
				m_csComment = CSTAT_CONTENTS;
			}
		}
	}
	if(m_csComment == CSTAT_COMMENT){
		text_list.push_back(TextInfoHalfChar(lpLine,  TxtMiru::TT_COMMENT));
		return true;
	}
	//
	WORD chrType = 0xffff;
	LPCTSTR lpTextStart = nullptr;
	LPCTSTR lpTextEnd   = nullptr;
	int ruby_pos_r = -1; // TT_RUBY_SEP     �u�b�v�̈ʒu
	int ruby_pos_l = -1; // TT_RUBY_L_START �u�m�����Ƀ��r�t���n�v�̈ʒu
	int note_pos_r = -1; // TT_NOTE_START   �u�m�����L�t���n�v�̈ʒu
	int note_pos_l = -1; // TT_NOTE_START_L �u�m�����ɒ��L�t���n�v�̈ʒu

	const auto &ad = CGrAozoraData::Data();
	const auto *it_plb = ad.GetPairTTList();
	const auto it_pl_len = ad.GetPairTTListLength();
	const auto *it_slb = ad.GetTTList();
	const auto it_sl_len = ad.GetTTListLength();
	const auto *text_type_name = ad.GetTT();
	const auto text_type_name_len = ad.GetTTLength();

	auto lpNextSrc = lpLine;
	bool bIndentReset = false; // ���s�݂̂̃C���f���g�w��L��
	bool bSplitLine = false;
	//
	//MatchPairStr tag_mps;
	while(lpNextSrc < lpEnd && *lpNextSrc){
		auto lpSrc = lpNextSrc;
		lpNextSrc = CGrText::CharNext(lpSrc);

		LPCTSTR lpFindSrc = nullptr;
		const MatchPairStr *pmps = nullptr;
		// �p�^�[���̒��ōŒZ��v�ɂȂ�p�^�[����I��
		if(ad.IsPairTTListFistChar(lpSrc)){
			const auto *it_pl=it_plb;
			for(int it_pl_idx=it_pl_len; it_pl_idx>0; --it_pl_idx, ++it_pl){
				const auto &mps = (*it_pl);
				auto lpFindSrcTmp = CGrText::GetFirstShortMatchPos(lpSrc, mps.first, mps.second);
				if(!lpFindSrcTmp){ continue; } // lpFindSrcTmp �́Asecond �̕����p�^�[�� �J�n�ʒu
				if(mps.second.str_len == 1 && _T('�n') == mps.second[0]){
					// �m�����ɒ��L�t���n�����[��m�����Ɂu�{�g�ّ��Y���m���R�g�A1-2-24�n���m���R�g�A1-2-24�n�v�̒��L�t���I���n
					// �̂悤�ɁA�m���n�̒��Ɂm���n������q�ɂȂ��Ă�����̂��X�L�b�v������(1���x���̂�)
					auto lpCheckStartStr = lpSrc+mps.first.str_len;
					do {
						auto lpReStartPos = CGrText::RFind(lpCheckStartStr, lpFindSrcTmp-1, l_lpTagStartStr);
						if(lpReStartPos && lpReStartPos > lpCheckStartStr){
							auto lpNextFindSrcTmp = CGrText::Find(lpFindSrcTmp+1, mps.second);
							if(lpNextFindSrcTmp){
								lpCheckStartStr = lpFindSrcTmp + 1;
								lpFindSrcTmp = lpNextFindSrcTmp;
							} else {
								break;
							}
						} else {
							break;
						}
					} while(1);
				}
				if(!lpFindSrc || lpFindSrc > lpFindSrcTmp){
					if(mps.first.type() == TxtMiru::TT_PICTURE_LAYOUT && !isPicture(CGrText::CharNextN(lpSrc, mps.first.len()))){
						continue;
					}
					lpFindSrc = lpFindSrcTmp;
					pmps = &mps;
				}
			}
		}
		// �s�𕪊�
		if(bSplitLine){
			buffer.MoveBackLineInfo(TxtMiru::LineInfo{ -1,0,0,0,0,0,text_list });
			//
			text_list.clear();
			ruby_pos_r = -1;
			ruby_pos_l = -1;
			bSplitLine = false;
		}
		//
		if(lpFindSrc && pmps){
			auto textType = pmps->first.type();
			if(lpTextStart){
				text_list.push_back(TextInfoLPTSTR(lpTextStart, lpTextEnd, chrType));
				lpTextStart = nullptr;
			}
			auto lpTTSrc = CGrText::CharNextN(lpSrc, pmps->first.len());
			auto lpTTSrcEnd = CGrText::CharNext(lpFindSrc)-1;
			std::tstring tt_str(lpTTSrc, lpTTSrcEnd);
			bool bno_empty_tt_str = (lpTTSrc != lpTTSrcEnd);
			TxtMiru::TextListPos lastTLP;
			bool bGetLastTLP = GetRFindTextType(lastTLP, text_list, TxtMiru::TT_TEXT);
			// �}�G�̃`�F�b�N�ǉ�(�S�Ẵ`�F�b�N�Ɉ���������Ȃ��ꍇ�ɁA����炵���w�肪���邩�ǂ�����)
			if(textType == TxtMiru::TT_OTHER && isPictureOther(tt_str.c_str())){
				textType = TxtMiru::TT_PICTURE_LAYOUT;
			}
			switch(textType){
			case TxtMiru::TT_TXTMIRU:
				if(bno_empty_tt_str){
					auto lpttstr = tt_str.c_str();
					if(CGrText::isMatchChar(lpttstr, _T("Link:"))){
						lpttstr += 5; //_T("Link:");
						auto &&tp = m_RangeTextPoint[TxtMiru::TT_LINK];
						text_list.push_back(TxtMiru::TextInfo(lpttstr, tp.iPos, -1, -1, -1, -1, TxtMiru::TT_LINK));
						tp.iIndex = text_list.size()-1;
						++tp.iPos;
					} else if(CGrText::isMatchChar(lpttstr, _T("LinkEnd"))){
						auto &&tp = m_RangeTextPoint[TxtMiru::TT_LINK];
						if(tp.iPos > 0 && tp.iIndex >= 0 && tp.iIndex < static_cast<signed int>(text_list.size()) && text_list[tp.iIndex].textType == TxtMiru::TT_LINK){
							if(text_list[tp.iIndex].str.size() == 0){
								auto tis = TextInfoSpec(_T(""), TxtMiru::TT_LINK, tp.iIndex, 0, text_list.size(), -1);
								{
									auto e = text_list.end();
									for(auto ii = text_list.begin() + tp.iIndex+1; ii!=e; ++ii){
										tis.str += ii->str;
									}
								}
								text_list[tp.iIndex].str = tis.str;
								tis.chrType = text_list[tp.iIndex].chrType;
								text_list.push_back(tis);
							} else {
								text_list.push_back(TxtMiru::TextInfo(text_list[tp.iIndex].str.c_str(), tp.iPos, tp.iIndex, 0, text_list.size(), -1, TxtMiru::TT_LINK));
							}
							tp.iIndex = -1;
						}
					} else if(CGrText::isMatchChar(lpttstr, _T("ID:"))){
						lpttstr += 3; //_T("ID:");
						text_list.push_back(TxtMiru::TextInfo(lpttstr, 0, -1, -1, -1, -1, TxtMiru::TT_ID));
					} else {
						m_paramList.push_back(tt_str);
					}
				}
				break;
			case TxtMiru::TT_ACCENT: break;
			case TxtMiru::TT_RUBY:
				{
					// RUBY : tt_str
					// TT_OTHER�ɂ��A���r���U���悤��
					TxtMiru::TextListPos lastOtherTLP;
					bool bOtherGetLastTLP = GetRFindTextType(lastOtherTLP, text_list, TxtMiru::TT_OTHER);
					if(bOtherGetLastTLP){
						/* TT_OTHER �� ���X�g�Ɋ܂܂�Ă����� */
						if(bGetLastTLP){
							/* TT_TEXT �� ���X�g�Ɋ܂܂�Ă����� */
							if(lastTLP < lastOtherTLP){
								/* TT_TEXT �� TT_OTHER ����ɓo�^����Ă������́ATT_OTHER�̈ʒu��D��(TT_OTHER �� TT_TEXT �͕ʂ̕�����ނƂ��Ĉ���) */
								bGetLastTLP = false; // �d������ RUBY�ݒ肳��Ȃ��悤�� �t���O���N���A
								if(ruby_pos_r >= 0){
									/* RUBY�J�n�ʒu���ݒ肳��Ă��鎞 */
									/*
									  �b�����s���t�̏ꍇ�́A �����Ɂu���v�̃��r���ӂ�
									  ������ TT_OTHER         �b : ruby_pos_r
									  ���� TT_TEXT            �� : ruby_pos_r+1
									  �� TT_RUBY_SEP          �� : lastOtherTLP.iIndex
									 */
									text_list[ruby_pos_r].textType = TxtMiru::TT_RUBY_SEP;
									text_list.push_back(TextInfoSpec(static_cast<std::tstring&&>(tt_str), textType, ruby_pos_r+1, 0, lastOtherTLP.iIndex, 0));
								} else {
									/* RUBY�J�n�ʒu���ݒ肳��Ă��Ȃ����́ATT_OTHER �̈ʒu���g�p���� */
									/*
									  �����s���t�̏ꍇ�́A ���Ɂu���v�̃��r���ӂ�
									  ����TT_OTHER            �� : lastOtherTLP.iIndex
									  ��TT_TEXT
									 */
									// ���r�J�n�ʒu�ɂ͏�� [TT_RUBY_SEP]������ // ��RUBY���Ή�
									TxtMiru::TextInfo ti_sep;
									ti_sep.textType = TxtMiru::TT_RUBY_SEP;
									auto text_it = text_list.begin() + lastOtherTLP.iIndex;
									text_list.insert(text_it, ti_sep); // �Z�p���[�^�̑}��
									// �r���ɑ}������̂ŁA���ۂɂ� ruby�ʒu�⑼�̃f�[�^�̎Q�ƈʒu�����炷�K�v������ ������
									incTextListIndexPos(lastOtherTLP.iIndex, text_list, lastOtherTLP.iIndex, 1);
									text_list.push_back(TextInfoSpec(static_cast<std::tstring&&>(tt_str), textType, lastOtherTLP.iIndex, 0, lastOtherTLP.iIndex, 0));
								}
							}
						} else {
							/* TT_TEXT �� ���X�g�Ɋ܂܂�Ă��Ȃ������� */
							if(ruby_pos_r >= 0){
								/* RUBY�J�n�ʒu���ݒ肳��Ă��鎞 */
								/*
								  �b���s���t�̏ꍇ�́A ���Ɂu���v�̃��r���ӂ�
								  ���� TT_OTHER
								  �� TT_RUBY_SEP
								 */
								text_list[ruby_pos_r].textType = TxtMiru::TT_RUBY_SEP;
								text_list.push_back(TextInfoSpec(static_cast<std::tstring&&>(tt_str), textType, ruby_pos_r+1, 0, lastOtherTLP.iIndex, 0));
							} else {
								/* RUBY�J�n�ʒu���ݒ肳��Ă��Ȃ����́ATT_OTHER �̈ʒu���g�p���� */
								/*
								  ���s���t�̏ꍇ�́A ���Ɂu���v�̃��r���ӂ�
								  ��TT_OTHER
								 */
								// ���r�J�n�ʒu�ɂ͏�� [TT_RUBY_SEP]������ // ��RUBY���Ή�
								TxtMiru::TextInfo ti_sep;
								ti_sep.textType = TxtMiru::TT_RUBY_SEP;
								auto text_it = text_list.begin() + lastOtherTLP.iIndex;
								text_list.insert(text_it, ti_sep);
								// �r���ɑ}������̂ŁA���ۂɂ� ruby�ʒu�⑼�̃f�[�^�̎Q�ƈʒu�����炷�K�v������ ������
								incTextListIndexPos(lastOtherTLP.iIndex, text_list, lastOtherTLP.iIndex, 1);
								text_list.push_back(TextInfoSpec(static_cast<std::tstring&&>(tt_str), textType, lastOtherTLP.iIndex, 0, lastOtherTLP.iIndex, 0));
							}
						}
					}
					//
					if(bGetLastTLP && bno_empty_tt_str){
						int last_iIndex = lastTLP.iIndex;
						int last_iPos   = lastTLP.iPos;
						if(ruby_pos_r >= 0){
							/* RUBY�J�n�ʒu���ݒ肳��Ă��鎞 */
							/*
							  �b���s���t�̏ꍇ�́A ���Ɂu���v�̃��r���ӂ�
							  ���� TT_TEXT            �b : ruby_pos_r
							  �� TT_RUBY_SEP          �� : last_iIndex, last_iPos
							 */
							text_list[ruby_pos_r].textType = TxtMiru::TT_RUBY_SEP;
							text_list.push_back(TextInfoSpec(static_cast<std::tstring&&>(tt_str), textType, ruby_pos_r+1, 0, last_iIndex, last_iPos));
						} else {
							/* RUBY�J�n�ʒu���ݒ肳��Ă��Ȃ����́ATT_TEXT �̈ʒu���g�p���� */
							/*
							  ���s���t�̏ꍇ�́A ���Ɂu���v�̃��r���ӂ�
							  �� TT_TEXT             �� : last_iIndex, last_iPos
							 */
							// ���r�J�n�ʒu�ɂ͏�� [TT_RUBY_SEP]������ // ��RUBY���Ή�
							TxtMiru::TextInfo ti_sep;
							ti_sep.textType = TxtMiru::TT_RUBY_SEP;
							auto text_it = text_list.begin() + last_iIndex;
							text_list.insert(text_it, ti_sep);
							// �r���ɑ}������̂ŁA���ۂɂ� ruby�ʒu�⑼�̃f�[�^�̎Q�ƈʒu�����炷�K�v������ ������
							incTextListIndexPos(last_iIndex, text_list, last_iIndex, 1);
							text_list.push_back(TextInfoSpec(static_cast<std::tstring&&>(tt_str), textType, last_iIndex, 0, last_iIndex, last_iPos));
						}
					}
				}
				ruby_pos_r = -1;
				break;
			case TxtMiru::TT_RUBY_L:
				if(!text_list.empty() && bno_empty_tt_str){
					auto lpNote = CGrText::Find(lpTTSrc, text_type_name[textType]);
					if(!lpNote || lpNote > lpTTSrcEnd/*�m�� �` �n�͈̔͊O */){
						break;
					}
					std::tstring target(lpTTSrc, CGrText::CharNext(lpNote)-1);
					std::tstring note(lpNote + text_type_name[textType].size(), lpTTSrcEnd);
					pushNote(text_list, target.c_str(), note.c_str(), textType);
				}
				break;
			case TxtMiru::TT_RUBY_L_START:
				ruby_pos_l = text_list.size();
				break;
			case TxtMiru::TT_RUBY_L_END:
				if(ruby_pos_l >= 0 && !text_list.empty() && bno_empty_tt_str){
					std::tstring note(lpTTSrc, lpTTSrcEnd);
					text_list.push_back(TextInfoSpec(note.c_str(), TxtMiru::TT_RUBY_L, ruby_pos_l, 0, text_list.size(), -1));
				}
				ruby_pos_l = -1;
				break;
			case TxtMiru::TT_NOTE:
				if(!text_list.empty() && bno_empty_tt_str){
					for(const auto check_tt : {TxtMiru::TT_NOTE, TxtMiru::TT_NOTE_L}){
						auto lpNote = CGrText::Find(lpTTSrc, text_type_name[check_tt]);
						if(!lpNote || lpNote > lpTTSrcEnd/*�m�� �` �n�͈̔͊O */){
							continue;
						}
						std::tstring target(lpTTSrc, CGrText::CharNext(lpNote)-1);
						std::tstring note(lpNote + text_type_name[check_tt].size(), lpTTSrcEnd);
						pushNote(text_list, target.c_str(), note.c_str(), check_tt);
					}
				}
				break;
			case TxtMiru::TT_NOTE_START:
				note_pos_r = text_list.size();
				break;
			case TxtMiru::TT_NOTE_START_L:
				note_pos_l = text_list.size();
				break;
			case TxtMiru::TT_NOTE_END:
				if(note_pos_r >= 0 && !text_list.empty() && bno_empty_tt_str){
					std::tstring note(lpTTSrc, lpTTSrcEnd);
					convertNoteStr(note);
					int note_pos_r_end = GetPrevIndexTexts(text_list, note_pos_r);
					text_list.push_back(TextInfoSpec(note.c_str(), TxtMiru::TT_NOTE, note_pos_r, 0, note_pos_r_end, CGrText::CharLen(text_list[note_pos_r_end].str.c_str())-1 ));
				}
				note_pos_r = -1;
				break;
			case TxtMiru::TT_NOTE_END_L:
				if(note_pos_l >= 0 && !text_list.empty() && bno_empty_tt_str){
					std::tstring note(lpTTSrc, lpTTSrcEnd);
					convertNoteStr(note);
					int note_pos_l_end = GetPrevIndexTexts(text_list, note_pos_l);
					text_list.push_back(TextInfoSpec(note.c_str(), TxtMiru::TT_NOTE_L, note_pos_l, 0, note_pos_l_end, CGrText::CharLen(text_list[note_pos_l_end].str.c_str())-1 ));
				}
				note_pos_l = -1;
				break;
			case TxtMiru::TT_TEXT_SIZE  :
			case TxtMiru::TT_TEXT_SIZE_L:
			case TxtMiru::TT_TEXT_SIZE_S:
				if(bno_empty_tt_str){
					auto lpNote = CGrText::Find(lpTTSrc, text_type_name[TxtMiru::TT_TEXT_SIZE]);
					if(!lpNote || lpNote > lpTTSrcEnd/*�m�� �` �n�͈̔͊O */){
						// �͈͐ݒ�
						int text_size = max(Str2Int(tt_str), 0);
						if(textType == TxtMiru::TT_TEXT_SIZE_S){
							text_size = -text_size;
							m_fontSizeS.push_back(m_iFontSize);
						} else {
							m_fontSizeL.push_back(m_iFontSize);
						}
						TxtMiru::TextInfo ti_size;
						ti_size.textType = TxtMiru::TT_TEXT_SIZE;
						ti_size.chrType  = text_size;
						text_list.push_back(ti_size);
						m_iFontSize      = text_size;
						break;
					}
					if(text_list.empty()){
						break;
					}
					std::tstring target(lpTTSrc, CGrText::CharNext(lpNote)-1);
					std::tstring note(lpNote+text_type_name[TxtMiru::TT_TEXT_SIZE].size());
					int text_size = max(Str2Int(note), 0);
					if(textType == TxtMiru::TT_TEXT_SIZE_S){
						text_size = -text_size;
					}
					auto len = text_list.size();
					pushNote(text_list, target.c_str(), _T(""), TxtMiru::TT_MaxNum/*Dummy*/);
					if(len < text_list.size()){
						auto ti_size = text_list[len];
						ti_size.textType = TxtMiru::TT_TEXT_SIZE;
						auto ti_str = text_list[ti_size.tpBegin.iIndex].str;
						TxtMiru::TextInfoList::iterator size_insert_it;
						auto lpti_str = ti_str.c_str();
						auto ti_tmp = text_list[ti_size.tpBegin.iIndex];
						int inc = 1;
						if(ti_size.tpBegin.iIndex == ti_size.tpEnd.iIndex){
							if(ti_size.tpBegin.iPos != 0){
								// str ����
								text_list[ti_size.tpBegin.iIndex].str = std::tstring(lpti_str, ti_size.tpBegin.iPos);
								auto size_center_it = text_list.begin() + ti_size.tpBegin.iIndex + 1;
								//
								ti_tmp.str = std::tstring(lpti_str + ti_size.tpBegin.iPos, ti_size.tpEnd.iPos - ti_size.tpBegin.iPos + 1);
								size_insert_it = text_list.insert(size_center_it, ti_tmp);
								++inc;
							} else {
								size_insert_it = text_list.begin() + ti_size.tpBegin.iIndex;
							}
							ti_size.chrType = text_size;
							size_insert_it = text_list.insert(size_insert_it, ti_size);
							// �e�L�X�g�̃T�C�Y�����ɖ߂�
							ti_size.chrType = m_iFontSize;
							size_insert_it += 2;
							size_insert_it = text_list.insert(size_insert_it, ti_size);
							if(/**/ti_size.tpBegin.iPos == ti_size.tpEnd.iPos
							   ||  ti_size.tpEnd.iPos == CGrText::CharLen(lpti_str)-1){
								;
							} else {
								(size_insert_it-1)->str = std::tstring(lpti_str + ti_size.tpBegin.iPos, ti_size.tpEnd.iPos - ti_size.tpBegin.iPos + 1);
								ti_tmp.str = std::tstring(lpti_str + ti_size.tpEnd.iPos + 1);
								size_insert_it += 1;
								text_list.insert(size_insert_it, ti_tmp);
								++inc;
							}
						} else {
							if(ti_size.tpBegin.iPos != 0){
								// str ����
								text_list[ti_size.tpBegin.iIndex].str = std::tstring(lpti_str, ti_size.tpBegin.iPos);
								auto size_center_it = text_list.begin() + ti_size.tpBegin.iIndex + 1;
								//
								ti_tmp.str = std::tstring(lpti_str + ti_size.tpBegin.iPos);
								size_insert_it = text_list.insert(size_center_it, ti_tmp);
								++inc;
							} else {
								size_insert_it = text_list.begin() + ti_size.tpBegin.iIndex;
							}
							ti_size.chrType = text_size;
							size_insert_it = text_list.insert(size_insert_it, ti_size);
							// �e�L�X�g�̃T�C�Y�����ɖ߂�
							ti_size.chrType = m_iFontSize;
							size_insert_it += ti_size.tpEnd.iIndex - ti_size.tpBegin.iIndex + 2;
							size_insert_it = text_list.insert(size_insert_it, ti_size);

							auto ti_end_str = (size_insert_it-1)->str;
							auto lpti_end_str = ti_end_str.c_str();
							if(ti_size.tpEnd.iPos == CGrText::CharLen(lpti_end_str)-1){
							} else {
								(size_insert_it-1)->str = std::tstring(lpti_end_str, ti_size.tpEnd.iPos + 1);
								ti_tmp.str = std::tstring(lpti_end_str + ti_size.tpEnd.iPos + 1);
								size_insert_it += 1;
								text_list.insert(size_insert_it, ti_tmp);
								++inc;
							}
						}
						incTextListIndexPos(ruby_pos_r, text_list, ti_size.tpBegin.iIndex, inc);
					}
				}
				break;
			case TxtMiru::TT_UNKOWN_ERROR        :
			case TxtMiru::TT_WHITE_DOT           :
			case TxtMiru::TT_ROUND_DOT           :
			case TxtMiru::TT_WHITE_ROUND_DOT     :
			case TxtMiru::TT_BLACK_TRIANGLE_DOT  :
			case TxtMiru::TT_WHITE_TRIANGLE_DOT  :
			case TxtMiru::TT_DOUBLE_ROUND_DOT    :
			case TxtMiru::TT_BULLS_EYE_DOT       :
			case TxtMiru::TT_SALTIRE_DOT         :
			case TxtMiru::TT_DOT                 :
			case TxtMiru::TT_LINE                :
			case TxtMiru::TT_WAVE_LINE           :
			case TxtMiru::TT_DEL_LINE            :
			case TxtMiru::TT_UNDER_LINE          :
			case TxtMiru::TT_SHORT_DASHED_LINE   :
			case TxtMiru::TT_DOT_LINE            :
			case TxtMiru::TT_DOUBLE_LINE         :
			case TxtMiru::TT_WHITE_DOT_L         :
			case TxtMiru::TT_ROUND_DOT_L         :
			case TxtMiru::TT_WHITE_ROUND_DOT_L   :
			case TxtMiru::TT_BLACK_TRIANGLE_DOT_L:
			case TxtMiru::TT_WHITE_TRIANGLE_DOT_L:
			case TxtMiru::TT_DOUBLE_ROUND_DOT_L  :
			case TxtMiru::TT_BULLS_EYE_DOT_L     :
			case TxtMiru::TT_SALTIRE_DOT_L       :
			case TxtMiru::TT_DOT_L               :
			case TxtMiru::TT_LINE_L              :
			case TxtMiru::TT_WAVE_LINE_L         :
			case TxtMiru::TT_SHORT_DASHED_LINE_L :
			case TxtMiru::TT_DOT_LINE_L          :
			case TxtMiru::TT_DOUBLE_LINE_L       :
			case TxtMiru::TT_BOLD                :
			case TxtMiru::TT_SUBTITLE1           :
			case TxtMiru::TT_SUBTITLE2           :
			case TxtMiru::TT_SUBTITLE3           :
			case TxtMiru::TT_MOVING_BORDER       :
			case TxtMiru::TT_HORZ_START          :
				if(tt_str.empty()){
					auto &&rtp = m_RangeTextPoint[textType];
					rtp.iLine = line_list.size();
					if(rtp.iCount == 0){
						if(bGetLastTLP){
							rtp.iIndex = lastTLP.iIndex      ;
							rtp.iPos   = lastTLP.iPos + 1    ;
						} else {
							rtp.iIndex = lastTLP.iIndex      ;
							rtp.iPos   = lastTLP.iPos        ;
						}
					}
					++rtp.iCount;
				} else {
					pushNote(text_list, tt_str.c_str(), text_type_name[textType], textType);
				}
				break;
			case TxtMiru::TT_CAPTION             :
				if(tt_str.empty()){
					text_list.push_back(TextInfoSpec(textType, 0));
				} else {
					TxtMiru::TextListPos tpBegin;
					if(!GetRFindText(tpBegin, text_list, tt_str.c_str())){
						break;
					}
					if(tpBegin.iPos > 0){
						text_list[tpBegin.iIndex].str = text_list[tpBegin.iIndex].str.substr(0, tpBegin.iPos);
					} else {
						text_list[tpBegin.iIndex].textType = TxtMiru::TT_COMMENT;
					}
					int tt_len = text_list.size();
					for(int caption_index = tpBegin.iIndex+1; caption_index<tt_len; ++caption_index){
						text_list[caption_index].textType = TxtMiru::TT_COMMENT;
					}
					auto *pti = GetPrevTextInfoPicture(line_list, text_list);
					if(pti){
						SetCaption(pti->str, tt_str.c_str());
					}
				}
				break;
			case TxtMiru::TT_LINE_BOX_START      :
				{
					LineBox linebox;
					linebox.top    = 0;
					linebox.bottom = 0;
					if(!m_Indent.empty()){
						const auto &indent = m_Indent.back();
						linebox.top = indent.iIndent1st;
					}
					if(!m_RIndent.empty()){
						const auto &indent = m_RIndent.back();
						linebox.bottom = indent.iIndent1st;
					}
					if(m_LineBox.size() < static_cast<unsigned int>(m_RangeTextPoint[textType].iCount)){
						m_LineBox.resize(m_RangeTextPoint[textType].iCount);
					}
					m_LineBox.push_back(linebox);
					++m_RangeTextPoint[textType].iCount;
					// 2.0.29.0
					text_list.push_back(
						TxtMiru::TextInfo(static_cast<const TCHAR*>(text_type_name[TxtMiru::TT_MOVING_BORDER]), m_RangeTextPoint[textType].iCount, linebox.top, linebox.bottom, 0, 0, textType)
						);
				}
				break;
			case TxtMiru::TT_RANGE_END           :
				{
					const auto *it_pl=it_plb;
					int skip = 2; // '�m��'�̂Q�������X�L�b�v
					if(CGrText::isMatchChar(tt_str.c_str(), _T("������"))){
						tt_str = CGrText::CharNextN(tt_str.c_str(), 3);
					} else if(CGrText::isMatchChar(tt_str.c_str(), _T("�����܂�"))){
						tt_str = CGrText::CharNextN(tt_str.c_str(), 4);
					}
					int len = CGrText::CharLen(tt_str.c_str()) + skip;
					for(int it_pl_idx=it_pl_len; it_pl_idx>0; --it_pl_idx, ++it_pl){
						const auto &mps = (*it_pl);
						if(mps.first.str_len == len && _tcscmp(CGrText::CharNextN(mps.first, skip), tt_str.c_str()) == 0){
							auto tt = mps.first.type();
							auto &&tp = m_RangeTextPoint[tt];
							auto tis = TextInfoSpec(static_cast<const TCHAR*>(text_type_name[tt]), tt, tp.iIndex, tp.iPos, text_list.size(), -1);
							if(tt == TxtMiru::TT_LINE_BOX_START && !m_LineBox.empty()){
								// �r�͂� �̏ꍇ
								const auto &linebox = m_LineBox.back();
								tis.tpBegin.iIndex = linebox.top   ; // �������FtpBegin.iIndex  �� TOP
								tis.tpBegin.iPos   = linebox.bottom; //           tpBegin.iPos    �� BOTTOM
								tis.chrType        = tp.iCount     ; //           chrType         �� �r�͂ݘA��
								tis.tpEnd.iPos     = 2             ; //           tpEnd.iPos      �� ���̃^�C�v ������ /* �J�n=0�A�r��=1, �I��=2*/
								tis.textType = TxtMiru::TT_LINE_BOX_END;
								m_LineBox.pop_back();
							} else if(tt == TxtMiru::TT_TEXT_SIZE_L || tt == TxtMiru::TT_TEXT_SIZE_S){
								if(TxtMiru::TT_TEXT_SIZE_S == tt && !m_fontSizeS.empty()){
									m_iFontSize = m_fontSizeS.back();
									m_fontSizeS.pop_back();
								} else if(TxtMiru::TT_TEXT_SIZE_L == tt && !m_fontSizeL.empty()){
									m_iFontSize = m_fontSizeL.back();
									m_fontSizeL.pop_back();
								} else {
									break;
								}
								tis.textType = TxtMiru::TT_TEXT_SIZE;
								tis.chrType  = m_iFontSize;
							} else if(tt == TxtMiru::TT_CAPTION){
								bool bBeginCaption = false;
								std::vector<std::tstring> string_list;
								for(auto &&ti : reverse_iterate(text_list)){
									if(ti.textType == TxtMiru::TT_CAPTION){
										ti.textType = TxtMiru::TT_COMMENT_BEGIN;
										bBeginCaption = true;
										break;
									} else if(TxtMiruType::isTextOrSkipOrRotateNum(ti.textType)){
										string_list.push_back(ti.str);
									}
								}
								if(!bBeginCaption){
									for(auto &&ll_item : reverse_iterate(line_list)){
										for(auto &&ti : reverse_iterate(ll_item.text_list)){
											if(ti.textType == TxtMiru::TT_CAPTION){
												ti.textType = TxtMiru::TT_COMMENT_BEGIN;
												bBeginCaption = true;
												if(!string_list.empty() && string_list.back() == _T("\n")){
													string_list.pop_back();
												}
												break;
											} else if(TxtMiruType::isTextOrSkipOrRotateNum(ti.textType)){
												string_list.push_back(ti.str);
											}
										}
										if(bBeginCaption){
											break;
										}
										string_list.push_back(_T("\n"));
									}
								}
								if(bBeginCaption){
									auto *pti = GetPrevTextInfoPicture(line_list, text_list);
									if(pti){
										std::tstring caption;
										for(const auto &item : reverse_iterate(string_list)){
											caption += item;
										}
										SetCaption(pti->str, caption.c_str());
									}
									text_list.push_back(TextInfoSpec(TxtMiru::TT_COMMENT_END, 0));
								}
								break;
							} else if(tt == TxtMiru::TT_HORZ_START){
								tis.textType = TxtMiru::TT_HORZ_END;
							}
							text_list.push_back(tis);
							--tp.iCount;
							if(tp.iCount < 0){
								tp.iCount = 0;
							}
							break;
						}
					}
				}
				break;
			case TxtMiru::TT_OTHER_NOTE        :
				if(!text_list.empty() && bno_empty_tt_str){
					auto lpNote = CGrText::Find(lpTTSrc, text_type_name[TxtMiru::TT_ERROR]);
					if(!lpNote || lpNote > lpTTSrcEnd/*�m�� �` �n�͈̔͊O */){
						break;
					}
					std::tstring target(lpTTSrc, CGrText::CharNext(lpNote)-1);
					std::tstring note(lpNote + text_type_name[TxtMiru::TT_ERROR].size(), lpTTSrcEnd);
					note.insert(0, _T("��{�ł́A"));
					pushNote(text_list, target.c_str(), note.c_str(), textType);
					break;
				}
				pushNote(text_list, tt_str.c_str(), text_type_name[textType], textType);
				break;
			case TxtMiru::TT_NO_READ:
				text_list.push_back(TxtMiru::TextInfo(static_cast<const TCHAR*>(text_type_name[textType]), 0xffff));
				text_list.push_back(TextInfoSpec(static_cast<std::tstring&&>(tt_str), textType, text_list.size()-1));
				break;
			case TxtMiru::TT_HORZ:
				if(bno_empty_tt_str){
					auto lpNote = CGrText::Find(lpTTSrc, text_type_name[TxtMiru::TT_TEXT_SIZE]);
					if(!lpNote || lpNote > lpTTSrcEnd/*�m�� �` �n�͈̔͊O */){
						break;
					}
					if(text_list.empty()){
						break;
					}
					std::tstring target(lpTTSrc, CGrText::CharNext(lpNote)-1);
					std::tstring note(lpNote+text_type_name[TxtMiru::TT_TEXT_SIZE].size());
					auto len = text_list.size();
					pushNote(text_list, target.c_str(), _T(""), TxtMiru::TT_MaxNum/*Dummy*/);
					if(len < text_list.size()){
						auto ti_size = text_list[len];
						ti_size.textType = TxtMiru::TT_HORZ_START;
						ti_size.chrType = 1;
						auto ti_str = text_list[ti_size.tpBegin.iIndex].str;
						TxtMiru::TextInfoList::iterator size_insert_it;
						auto lpti_str = ti_str.c_str();
						auto ti_tmp = text_list[ti_size.tpBegin.iIndex];
						int inc = 1;
						if(ti_size.tpBegin.iIndex == ti_size.tpEnd.iIndex){
							if(ti_size.tpBegin.iPos != 0){
								// str ����
								text_list[ti_size.tpBegin.iIndex].str = std::tstring(lpti_str, ti_size.tpBegin.iPos);
								auto size_center_it = text_list.begin() + ti_size.tpBegin.iIndex + 1;
								//
								ti_tmp.str = std::tstring(lpti_str + ti_size.tpBegin.iPos, ti_size.tpEnd.iPos - ti_size.tpBegin.iPos + 1);
								size_insert_it = text_list.insert(size_center_it, ti_tmp);
								++inc;
							} else {
								size_insert_it = text_list.begin() + ti_size.tpBegin.iIndex;
							}
							size_insert_it = text_list.insert(size_insert_it, ti_size);
							size_insert_it += 2;
							ti_size.textType = TxtMiru::TT_HORZ_END;
							size_insert_it = text_list.insert(size_insert_it, ti_size);
							if(/**/ti_size.tpBegin.iPos == ti_size.tpEnd.iPos
							   ||  ti_size.tpEnd.iPos == CGrText::CharLen(lpti_str)-1){
								;
							} else {
								(size_insert_it-1)->str = std::tstring(lpti_str + ti_size.tpBegin.iPos, ti_size.tpEnd.iPos - ti_size.tpBegin.iPos + 1);
								ti_tmp.str = std::tstring(lpti_str + ti_size.tpEnd.iPos + 1);
								size_insert_it += 1;
								text_list.insert(size_insert_it, ti_tmp);
								++inc;
							}
						} else {
							if(ti_size.tpBegin.iPos != 0){
								// str ����
								text_list[ti_size.tpBegin.iIndex].str = std::tstring(lpti_str, ti_size.tpBegin.iPos);
								auto size_center_it = text_list.begin() + ti_size.tpBegin.iIndex + 1;
								//
								ti_tmp.str = std::tstring(lpti_str + ti_size.tpBegin.iPos);
								size_insert_it = text_list.insert(size_center_it, ti_tmp);
								++inc;
							} else {
								size_insert_it = text_list.begin() + ti_size.tpBegin.iIndex;
							}
							size_insert_it = text_list.insert(size_insert_it, ti_size);
							size_insert_it += ti_size.tpEnd.iIndex - ti_size.tpBegin.iIndex + 2;
							ti_size.textType = TxtMiru::TT_HORZ_END;
							size_insert_it = text_list.insert(size_insert_it, ti_size);

							auto ti_end_str = (size_insert_it-1)->str;
							auto lpti_end_str = ti_end_str.c_str();
							if(ti_size.tpEnd.iPos == CGrText::CharLen(lpti_end_str)-1){
							} else {
								(size_insert_it-1)->str = std::tstring(lpti_end_str, ti_size.tpEnd.iPos + 1);
								ti_tmp.str = std::tstring(lpti_end_str + ti_size.tpEnd.iPos + 1);
								size_insert_it += 1;
								text_list.insert(size_insert_it, ti_tmp);
								++inc;
							}
						}
						incTextListIndexPos(ruby_pos_r, text_list, ti_size.tpBegin.iIndex, inc);
					}
				}
				break;
			case TxtMiru::TT_ROTATE_NUM:
			case TxtMiru::TT_SUP_NOTE:
			case TxtMiru::TT_SUB_NOTE:
				if(bno_empty_tt_str && bGetLastTLP){
					auto lpPreStr = text_list[lastTLP.iIndex].str.c_str();
					int nPreStrLen = CGrText::CharLen(lpPreStr);
					auto lpFindSrcTmp = CGrText::Find(lpTTSrc, _T("�u"));
					if(lpFindSrcTmp >= lpTTSrcEnd){ lpFindSrcTmp = nullptr; }
					if(!lpFindSrcTmp && tt_str.size() >= 2){
						// �m���Q�߂́u�Q�v�͏�t���������n-> n�����ڎw��L�� �ɂȂ��Ă��Ȃ�
						//  �c�����̕�����2�����ȏ�̎��AA1�Ƃ�0.1�̂悤�� charType�̈Ⴄ���̂����邩���`�F�b�N����
						// ��1�m���̒��O �ȊO�͖��Ή�
						// ��2 ���r�Ƃ� ����q�ɂ����Ή�
						// ��3 aa12�m���ua12�v�͏c�����n�����Ή� (�uaa�v�̕������K�v�Ȃ̂� a12�m���ua12�v�͏c�����n��OK)
						// ��4 �S�p�����̏c�����w����t��
						TxtMiru::TextInfoList tt_text_list;
						TxtMiru::TextInfo *ptt_ti = nullptr;
						for(auto lpTTStr = tt_str.c_str(); *lpTTStr;){
							auto lpNextTTStr = CGrText::CharNext(lpTTStr);
							auto chrType = CGrText::GetStringTypeEx(lpTTStr, lpNextTTStr-lpTTStr);
							if(!ptt_ti || ptt_ti->chrType != chrType){
								TxtMiru::TextInfo tt_ti;
								tt_ti.chrType = chrType;
								tt_ti.str     = std::tstring(lpTTStr, lpNextTTStr-lpTTStr);
								tt_text_list.push_back(tt_ti);
								ptt_ti = &tt_text_list[tt_text_list.size()-1];
							} else {
								ptt_ti->str += std::tstring(lpTTStr, lpNextTTStr-lpTTStr);
							}
							lpTTStr = lpNextTTStr;
						}
						int tt_text_list_size = tt_text_list.size();
						int text_list_size    = text_list   .size();
						if(tt_text_list_size > 1 && text_list_size >= tt_text_list_size){
							ptt_ti = &tt_text_list[tt_text_list_size-1];
							int last_index = text_list_size - 1;
							auto *pti = &(text_list[last_index]);
							bool bNoText = false;
							for(int tt_index=tt_text_list_size-1; tt_index>=1; --tt_index, --ptt_ti, --pti){
								if(pti->str != ptt_ti->str){
									bNoText = true;
									break;
								}
							}
							if(bNoText){
								break;
							}
							if(pti->str == ptt_ti->str){
								pti->str = tt_str;
								pti->textType = textType;
								text_list.resize(text_list_size-tt_text_list_size+1);
								lastTLP.iIndex = text_list.size()-1;
								lastTLP.iPos   = 0;
							}
							break;
						}
					}
					if(nPreStrLen == 1){
						if(lpFindSrcTmp){ // �{���A�u �͌�����Ȃ��͂��B�������� n�����ڂ̎w�肪����
							if(max(Str2Int(tt_str)-1, 0) != 0){ break; } // ��O�̕����́A�ꕶ���Ȃ̂ň�ڈȊO�͂�������
							tt_str = std::tstring(lpFindSrcTmp, lpTTSrcEnd);
						}
						if(CGrText::isMatchChar(lpPreStr, tt_str.c_str())){ // ��O�̕����Ɠ������ǂ������`�F�b�N
							text_list[lastTLP.iIndex] = TextInfoSpec(static_cast<std::tstring&&>(tt_str), textType, lastTLP);
							text_list[lastTLP.iIndex].chrType = CGrText::GetStringTypeEx(lpPreStr, CGrText::CharNext(lpPreStr)-lpPreStr);
						}
					} else if(nPreStrLen >= 2){
						LPCTSTR lpPreTarget = nullptr;
						// �m���Q�߂́u�Q�v�͏�t���������n-> n�����ڎw��L��
						// �m���u2�v�͏�t���������n         -> n�����ڂ̎w�薳
						if(!lpFindSrcTmp){ // n�����ڂ̎w�薳��
							lpPreTarget = CGrText::RFind(lpPreStr, CGrText::CharNextN(lpPreStr,nPreStrLen-1), tt_str.c_str());
							if(!lpPreTarget){
								// ������������������ꍇ�AiIndex=0�܂Ń^�[�Q�b�g������(tt_str)����������
								// ���̏ꍇ�A�������镶����� lastTLP�ł͖��� tmp_iIndex�ɂȂ� text_list��insert����K�v������
								break; // �w�肵����������(����)
							}
							// �w�肵�������L��
						} else { // n�����ڎw��L��
							int n = max(Str2Int(tt_str)-1, 0);
							if(n < 0){ break; } // n�̒l���s��
							auto lpFindSrcEndTmp = CGrText::Find(lpTTSrc, _T("�v"));
							if(!lpFindSrcEndTmp){ break; } // �v��������΁A����
							tt_str = std::tstring(CGrText::CharNext(lpFindSrcTmp), lpFindSrcEndTmp);
							auto lpPreTargetNext = lpPreStr;
							for(;n >= 0; --n){
								if(!(*lpPreTargetNext)){ lpPreTarget = nullptr; break; }
								lpPreTarget = CGrText::Find(lpPreTargetNext, tt_str.c_str());
								if(!lpPreTarget){ break; }
								if(!(*lpPreTarget)){ lpPreTarget = nullptr; break; }
								lpPreTargetNext = CGrText::CharNext(lpPreTarget);
							}
							if(!lpPreTarget){ break; } // �w�肵����������(����)
						}
						// 1112233 : 22 ��texttype �ƂȂ��Ă���ꍇ
						// ��U�A111 22 33 �ɕ����� 22 ���������ύX
						int tt_str_len = CGrText::CharLen(tt_str.c_str());
						std::tstring preStr1(lpPreStr, lpPreTarget);
						std::tstring preStr2(lpPreTarget, CGrText::CharNextN(lpPreTarget, tt_str_len));
						std::tstring preStr3 = CGrText::CharNextN(lpPreTarget, tt_str_len);
						auto tmpti = text_list[lastTLP.iIndex];
						if(preStr1.empty()){
							auto chrType = text_list[lastTLP.iIndex].chrType;
							text_list[lastTLP.iIndex] = TextInfoSpec(static_cast<std::tstring&&>(preStr2), textType, lastTLP);
							text_list[lastTLP.iIndex].chrType = chrType;
						} else {
							text_list[lastTLP.iIndex].str = preStr1;
							++lastTLP.iIndex;
							text_list.push_back(TextInfoSpec(static_cast<std::tstring&&>(preStr2), textType, lastTLP));
						}
						if(preStr3.empty()){ break; }
						tmpti.str = preStr3;
						text_list.push_back(tmpti);
					}
				}
				break;
			case TxtMiru::TT_SMALL_NOTE_R:
			case TxtMiru::TT_SMALL_NOTE_L:
				if(bno_empty_tt_str && bGetLastTLP){
					TxtMiru::TextListPos tpBegin;
					if(GetRFindText(tpBegin, text_list, tt_str.c_str())){
						// 2�ɕ�����
						if(tpBegin.iPos != 0){
							auto first_str = text_list[tpBegin.iIndex].str;
							auto lpFistStr = first_str.c_str();
							auto tmp_ti = text_list[tpBegin.iIndex];
							auto tmp_it = text_list.begin() + tpBegin.iIndex;
							text_list.insert(tmp_it, tmp_ti);
							text_list[tpBegin.iIndex+0].str = std::tstring(lpFistStr, CGrText::CharNextN(lpFistStr, tpBegin.iPos));
							text_list[tpBegin.iIndex+1].str = CGrText::CharNextN(lpFistStr, tpBegin.iPos);
							//
							++tpBegin.iIndex;
							tpBegin.iPos = 0;
							// �r���ɑ}������̂ŁA���ۂɂ� ruby�ʒu�⑼�̃f�[�^�̎Q�ƈʒu�����炷�K�v������ ������
							incTextListIndexPos(ruby_pos_r, text_list, tpBegin.iIndex, 1);
						}
						int offset = GetTextListOffset(tpBegin, text_list) + CGrText::CharLen(tt_str.c_str()) -1;
						auto tpEnd = GetTextListPos(offset, text_list);
						for(int tmp_iIndex=tpBegin.iIndex; tmp_iIndex<=tpEnd.iIndex; ++tmp_iIndex){
							text_list[tmp_iIndex].textType = textType;
						}
					}
				}
				break;
			case TxtMiru::TT_SMALL_NOTE:
				if(bno_empty_tt_str){
					TxtMiru::TextInfo ti;
					if(bGetLastTLP){
						ti = TextInfoSpec(static_cast<std::tstring&&>(tt_str), textType, lastTLP);
					} else {
						ti = TextInfoSpec(static_cast<std::tstring&&>(tt_str), textType, 0);
					}
					auto lpSNStr = tt_str.c_str();
					ti.chrType = CGrText::GetStringTypeEx(lpSNStr, CGrText::CharNext(lpSNStr)-lpSNStr);
					text_list.push_back(ti);
				}
				break;
			case TxtMiru::TT_INDENT      :
			case TxtMiru::TT_INDENT_START:
				{
					auto lpTurn = CGrText::Find(lpTTSrc, text_type_name[TxtMiru::TT_INDENT_START]);
					if(lpTurn && lpTurn <= lpTTSrcEnd/*�m�� �` �n�͈̔� */){ // �܂�Ԃ���
						std::tstring first_indent(lpTTSrc, CGrText::CharNext(lpTurn)-1);
						std::tstring second_indent(lpTurn + text_type_name[TxtMiru::TT_INDENT_START].size(), lpTTSrcEnd);
						indentStart(buffer, text_list, textType, Str2Int(first_indent), Str2Int(second_indent));
					} else {
						indentStart(buffer, text_list, textType, Str2Int(tt_str));
					}
				}
				break;
			case TxtMiru::TT_INDENT2      : indentStart (buffer, text_list, TxtMiru::TT_INDENT      , 0, Str2Int(tt_str)); break;
			case TxtMiru::TT_INDENT_START2: indentStart (buffer, text_list, TxtMiru::TT_INDENT_START, 0, Str2Int(tt_str)); break;
			case TxtMiru::TT_INDENT3      : indentStart (buffer, text_list, TxtMiru::TT_INDENT      , Str2Int(tt_str)); break;
			case TxtMiru::TT_RINDENT      :
			case TxtMiru::TT_RINDENT_START: rindentStart(buffer, text_list, textType                , Str2Int(tt_str)); break;
			case TxtMiru::TT_INDENT_END:
				indentEnd(buffer, text_list, textType); break;
			case TxtMiru::TT_RINDENT_END:
				rindentEnd(buffer, text_list, textType); break;
			case TxtMiru::TT_LIMIT_CHAR_START:
				m_LimitChar.push_back(Str2Int(tt_str));
				text_list.push_back(TextInfoSpec(TxtMiru::TT_LIMIT_CHAR_START, m_LimitChar.back(), 0));
				break;
			case TxtMiru::TT_LIMIT_CHAR_END:
				if(!m_LimitChar.empty()){
					m_LimitChar.pop_back();
				}
				if(m_LimitChar.empty()){
					text_list.push_back(TextInfoSpec(TxtMiru::TT_LIMIT_CHAR_START, 0, 0));
				} else {
					text_list.push_back(TextInfoSpec(TxtMiru::TT_LIMIT_CHAR_START, m_LimitChar.back(), 0));
				}
				break;
			case TxtMiru::TT_BOLD_START:
			case TxtMiru::TT_BOLD_END:
				text_list.push_back(TextInfoSpec(textType, 0));
				break;
			case TxtMiru::TT_NEXT_LINE:
				// �s�̍Œ��� ���s�����������ꍇ�A���X�g�𕪊�
				bSplitLine = true;
				break;
			case TxtMiru::TT_NEXT_LAYOUT:
			case TxtMiru::TT_NEXT_PAGE:
			case TxtMiru::TT_NEXT_PAPER:
			case TxtMiru::TT_NEXT_PAPER_FIRST:
				if(!text_list.empty()){
					// �s�̍Œ��� ���i�E���ŁE���������������ꍇ�A���X�g�𕪊�
					buffer.MoveBackLineInfo(TxtMiru::LineInfo{ -1,0,0,0,0,0,text_list });
					//
					text_list.clear();
					ruby_pos_r = -1;
					ruby_pos_l = -1;
				}
				text_list.push_back(TextInfoSpec(textType, 0));
				bSplitLine = true;
				break;
			case TxtMiru::TT_OTHER:
				if(CGrText::isMatchChar(lpSrc, text_type_name[TxtMiru::TT_NO_READ])){
					auto lpNote = CGrText::Find(lpTTSrc, text_type_name[TxtMiru::TT_OTHER]);
					LPCTSTR lpConvertStr;
					if(lpNote && lpNote <= lpTTSrcEnd){
						// ���m���n�ߓ�d�R���ʁA1-1-52�n
						std::tstring target(lpTTSrc, CGrText::CharNext(lpNote)-1);
						lpConvertStr = ConvertNoteChar(target.c_str());
					} else {
						// ���m���n�ߓ�d�R���ʁn
						lpConvertStr = ConvertNoteChar(tt_str.c_str());
					}
					if(lpConvertStr){
						int icnt = CGrText::CharLen(lpConvertStr);
						text_list.push_back(TextInfoLPTSTR(lpConvertStr, CGrText::CharNextN(lpConvertStr, icnt),
														   CGrText::GetStringTypeEx(lpConvertStr, icnt)));
					} else {
						// ���m��xxx�n��xxx���ϊ��ΏۂłȂ���΁A���͕\���Ώ�
						lpFindSrc = nullptr;
					}
					break;
				}
				if(CGrText::CharLen(tt_str.c_str()) == 1){ // �Ԃ�_ ? �P�_
					if(bGetLastTLP){
						text_list.push_back(TextInfoSpec(static_cast<std::tstring&&>(tt_str), TxtMiru::TT_GUID_MARK, lastTLP));
					} else {
						text_list.push_back(TextInfoSpec(static_cast<std::tstring&&>(tt_str), TxtMiru::TT_GUID_MARK, 0));
					}
				} else {
					auto lpNote = CGrText::Find(lpTTSrc, text_type_name[TxtMiru::TT_OTHER]);
					if(!lpNote || lpNote > lpTTSrcEnd/*�m�� �` �n�͈̔͊O */){
						auto lpConvertStr = ConvertNoteChar(tt_str.c_str());
						if(lpConvertStr){
							// '�s' : �n�ߓ�d�R����, '�t' : �I����d�R���� �Ȃ� �󕶌ɂŎg�p�ł��Ȃ�������ϊ�
							int icnt = CGrText::CharLen(lpConvertStr);
							text_list.push_back(TextInfoLPTSTR(lpConvertStr, CGrText::CharNextN(lpConvertStr, icnt),
															   CGrText::GetStringTypeEx(lpConvertStr, icnt)));
						}
						break;
					}
					if(bGetLastTLP && bno_empty_tt_str){
						std::tstring ttsrc(lpTTSrc, lpTTSrcEnd);
						std::tstring target(lpTTSrc, CGrText::CharNext(lpNote)-1);
						auto lpFindSrcTmp    = CGrText::Find(lpTTSrc, _T("�u"));
						auto lpFindSrcEndTmp = CGrText::Find(lpTTSrc, _T("�v"));
						if(lpFindSrcTmp && lpFindSrcEndTmp && lpFindSrcEndTmp > lpFindSrcTmp){
							target = std::tstring(CGrText::CharNext(lpFindSrcTmp), lpFindSrcEndTmp);
						}
						auto lpLastStr = text_list[lastTLP.iIndex].str.c_str();
						if(CGrText::CharLen(lpLastStr) == 1){
							// ���m��xxxx�n�ɂȂ��Ă���̂ŁA�O�́u���v�� ConvertNoteChar �̌��ʂŒu��
							text_list[lastTLP.iIndex].tpBegin = lastTLP;
							auto lpConvertStr = ConvertNoteChar(ttsrc.c_str());
							if(lpConvertStr){
								text_list[lastTLP.iIndex].textType = TxtMiru::TT_TEXT;
								text_list[lastTLP.iIndex].str = lpConvertStr;
							} else {
								text_list[lastTLP.iIndex].textType = TxtMiru::TT_OTHER;
								text_list[lastTLP.iIndex].str = target;
							}
						} else {
							// �v����
							auto lpConvertStr = ConvertNoteChar(ttsrc.c_str());
							if(lpConvertStr){
								text_list[lastTLP.iIndex].str = std::tstring(lpLastStr, CGrText::CharNextN(lpLastStr, CGrText::CharLen(lpLastStr)) - 1);
								text_list.push_back(TxtMiru::TextInfo(lpConvertStr, CGrText::GetStringTypeEx(lpConvertStr, CGrText::CharLen(lpConvertStr))));
							} else {
								auto ct = CGrText::GetStringTypeEx(CGrText::CharNextN(lpLastStr, CGrText::CharLen(lpLastStr) - 1), 1);
								text_list[lastTLP.iIndex].str = std::tstring(lpLastStr, CGrText::CharNextN(lpLastStr, CGrText::CharLen(lpLastStr)) - 1);
								text_list.push_back(
									TxtMiru::TextInfo(
										static_cast<std::tstring&&>(target),
										ct,
										TxtMiru::TextListPos{lastTLP.iIndex + 1},
										TxtMiru::TextListPos(),
										TxtMiru::TT_OTHER
											)
									);
							}
						}
					}
				}
				break;
			case TxtMiru::TT_PICTURE_LAYOUT:
				if(bno_empty_tt_str){
					// �m�����V��̐}�ifig42154_01.png�A��321�~�c123�j����n
					// �m���u��ꎵ���@�����S���d�m���������䗝���i���ʁv�̃L���v�V�����t���̐}�ifig4990_07.png�A��321�~�c123�j����n
					// �m���}�ifig42154_01.png�j����n
					// �m���}�P�ifig42154_01.png�j����n
					// �m���}10�ifig42154_01.png�j����n
					auto lpNoteEnd = CGrText::Find(lpTTSrc, _T("�i"));
					if(!lpNoteEnd){
						lpNoteEnd = CGrText::Find(lpTTSrc, _T(" "));
					}
					if(!lpNoteEnd || lpNoteEnd > lpTTSrcEnd/*�m�� �` �n�͈̔͊O */){
						break;
					}
					std::tstring note_str(lpTTSrc, lpNoteEnd);
					std::tstring other_str(CGrText::CharNext(lpNoteEnd), lpTTSrcEnd);
					std::tstring filename_str;
					std::tstring size_str;
					std::tstring pos_str;
					auto lpNextPCStr = other_str.c_str();
					while(lpNextPCStr && *lpNextPCStr){
						auto lpPCStr = lpNextPCStr;
						lpNextPCStr = CGrText::Find(lpNextPCStr, _T("�A"));
						std::tstring buf;
						if(!lpNextPCStr){
							buf = lpPCStr;
						} else {
							buf = std::tstring(lpPCStr, lpNextPCStr);
							lpNextPCStr = CGrText::CharNext(lpNextPCStr);
						}
						/* */if(filename_str.empty()                ){ filename_str = buf; } // �t�@�C����
						else if(CGrText::Find(buf.c_str(), _T("�~"))){ size_str     = buf; } // �T�C�Y
						else {
							// �e�L�X�g���C�A�E�g�̐ݒ�(���J���Ȃ�)
							for(const auto &p : l_PicturePositionType){
								if(CGrText::isMatchChar(buf.c_str(), p.name)){
									pos_str = buf;
									textType = p.textType;
									break;
								}
							}
						}
					}
					tt_str  = filename_str; tt_str.append(1, '\0');
					tt_str += note_str    ; tt_str.append(1, '\0');
					tt_str += size_str    ; tt_str.append(1, '\0');
					tt_str += pos_str     ; tt_str.append(1, '\0');
					tt_str.append(1, '\0');
					{
						std::tstring caption;
						auto lpFindCaptionTmp = CGrText::GetShortMatchStr(caption, lpTTSrc, _T("�u"), _T("�v�̃L���v�V����"));
						if(lpFindCaptionTmp){
							tt_str += caption;
						}
					}
					tt_str.append(1, '\0');
					text_list.push_back(TextInfoSpec(static_cast<std::tstring&&>(tt_str), textType, lastTLP));
				}
				break;
			case TxtMiru::TT_ENDOFCONTENTS:
				text_list.push_back(TextInfoSpec(textType, 0));
				m_bEndofContents = true;
				break;
			case TxtMiru::TT_CENTER:
				text_list.push_back(TextInfoSpec(textType, 0));
				break;
			default:
				if(bno_empty_tt_str){
					text_list.push_back(TextInfoSpec(static_cast<std::tstring&&>(tt_str), textType, lastTLP));
				}
				break;
			}
			if(lpFindSrc){
				lpNextSrc = CGrText::CharNextN(lpFindSrc, pmps->second.len());
			}
		} /* End : if(lpFindSrc && pmps) */
		if(lpFindSrc){
			continue;
		}
		// �Œ蕶��
		if(ad.IsTTListFistChar(lpSrc)){
			const auto *it_ps=it_slb; // MatchStrData:TT_RUBY_SEP,TT_KU1_CHAR...
			for(int it_ps_idx=it_sl_len; it_ps_idx>0; --it_ps_idx, ++it_ps){
				const auto &ms = (*it_ps);
				auto textType = ms.type();
				if(textType == TxtMiru::TT_OVERLAP_CHAR && !m_bUseOverlapChar){
					continue;
				}
				if(CGrText::isMatchChar(lpSrc, ms)){
					if(lpTextStart){
						text_list.push_back(TextInfoLPTSTR(lpTextStart, lpTextEnd, chrType));
						lpTextStart = nullptr;
					}
					auto tmpCharType = CGrText::GetStringTypeEx(lpSrc, lpNextSrc-lpSrc);
					if(textType == TxtMiru::TT_RUBY_SEP){
						text_list.push_back(TxtMiru::TextInfo(static_cast<const TCHAR*>(ms), tmpCharType));
						ruby_pos_r = text_list.size()-1;
					} else {
						TxtMiru::TextListPos lastTLP;
						GetRFindTextType(lastTLP, text_list, TxtMiru::TT_TEXT);
						text_list.push_back(TxtMiru::TextInfo(static_cast<const TCHAR*>(ms), tmpCharType, textType, lastTLP));
					}
					lpFindSrc = lpNextSrc;
					lpNextSrc = CGrText::CharNextN(lpSrc, ms.len());
					break;
				}
			}
			if(lpFindSrc){
				continue;
			}
		}
		// �{��
		// �u���v�̏ꍇ�A�O�̕��������� ���� �Љ��� �Ȃ� ���̕�����ނɕύX
		if(lpTextStart){
			auto tmpCharType = CGrText::GetStringTypeEx(lpSrc, lpNextSrc-lpSrc);
			if((chrType == C3_KATAKANA || chrType == 0x8100) && tmpCharType == (C3_KATAKANA|0x8100|0xf000)){
				tmpCharType = chrType;
			}
			if(chrType != tmpCharType){
				text_list.push_back(TextInfoLPTSTR(lpTextStart, lpTextEnd, chrType));
				lpTextStart = nullptr;
			}
		}
		//
		lpTextEnd = lpSrc;
		if(!lpTextStart){
			lpTextStart = lpTextEnd;
			chrType = CGrText::GetStringTypeEx(lpTextStart, CGrText::CharNext(lpTextStart)-lpTextStart);
		}
	} // while(lpNextSrc < lpEnd && *lpNextSrc)
	// �{��
	if(lpTextStart && *lpTextStart != _T('\r') && *lpTextStart != _T('\n')){
		text_list.push_back(TextInfoLPTSTR(lpTextStart, lpTextEnd, chrType));
	}
	for(int i=0; i<sizeof(m_RangeTextPoint)/sizeof(m_RangeTextPoint[0]); ++i){
		auto tt = static_cast<TxtMiru::TextType>(i);
		if(tt != TxtMiru::TT_LINE_BOX_START && m_RangeTextPoint[tt].iCount != 0){
			const auto &tp = m_RangeTextPoint[tt];
			text_list.push_back(TextInfoSpec(static_cast<const TCHAR*>(text_type_name[tt]), tt, tp.iIndex, tp.iPos, text_list.size(), -1));
			m_RangeTextPoint[i].iIndex = 0;
			m_RangeTextPoint[i].iPos   = 0;
		}
	}
	if(!m_LineBox.empty()){
		const auto *it = &m_LineBox[0];
		auto len = m_LineBox.size();
		auto tis = TextInfoSpec(static_cast<const TCHAR*>(text_type_name[TxtMiru::TT_MOVING_BORDER]), TxtMiru::TT_LINE_BOX_START, 0, 0, text_list.size(), -1);
		tis.chrType = 1;
		tis.tpEnd.iPos = 1;/* �J�n=0�A�r��=1, �I��=2 */
		for(; len>0; --len, ++it, ++tis.chrType){
			const auto &linebox = *it;
			tis.tpBegin.iIndex = linebox.top   ;
			tis.tpBegin.iPos   = linebox.bottom;
			text_list.push_back(tis);
		}
	}
	return true;
}

void CGrAozoraTxtParser::rindentEnd(CGrTxtBuffer &buffer, TxtMiru::TextInfoList &text_list, TxtMiru::TextType textType)
{
	if(m_bRIndentReset && !m_RIndent.empty()){
		m_RIndent.pop_back();
	}
	m_bRIndentReset = false;
	if(!m_RIndent.empty()){
		m_RIndent.pop_back();
	}
	text_list.push_back(TextInfoSpec(textType, 0));
	if(!m_RIndent.empty()){
		const auto &indent = m_RIndent.back();
		text_list.push_back(TextInfoSpec(TxtMiru::TT_RINDENT_START, indent.iIndent1st));
	}
	// �m�������Ŏ��グ�I���n�ȗ����̑Ή� �o�O�C��
	if(textType == TxtMiru::TT_RINDENT_END && !m_RIndent.empty()){
		rindentEnd(buffer, text_list, textType);
	}
}
void CGrAozoraTxtParser::indentEnd(CGrTxtBuffer &buffer, TxtMiru::TextInfoList &text_list, TxtMiru::TextType textType)
{
	if(m_bIndentReset && !m_Indent.empty()){
		m_Indent.pop_back();
	}
	m_bIndentReset = false;
	if(!m_Indent.empty()){
		m_Indent.pop_back();
	}
	text_list.push_back(TextInfoSpec(textType, 0));
	if(!m_Indent.empty()){
		const auto &indent = m_Indent.back();
		text_list.push_back(TextInfoSpec(TxtMiru::TT_INDENT_START , indent.iIndent1st, indent.iIndent2nd));
	}
	// �m�������Ŏ������I���n�ȗ����̑Ή� �o�O�C��
	if(textType == TxtMiru::TT_INDENT_END && !m_Indent.empty()){
		indentEnd(buffer, text_list, textType);
	}
}

void CGrAozoraTxtParser::rindentStart(CGrTxtBuffer &buffer, TxtMiru::TextInfoList &text_list, TxtMiru::TextType textType, int iIndent)
{
	// �m�������Ŏ��グ�I���n�ȗ����̑Ή� �o�O�C��
	if(m_bRIndentReset && !m_RIndent.empty()){
		m_RIndent.pop_back();
	}
	m_bRIndentReset = false;
	if(!m_RIndent.empty()){
		buffer.MoveBackLineInfo(
			TxtMiru::LineInfo { -1,0,0,0,0,0, TxtMiru::TextInfoList{TextInfoSpec(TxtMiru::TT_RINDENT_END, 0)} }
			);
	}
	Indent indent = {iIndent, iIndent};
	if(textType == TxtMiru::TT_RINDENT){
		m_bRIndentReset = true;
	}
	text_list.push_back(TextInfoSpec(textType, indent.iIndent1st));
	m_RIndent.push_back(indent);
}

void CGrAozoraTxtParser::indentStart(CGrTxtBuffer &buffer, TxtMiru::TextInfoList &text_list, TxtMiru::TextType textType, int iIndent)
{
	indentStart(buffer, text_list, textType, iIndent, iIndent);
}
void CGrAozoraTxtParser::indentStart(CGrTxtBuffer &buffer, TxtMiru::TextInfoList &text_list, TxtMiru::TextType textType, int iIndent1st, int iIndent2nd)
{
	// �m�������Ŏ������I���n�ȗ����̑Ή� �o�O�C��
	if(m_bIndentReset && !m_Indent.empty()){
		m_Indent.pop_back();
	}
	m_bIndentReset = false;
	if(!m_Indent.empty()){
		buffer.MoveBackLineInfo(
			TxtMiru::LineInfo { -1,0,0,0,0,0, TxtMiru::TextInfoList{TextInfoSpec(TxtMiru::TT_INDENT_END, 0)} }
			);
	}
	Indent indent = {iIndent1st, iIndent2nd};
	if(textType == TxtMiru::TT_INDENT){
		m_bIndentReset = true;
	}
	text_list.push_back(TextInfoSpec(textType, indent.iIndent1st, indent.iIndent2nd));
	m_Indent.push_back(indent);
}

bool CGrAozoraTxtParser::ReadBuffer(LPCTSTR lpBuffer, CGrTxtBuffer &buffer)
{
	return ReadBuffer(lpBuffer, buffer, _T(""));
}
#include "LoadDllFunc.h"
bool CGrAozoraTxtParser::ReadDataBuffer(LPBYTE lpData, DWORD fsize, CGrTxtBuffer &buffer, LPCTSTR lpFileName)
{
	bool bret = true;
	if(fsize > 0){
		HANDLE utfhHeap = INVALID_HANDLE_VALUE;
		WCHAR *lpUtfData = nullptr;
		do {
			DetectEncodingInfo detectEnc = {};
			auto *beginData = lpData;
			if(fsize >= 2){
				// BOM �Ń`�F�b�N
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
			if(detectEnc.nCodePage == 0){
				const auto &param = CGrTxtMiru::theApp().Param();
				std::tstring dll_filename;
				param.GetText(CGrTxtParam::GuessEncodingDLL, dll_filename);
				if(!dll_filename.empty()){
					CGrLoadDllFunc func(dll_filename.c_str());
					UINT(cdecl *DetectInputCodepage)(const char *buf, int buflen);
					SetProcPtr(DetectInputCodepage, func.GetProcAddress("DetectInputCodepage", NULL));
					if(DetectInputCodepage){
						detectEnc.nCodePage = DetectInputCodepage(reinterpret_cast<char*>(lpData), fsize);
					}
				}
			}
			IMultiLanguage2 *pIML2 = nullptr;
			if (CoCreateInstance(CLSID_CMultiLanguage, nullptr, CLSCTX_INPROC_SERVER, IID_IMultiLanguage2, reinterpret_cast<void**>(&pIML2)) == S_OK) {
				;
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
					if(detectEnc.nCodePage == 1252/* �ϊ��G���[? ����? */){
						detectEnc.nCodePage = 932/* sjis */;
					}
				}
				if(detectEnc.nCodePage == 932/* sjis */ && CGrText::isUTF16(beginData, fsize)){
					detectEnc.nCodePage = 1200/* UTF-16LE */;
				}
			}
			if(detectEnc.nCodePage != 1200/* UTF-16LE */){
				DWORD mode = 0;
				auto wlen = static_cast<UINT>(fsize);

				utfhHeap = ::HeapCreate(0, 0, 0);
				if (utfhHeap) {
					lpUtfData = static_cast<WCHAR*>(::HeapAlloc(utfhHeap, HEAP_ZERO_MEMORY, sizeof(WCHAR) * (wlen + 1)));
					if (lpUtfData) {
						auto file_size = static_cast<UINT>(fsize);
						if (S_OK == pIML2->ConvertStringToUnicodeEx(&mode, detectEnc.nCodePage, reinterpret_cast<CHAR*>(beginData), &file_size, lpUtfData, &wlen, 0, nullptr)) {
							lpUtfData[wlen] = '\0';
							lpData = reinterpret_cast<LPBYTE>(lpUtfData);
						}
					}
				}
			}
			pIML2->Release();
			m_title = CGrShell::GetFileName(lpFileName);
			bool bUsePreParser = true;
			{
				const auto &param = CGrTxtMiru::theApp().Param();
				int usePreParser[3] = {}; /* 0:Pre-Parser���g�p����, 1:Pre-Parser�̃G���[��\�����Ȃ�, 2:(n)MB�ȏ�̃t�@�C����Pre-Parser���g�p���Ȃ� */
				param.GetPoints(CGrTxtParam::UsePreParser, usePreParser, sizeof(usePreParser)/sizeof(int));
				if(usePreParser[2] > 0 && fsize > static_cast<UINT>(usePreParser[2]) * 1024/*KB*/ * 1024/*MB*/){
					bUsePreParser = false;
				}
			}
			bret = ReadBuffer(reinterpret_cast<LPTSTR>(lpData), buffer, lpFileName, bUsePreParser);
		} while(0);
		if(utfhHeap && utfhHeap != INVALID_HANDLE_VALUE){
			::HeapFree(utfhHeap, 0, lpUtfData);
			::HeapDestroy(utfhHeap);
		}
	}
	return bret;
}
bool CGrAozoraTxtParser::ReadBuffer(LPCTSTR lpBuffer, CGrTxtBuffer &buffer, LPCTSTR lpFileName, bool bUsePreParser/* = true 2.0.30.0*/)
{
	m_lastWriteTime.clear();
	if(!lpBuffer){ return false; }
	std::tstring ret;
	CGrJScript script;
	if(bUsePreParser && LoadPreParser(script, _T("text"))){
		if(PreParse(script, ret, lpFileName, lpBuffer)){
			lpBuffer = ret.c_str();
		}
	}
	//
	buffer.MoveBackLineInfo(TxtMiru::LineInfo{ 1 });
	//
	m_Indent.clear();
	m_csComment      = CSTAT_CONTENTS;
	m_bEndofContents = false;
	m_bIndentReset   = false;
	m_bRIndentReset  = false;
	m_iFontSize      = 0;
	m_fontSizeS.clear();
	m_fontSizeL.clear();
	for(auto &&item : m_RangeTextPoint){
		item.iCount = 0;
		item.iLine  = 0;
		item.iIndex = 0;
		item.iPos   = 0;
	}
	int iFileLineNo = 1;
	bool bTrans = false;
	bool cr = false;
	LPTSTR lpStr = (LPTSTR)lpBuffer;
	auto lpLine = lpBuffer;
	for(;*lpStr; ++lpStr){
		if(*lpStr == _T('\n') || cr){
			if(*lpStr == _T('\n')){
				*lpStr = _T('\0');
				addLine(buffer, lpLine, lpStr, bTrans, iFileLineNo);
				lpLine = lpStr + 1;
				++iFileLineNo;
			} else {
				addLine(buffer, lpLine, lpStr, bTrans, iFileLineNo);
				lpLine = lpStr;
				++iFileLineNo;
			}
			bTrans = false;
		}
		if(*lpStr == _T('\r')){
			cr = true;
			*lpStr = _T('\0');
		} else if(*lpStr == _T('�k')){
			bTrans = true;
			cr = false;
		} else {
			cr = false;
		}
	}
	if(lpLine){
		addLine(buffer, lpLine, lpStr, bTrans, iFileLineNo);
	}
	// �^�C�g��
	auto param_it = std::find(m_paramList.begin(), m_paramList.end(), _T("NOTITLE"));
	if(param_it == m_paramList.end()){
		// �^�C�g�����
		buffer.ForEachAll(static_cast<CGrTxtBuffer::CGrTypeFunc&&>(TitleType(buffer, m_title, m_author, m_info)));
	}
	if(m_bEndofContents){
		buffer.ForEachAll(static_cast<CGrTxtBuffer::CGrTypeFunc&&>(InfoType(buffer, m_info)));
	}
	//
	buffer.ForEachAll(static_cast<CGrTxtBuffer::CGrTypeFunc&&>(BoldType(buffer)));
	// �s�ԍ��ݒ�
	{
		iFileLineNo = 1;
		for(auto &&item : buffer.GetLineList()){
			if(item.iFileLineNo <= 0){
				item.iFileLineNo = iFileLineNo;
			} else {
				iFileLineNo = item.iFileLineNo;
			}
		}
	}
	return true;
}

