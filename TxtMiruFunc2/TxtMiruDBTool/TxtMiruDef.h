#ifndef __TXTMIRUDEF_H__
#define __TXTMIRUDEF_H__

#include <windows.h>
#include "stltchar.h"
#include <vector>
#include <map>

#define ARCFILE_SPLIT_CHAR _T("|") // URI, �t�@�C�����Ŏg�p�ł��Ȃ���������؂蕶���ɂ���
enum {
	ID_TEXTVIEW = 1,
};

namespace TxtDocMessage
{
	enum {
		OPEN         ,
		GOTO_PAGE    ,
		NEXT_PAGE    ,
		PREV_PAGE    ,
		FIRST_PAGE   ,
		END_PAGE     ,
		RELOAD       ,
		ADD_BOOKMARK ,
		GOTO_BOOKMARK,
		SHOW_BOOKMARK,
		SEARCH       ,
		SEARCH_NEXT  ,
		SEARCH_PREV  ,
		CONFIG       ,
		UPDATE_CONFIG = 41000,
		UPDATE_LAYOUT,
		UPDATE_STYLE_LIST ,
		GOTO_NPAGE   ,
		UPDATE_CMD_UI,
		OPEN_DD_FILE ,
		OPEN_DD_FILE_NOHIST,
		OPEN_FILE    ,
	};
};

namespace TxtMiru
{
	enum TextType {
		TT_TEXT                  ,
		TT_RUBY                  , // �s--�t
		TT_RUBY_L                , // �m���u--�v�̍��Ɂu...�v�̃��r�n
		TT_RUBY_L_START          , // �m�����Ƀ��r�t���n
		TT_RUBY_L_END            , // �m�����Ɂu...�v�̃��r�t���I���n
		TT_WHITE_DOT             , // �m���u--�v�ɔ��S�}�T�_�n
		TT_ROUND_DOT             , // �m���u--�v�ɊۖT�_�n
		TT_WHITE_ROUND_DOT       , // �m���u--�v�ɔ��ۖT�_�n
		TT_BLACK_TRIANGLE_DOT    , // �m���u--�v�ɍ��O�p�T�_�n
		TT_WHITE_TRIANGLE_DOT    , // �m���u--�v�ɔ��O�p�T�_�n
		TT_DOUBLE_ROUND_DOT      , // �m���u--�v�ɓ�d�ۖT�_�n
		TT_BULLS_EYE_DOT         , // �m���u--�v�Ɏւ̖ږT�_�n
		TT_SALTIRE_DOT           , // �m���u--�v�ɂ΂T�_�n
		TT_DOT                   , // �m���u--�v�ɖT�_�n
		TT_LINE                  , // �m���u--�v�ɖT���n
		TT_WAVE_LINE             , // �m���u--�v�ɔg���n
		TT_DEL_LINE              , // �m���u--�v�Ɏ�����n
		TT_UNDER_LINE            , // �m���u--�v�ɍ��T���n
		TT_SHORT_DASHED_LINE     , // �m���u--�v�ɔj���n
		TT_DOT_LINE              , // �m���u--�v�ɓ_���n,�m���u--�v�ɔj���n
		TT_DOUBLE_LINE           , // �m���u--�v�ɓ�d�T���n
		TT_WHITE_DOT_L           , // �m���u--�v�̍��ɔ��S�}�T�_�n
		TT_ROUND_DOT_L           , // �m���u--�v�̍��ɊۖT�_�n
		TT_WHITE_ROUND_DOT_L     , // �m���u--�v�̍��ɔ��ۖT�_�n
		TT_BLACK_TRIANGLE_DOT_L  , // �m���u--�v�̍��ɍ��O�p�T�_�n
		TT_WHITE_TRIANGLE_DOT_L  , // �m���u--�v�̍��ɔ��O�p�T�_�n
		TT_DOUBLE_ROUND_DOT_L    , // �m���u--�v�̍��ɓ�d�ۖT�_�n
		TT_BULLS_EYE_DOT_L       , // �m���u--�v�̍��Ɏւ̖ږT�_�n
		TT_SALTIRE_DOT_L         , // �m���u--�v�ɍ��ɂ΂T�_�n
		TT_DOT_L                 , // �m���u--�v�̍��ɖT�_�n
		TT_LINE_L                , // �m���u--�v�̍��ɖT���n
		TT_WAVE_LINE_L           , // �m���u--�v�̍��ɔg���n
		TT_SHORT_DASHED_LINE_L   , // �m���u--�v�̍��ɔj���n
		TT_DOT_LINE_L            , // �m���u--�v�̍��ɓ_���n,�m���u--�v�̍��ɔj���n
		TT_DOUBLE_LINE_L         , // �m���u--�v�̍��ɓ�d�T���n
		TT_RANGE_END             , // �m�����S�}�T�_�I���n�m���ۖT�_�I���n�m�����ۖT�_�I���n�m�����O�p�T�_�I���n�m�����O�p�T�_�I���n
		/*                       */// �m����d�ۖT�_�I���n�m���ւ̖ږT�_�I���n�m���T�_�I���n�m���T���I���n�m���g���I���n�m��������I���n
		/*                       */// �m�����T���I���n�m���j���I���n�m���_���I���n,�m���j���I���n�m����d�T���I���n
		TT_NOTE                  , // �m���u--�v�̒��L�n
		TT_NOTE_START            , // �m�����L�t���n
		TT_NOTE_END              , // �m���u--�v�̒��L�t���I���n
		TT_NOTE_L                , // �m���u--�v�̍��Ɂu...�v�̒��L�n
		TT_NOTE_START_L          , // �m�����ɒ��L�t���n
		TT_NOTE_END_L            , // �m�����Ɂu--�v�̒��L�t���I���n
		TT_SMALL_NOTE            , // �m���i--�j�n               :  ������
		TT_SMALL_NOTE_R          , // �m���u�v�͍s�E�������n
		TT_SMALL_NOTE_L          , // �m���u�v�͍s���������n
		TT_SUP_NOTE              , // �m��--�v�͏�t���������n
		TT_SUB_NOTE              , // �m��--�v�͉��t���������n
		TT_ERROR                 , //  �v�͒�{�ł́u--�v�n
		TT_UNKOWN_ERROR          , // �m���u--�v�̓}�}�n, �m���u--�v�Ɂu�}�}�v�̒��L�n
		TT_NO_READ               , //  ���m�����Ǖs�A--�n
		TT_INDENT                , // �m�����̍s���������n,�m�����̍s���������A�܂�Ԃ��ā��������n,�m���V���灛�������n,�m���V���灛�������A�܂�Ԃ��ā��������n�i������j
		TT_INDENT2               , // �m�����̍s�V�t���A�܂�Ԃ��ā��������n�i�s�����������O�̏ꍇ�j
		TT_INDENT3               , // �m���V���灛�������n
		TT_INDENT_START          , // �m����������--�������n
		TT_INDENT_START2         , // �m������������s�V�t���A�܂�Ԃ��ā��������n�`�m�������Ŏ������I���n
		TT_INDENT_END            , // �m�������Ŏ������I���n
		TT_RINDENT               , // �m�����̍s�n�t���n
		TT_RINDENT_START         , // �m����������n�t���n,�m����������n���灛���グ�n
		TT_RINDENT_END           , //  �������Œn�t���I���n,�m�������Ŏ��グ�I���n
		TT_LIMIT_CHAR_START      , // �m���������灛���l�߁n
		TT_LIMIT_CHAR_END        , // �m�������Ŏ��l�ߏI���n
		TT_PICTURE_LAYOUT        , // �i�m���ʐ^�i--�j����n�m���}�G�i--�j����n
		TT_PICTURE_HALF_PAGE     , // ��
		TT_PICTURE_FULL_PAGE     , // ���J��
		TT_LINE_BOX_START        , // �m����������r�͂݁n
		TT_LINE_BOX_END          , // �m�������Ōr�͂ݏI���n
		TT_MOVING_BORDER         , // �m���u�v�͌r�͂݁n
		TT_ACCENT                , // �k---�l�m��--�j�t���n
		TT_OTHER_NOTE            , // �m���u--�v�n
		TT_GUID_MARK             , //  �P�_
		TT_OTHER                 , // �m��--�n
		TT_OVERLAP_CHAR          , //  �B�v, ���h
		TT_KU1_CHAR              , // ���̎��_��
		TT_KU2_CHAR              , // ���̎��_��i���_�j
		TT_RUBY_SEP              , //  �b
		TT_ROTATE_NUM            , // �m���u...�v�͏c�����n
		TT_ROTATE_NUM_AUTO       , // �c���� �������Ŕ��f
		TT_COMMENT_BEGIN         , // �R�����g�J�n
		TT_COMMENT               , // �R�����g
		TT_COMMENT_END           , // �R�����g�I��
		TT_BOLD                  , // ����
		TT_BOLD_START            , // ����
		TT_BOLD_END              , // ����
		TT_NEXT_LINE             , // ���s
		TT_NEXT_LAYOUT           , // ���i
		TT_NEXT_PAGE             , // ����
		TT_NEXT_PAPER            , // ���� (��y�[�W)
		TT_NEXT_PAPER_FIRST      , // �����J�� (�����y�[�W) 
		TT_LINE_CHAR             , // �\
		TT_TITLE                 , // �^�C�g��
		TT_AUTHOR                , // ����
		TT_SUBTITLE1             , // �匩�o��
		TT_SUBTITLE2             , // �����o��
		TT_SUBTITLE3             , // �����o��
		TT_SKIP_CHAR             , // �X�L�b�v(��\������)
		TT_SKIP_CHAR_AUTO        , // �X�L�b�v(��\������)
		TT_NOMBRE1               , // �m���u��1
		TT_NOMBRE2               , // �m���u��2
		TT_RUNNINGHEADS          , // ��
		TT_ENDOFCONTENTS         , // �{���I��
		TT_LINK                  , // Link
		TT_TEXT_SIZE             , // �m���u...�v��n�i�K�傫�ȕ����n�m���u...�v��n�i�K�����ȕ����n
		TT_TEXT_SIZE_L           , // �m���u...�v��n�i�K�傫�ȕ����n
		TT_TEXT_SIZE_S           , // �m���u...�v��n�i�K�����ȕ����n
		TT_CENTER                , // �m���y�[�W�̍��E�����n 
		TT_ID                    , // ID �y�[�W�������N�p
		TT_FILE                  , // NextFile�p
		TT_TXTMIRU               , // TXTMIRU�p�^�O
		TT_MaxNum                , // TextType�̌���
	};
	//
	struct CharPoint {
		int lcol; // �S�ł�ʂ��Ẳ�ʉE����̍s�� ��������������������������������������������������������
		int x   ; // ��ʏ�̍��W(��g) : ��        ��    ��    ����    ��    ����          ����    ��    ��
		int y   ; //                    : ��        ��    ��    ����    ��    ����          ����    ��    ��
		int h   ; //                    : ����      ��������������������������������������������������������
		int w   ; //                    : ��        ��    ��    ����    ��    ����          ����    ��    ��
		int ch_h; // �����̍���                     ��    ��    ����    ��    ����          ����    ��    ��
		int ch_w; //       ��                       ��������������������������������������������������������
		int zx  ; // �`��J�n�ʒu : offset�ʒu
		int zy  ; // 
		CharPoint() : lcol(0), x(0), y(0), h(0), w(0), ch_h(0), ch_w(0), zx(0), zy(0){}
		CharPoint(int lcol0, int x0, int y0, int h0, int w0, int ch, int cw) : lcol(lcol0), x(x0),y(y0),h(h0),w(w0),ch_h(ch),ch_w(cw),zx(x0),zy(y0){}
	};
	struct TextPoint {
		int iLine;
		int iIndex;
		int iPos;
		TextPoint() : iLine(0), iIndex(0), iPos(0){}
		TextPoint(int l, int i, int p) : iLine(l), iIndex(i), iPos(p){}
		bool operator <  (const TextPoint &tp) const { return iLine < tp.iLine || (iLine == tp.iLine && (iIndex < tp.iIndex || (iIndex == tp.iIndex && iPos <  tp.iPos))); }
		bool operator <= (const TextPoint &tp) const { return iLine < tp.iLine || (iLine == tp.iLine && (iIndex < tp.iIndex || (iIndex == tp.iIndex && iPos <= tp.iPos))); }
		bool operator == (const TextPoint &tp) const { return iLine == tp.iLine && iIndex == tp.iIndex && iPos == tp.iPos; }
	};
	struct TextListPos {
		int iIndex;
		int iPos;
		TextListPos() : iIndex(0), iPos(0){}
		TextListPos(int i, int p) : iIndex(i), iPos(p){}
		bool operator <(const TextListPos &pos) const { return iIndex < pos.iIndex || (iIndex == pos.iIndex && iPos < pos.iPos); }
		bool operator ==(const TextListPos &pos) const { return pos.iIndex == iIndex && pos.iPos == iPos; }
	};
	struct TextTurnPos {
		int iIndex;
		int iPos;
		int iNum;
		int iHeight;
		int iLineNoInAll;
		int iRIndent; // �n�t�� (�s�̓r���Œn�t���ɂȂ����ꍇ�ɕK�v
		TextTurnPos() : iIndex(0), iPos(0), iNum(0), iHeight(0), iLineNoInAll(0), iRIndent(-1){}
		TextTurnPos(int i, int p, int l) : iIndex(i), iPos(p), iNum(0), iHeight(0), iLineNoInAll(l), iRIndent(-1){}
		bool operator < (const TextTurnPos &ttp) const { return iIndex < ttp.iIndex || (iIndex == ttp.iIndex && iPos <  ttp.iPos); }
		bool operator <=(const TextTurnPos &ttp) const { return iIndex < ttp.iIndex || (iIndex == ttp.iIndex && iPos <= ttp.iPos); }
		bool operator < (const TextListPos &pos) const { return iIndex < pos.iIndex || (iIndex == pos.iIndex && iPos <  pos.iPos); }
		bool operator <=(const TextListPos &pos) const { return iIndex < pos.iIndex || (iIndex == pos.iIndex && iPos <= pos.iPos); }
		bool operator ==(const TextListPos &pos) const { return pos.iIndex == iIndex && pos.iPos == iPos; }
	};
	struct TextStyle {
		bool bold;
		int fontSize;
	};
	struct TextInfo {
		std::tstring str;
		WORD chrType;
		TextListPos tpBegin;
		TextListPos tpEnd;
		TextType textType;
		TextInfo() : chrType(0xffff), tpBegin(0,0), tpEnd(0,-1), textType(TT_TEXT){}
		TextInfo(std::tstring &s, WORD ct, TextListPos b, TextListPos e, TextType tt) : str(s), chrType(ct), tpBegin(b), tpEnd(e), textType(tt){}
		TextInfo(std::tstring &s, WORD ct, int bi, int bp, int ei, int ep, TextType tt) : str(s), chrType(ct), tpBegin(bi, bp), tpEnd(ei, ep), textType(tt){}

		TextInfo(std::tstring &s, WORD t) : str(s), chrType(t), tpBegin(), tpEnd(), textType(TT_TEXT){}
		TextInfo(std::tstring &s, WORD w, TextType t, TextListPos b) : str(s), chrType(w), textType(t), tpBegin(b), tpEnd(b){}

		TextInfo(LPCTSTR s, WORD ct, int bi, int bp, int ei, int ep, TextType tt) : str(s), chrType(ct), tpBegin(bi, bp), tpEnd(ei, ep), textType(tt){}
		TextInfo(LPCTSTR s, WORD t) : str(s), chrType(t), tpBegin(), tpEnd(), textType(TT_TEXT){}
		TextInfo(LPCTSTR s, WORD w, TextType t, TextListPos b) : str(s), chrType(w), textType(t), tpBegin(b), tpEnd(b){}
	};
	//
	struct TextParserPoint : public TextPoint {
		TextParserPoint(int l, const TextListPos &tp) : TextPoint(l, tp.iIndex, tp.iPos){}
	};
	// offsetType :
	//    ��������                        ������������������������
	//  1 ��    ����������                ��    ����    ����    ������������������
	//  2 ��    ����    ����������        ����������    ����    ����    ����    ����������
	//  3 ��    ����    ����    ����������        ����������    ����    ������������    ��
	//  4 ��    ����    ����    ����    ��                ����������������        ��������
	//    ��������������������������������
	//      FULL   BOTTOM3 BOTTOM2 BOTTOM1  TOP1    TOP2    TOP3   L2T3    L2T2    L3T3    NONE
	//      1111    0111    0011    0001    1000    1100    1110   0110    0100    0010    0000
	struct TextOffset {
		WORD offsetType; // XXXX:YYYY
		int x;
		int y;
		int h;
		int w;
	};
	//
	typedef std::vector<TextInfo> TextInfoList;
	typedef std::map<TextPoint, CharPoint > CharPointMap ;
	typedef std::map<TextPoint, TextStyle > TextStyleMap ;
	typedef std::map<TextPoint, TextOffset> TextOffsetMap;
	//
	struct LineInfo {
		int iStartCol;
		int iEndCol;
		int iFirstIndent ; // �V�t��
		int iSecondIndent; // �V�t��(�Q�s�ڈȍ~)
		int iRIndent     ; // �n�t��
		TextInfoList text_list;
		std::vector<TextTurnPos> tlpTurnList;
		LineInfo() : iStartCol(0), iEndCol(0), iFirstIndent(0), iSecondIndent(0), iRIndent(0){}
	};
	typedef std::vector<LineInfo> LineList;
	//
	struct TxtLayout
	{
		int left;
		int top;
		int right;
		int bottom;
		int lines;
		int characters;
	};
	typedef std::vector<TxtLayout> TxtLayoutList;

	struct LineSize
	{
		int width;
		int space;
	};

	struct PictPos
	{
		int iStartLayout;
		int iEndLayout;
	};
	typedef std::map<TextPoint,PictPos> PictPosMap;
};

#endif // __TXTMIRUDEF_H__
