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
		UPDATE_KEYBORD,
		GOTO_NPAGE   ,
		UPDATE_CMD_UI,
		OPEN_DD_FILE ,
		OPEN_DD_FILE_NOHIST,
		OPEN_FILE    ,
	};
};

namespace TxtMiru
{
	enum class TextType {
		TEXT                  ,
		RUBY                  , // �s--�t
		RUBY_L                , // �m���u--�v�̍��Ɂu...�v�̃��r�n
		RUBY_L_START          , // �m�����Ƀ��r�t���n
		RUBY_L_END            , // �m�����Ɂu...�v�̃��r�t���I���n
		WHITE_DOT             , // �m���u--�v�ɔ��S�}�T�_�n
		ROUND_DOT             , // �m���u--�v�ɊۖT�_�n
		WHITE_ROUND_DOT       , // �m���u--�v�ɔ��ۖT�_�n
		BLACK_TRIANGLE_DOT    , // �m���u--�v�ɍ��O�p�T�_�n
		WHITE_TRIANGLE_DOT    , // �m���u--�v�ɔ��O�p�T�_�n
		DOUBLE_ROUND_DOT      , // �m���u--�v�ɓ�d�ۖT�_�n
		BULLS_EYE_DOT         , // �m���u--�v�Ɏւ̖ږT�_�n
		SALTIRE_DOT           , // �m���u--�v�ɂ΂T�_�n
		DOT                   , // �m���u--�v�ɖT�_�n
		LINE                  , // �m���u--�v�ɖT���n
		WAVE_LINE             , // �m���u--�v�ɔg���n
		DEL_LINE              , // �m���u--�v�Ɏ�����n
		UNDER_LINE            , // �m���u--�v�ɍ��T���n
		SHORT_DASHED_LINE     , // �m���u--�v�ɔj���n
		DOT_LINE              , // �m���u--�v�ɓ_���n,�m���u--�v�ɔj���n
		DOUBLE_LINE           , // �m���u--�v�ɓ�d�T���n
		WHITE_DOT_L           , // �m���u--�v�̍��ɔ��S�}�T�_�n
		ROUND_DOT_L           , // �m���u--�v�̍��ɊۖT�_�n
		WHITE_ROUND_DOT_L     , // �m���u--�v�̍��ɔ��ۖT�_�n
		BLACK_TRIANGLE_DOT_L  , // �m���u--�v�̍��ɍ��O�p�T�_�n
		WHITE_TRIANGLE_DOT_L  , // �m���u--�v�̍��ɔ��O�p�T�_�n
		DOUBLE_ROUND_DOT_L    , // �m���u--�v�̍��ɓ�d�ۖT�_�n
		BULLS_EYE_DOT_L       , // �m���u--�v�̍��Ɏւ̖ږT�_�n
		SALTIRE_DOT_L         , // �m���u--�v�ɍ��ɂ΂T�_�n
		DOT_L                 , // �m���u--�v�̍��ɖT�_�n
		LINE_L                , // �m���u--�v�̍��ɖT���n
		WAVE_LINE_L           , // �m���u--�v�̍��ɔg���n
		SHORT_DASHED_LINE_L   , // �m���u--�v�̍��ɔj���n
		DOT_LINE_L            , // �m���u--�v�̍��ɓ_���n,�m���u--�v�̍��ɔj���n
		DOUBLE_LINE_L         , // �m���u--�v�̍��ɓ�d�T���n
		RANGE_END             , // �m�����S�}�T�_�I���n�m���ۖT�_�I���n�m�����ۖT�_�I���n�m�����O�p�T�_�I���n�m�����O�p�T�_�I���n/*                       */// �m����d�ۖT�_�I���n�m���ւ̖ږT�_�I���n�m���T�_�I���n�m���T���I���n�m���g���I���n�m��������I���n/*                       */// �m�����T���I���n�m���j���I���n�m���_���I���n,�m���j���I���n�m����d�T���I���n
		NOTE                  , // �m���u--�v�̒��L�n
		NOTE_START            , // �m�����L�t���n
		NOTE_END              , // �m���u--�v�̒��L�t���I���n
		NOTE_L                , // �m���u--�v�̍��Ɂu...�v�̒��L�n
		NOTE_START_L          , // �m�����ɒ��L�t���n
		NOTE_END_L            , // �m�����Ɂu--�v�̒��L�t���I���n
		SMALL_NOTE            , // �m���i--�j�n               :  ������
		SMALL_NOTE_R          , // �m���u�v�͍s�E�������n
		SMALL_NOTE_L          , // �m���u�v�͍s���������n
		SUP_NOTE              , // �m��--�v�͏�t���������n
		SUB_NOTE              , // �m��--�v�͉��t���������n
		ERROR_STR             , //  �v�͒�{�ł́u--�v�n
		UNKOWN_ERROR          , // �m���u--�v�̓}�}�n, �m���u--�v�Ɂu�}�}�v�̒��L�n
		NO_READ               , //  ���m�����Ǖs�A--�n
		INDENT                , // �m�����̍s���������n,�m�����̍s���������A�܂�Ԃ��ā��������n,�m���V���灛�������n,�m���V���灛�������A�܂�Ԃ��ā��������n�i������j
		INDENT2               , // �m�����̍s�V�t���A�܂�Ԃ��ā��������n�i�s�����������O�̏ꍇ�j
		INDENT3               , // �m���V���灛�������n
		INDENT_START          , // �m����������--�������n
		INDENT_START2         , // �m������������s�V�t���A�܂�Ԃ��ā��������n�`�m�������Ŏ������I���n
		INDENT_END            , // �m�������Ŏ������I���n
		RINDENT               , // �m�����̍s�n�t���n
		RINDENT_START         , // �m����������n�t���n,�m����������n���灛���グ�n
		RINDENT_END           , //  �������Œn�t���I���n,�m�������Ŏ��グ�I���n
		LIMIT_CHAR_START      , // �m���������灛���l�߁n
		LIMIT_CHAR_END        , // �m�������Ŏ��l�ߏI���n
		PICTURE_LAYOUT        , // �i�m���ʐ^�i--�j����n�m���}�G�i--�j����n
		PICTURE_HALF_PAGE     , // ��
		PICTURE_FULL_PAGE     , // ���J��
		LINE_BOX_START        , // �m����������r�͂݁n
		LINE_BOX_END          , // �m�������Ōr�͂ݏI���n
		MOVING_BORDER         , // �m���u�v�͌r�͂݁n
		ACCENT                , // �k---�l�m��--�j�t���n
		OTHER_NOTE            , // �m���u--�v�n
		GUID_MARK             , //  �P�_
		OTHER                 , // �m��--�n
		OVERLAP_CHAR          , //  �B�v, ���h
		KU1_CHAR              , // ���̎��_��
		KU2_CHAR              , // ���̎��_��i���_�j
		RUBY_SEP              , //  �b
		ROTATE_NUM            , // �m���u...�v�͏c�����n
		ROTATE_NUM_AUTO       , // �c���� �������Ŕ��f
		HORZ                  , // �m���u...�v�͉��g�݁n
		HORZ_START            , // �m���������牡�g�݁n
		HORZ_END              , // �m�������ŉ��g�ݏI���n
		COMMENT_BEGIN         , // �R�����g�J�n
		COMMENT               , // �R�����g
		COMMENT_END           , // �R�����g�I��
		BOLD                  , // ����
		BOLD_START            , // ����
		BOLD_END              , // ����
		NEXT_LINE             , // ���s
		NEXT_LAYOUT           , // ���i
		NEXT_PAGE             , // ����
		NEXT_PAPER            , // ���� (��y�[�W)
		NEXT_PAPER_FIRST      , // �����J�� (�����y�[�W)
		LINE_CHAR             , // �\
		TITLE                 , // �^�C�g��
		AUTHOR                , // ����
		SUBTITLE1             , // �匩�o��
		SUBTITLE2             , // �����o��
		SUBTITLE3             , // �����o��
		SKIP_CHAR             , // �X�L�b�v(��\������)
		SKIP_CHAR_AUTO        , // �X�L�b�v(��\������)
		NOMBRE1               , // �m���u��1
		NOMBRE2               , // �m���u��2
		RUNNINGHEADS          , // ��
		ENDOFCONTENTS         , // �{���I��
		LINK                  , // Link
		TEXT_SIZE             , // �m���u...�v��n�i�K�傫�ȕ����n�m���u...�v��n�i�K�����ȕ����n
		TEXT_SIZE_L           , // �m���u...�v��n�i�K�傫�ȕ����n
		TEXT_SIZE_S           , // �m���u...�v��n�i�K�����ȕ����n
		CENTER                , // �m���y�[�W�̍��E�����n
		CAPTION               , // �m���u...�v�̓L���v�V�����n
		ID                    , // ID �y�[�W�������N�p
		FILE                  , // NextFile�p
		TXTMIRU               , // TXTMIRU�p�^�O
		MaxNum                , // TextType�̌���
	};
	//
	struct CharPoint {
		int lcol = 0; // �S�ł�ʂ��Ẳ�ʉE����̍s�� ��������������������������������������������������������
		int x    = 0; // ��ʏ�̍��W(��g) : ��        ��    ��    ����    ��    ����          ����    ��    ��
		int y    = 0; //                    : ��        ��    ��    ����    ��    ����          ����    ��    ��
		int h    = 0; //                    : ����      ��������������������������������������������������������
		int w    = 0; //                    : ��        ��    ��    ����    ��    ����          ����    ��    ��
		int ch_h = 0; // �����̍���                     ��    ��    ����    ��    ����          ����    ��    ��
		int ch_w = 0; //       ��                       ��������������������������������������������������������
		int zx   = 0; // �`��J�n�ʒu : offset�ʒu
		int zy   = 0; //
	};
	// TxtMiru::TT_LINE_BOX�̎���ch_w�͘g�̎w��Ɏg�p����
	const UINT CPBorderNoSide = 0b0000; // CharPoint.ch_w : �g�Ȃ�
	const UINT CPBorderRight  = 0b0001; // CharPoint.ch_w : �E�g
	const UINT CPBorderLeft   = 0b0010; // CharPoint.ch_w : ���g
	struct TextPoint {
		int iLine  = 0;
		int iIndex = 0;
		int iPos   = 0;
		bool operator <  (const TextPoint &tp) const { return iLine < tp.iLine || (iLine == tp.iLine && (iIndex < tp.iIndex || (iIndex == tp.iIndex && iPos <  tp.iPos))); }
		bool operator <= (const TextPoint &tp) const { return iLine < tp.iLine || (iLine == tp.iLine && (iIndex < tp.iIndex || (iIndex == tp.iIndex && iPos <= tp.iPos))); }
		bool operator == (const TextPoint &tp) const { return iLine == tp.iLine && iIndex == tp.iIndex && iPos == tp.iPos; }
	};
	struct TextListPos {
		int iIndex = 0;
		int iPos   = 0;
		bool operator <(const TextListPos &pos) const { return iIndex < pos.iIndex || (iIndex == pos.iIndex && iPos < pos.iPos); }
		bool operator ==(const TextListPos &pos) const { return pos.iIndex == iIndex && pos.iPos == iPos; }
	};
	struct TextTurnPos {
		int iIndex       = 0;
		int iPos         = 0;
		int iLineNoInAll = 0;
		int iNum         = 0;
		int iHeight      = 0;
		int iRIndent     = -1; // �n�t�� (�s�̓r���Œn�t���ɂȂ����ꍇ�ɕK�v
		bool operator < (const TextTurnPos &ttp) const { return iIndex < ttp.iIndex || (iIndex == ttp.iIndex && iPos <  ttp.iPos); }
		bool operator <=(const TextTurnPos &ttp) const { return iIndex < ttp.iIndex || (iIndex == ttp.iIndex && iPos <= ttp.iPos); }
		bool operator < (const TextListPos &pos) const { return iIndex < pos.iIndex || (iIndex == pos.iIndex && iPos <  pos.iPos); }
		bool operator <=(const TextListPos &pos) const { return iIndex < pos.iIndex || (iIndex == pos.iIndex && iPos <= pos.iPos); }
		bool operator ==(const TextListPos &pos) const { return pos.iIndex == iIndex && pos.iPos == iPos; }
	};
	const UINT TTPStatusNone = 0b0000; // �Ȃ�
	const UINT TTPStatusBold = 0b0001; // ����
	const UINT TTPStatusHorz = 0b0010; // ���g��
	struct TextStyle {
		UINT uStatus = TTPStatusNone;
		int fontSize = 0;
	};
	struct TextInfo {
		std::tstring str;
		WORD chrType;
		TextListPos tpBegin;
		TextListPos tpEnd;
		TextType textType;
		TextInfo() : chrType(0xffff), tpBegin{0,0}, tpEnd{0,-1}, textType(TextType::TEXT){}
		TextInfo(std::tstring &&s, WORD ct, TextListPos b, TextListPos e, TextType tt) : str(s), chrType(ct), tpBegin(b), tpEnd(e), textType(tt){}
		TextInfo(std::tstring &&s, WORD ct, int bi, int bp, int ei, int ep, TextType tt) : str(s), chrType(ct), tpBegin{bi, bp}, tpEnd{ei, ep}, textType(tt){}

		TextInfo(std::tstring &&s, WORD t) : str(s), chrType(t), tpBegin(), tpEnd(), textType(TextType::TEXT){}
		TextInfo(std::tstring &&s, WORD w, TextType t, TextListPos b) : str(s), chrType(w), textType(t), tpBegin(b), tpEnd(b){}

		TextInfo(LPCTSTR s, WORD ct, int bi, int bp, int ei, int ep, TextType tt) : str(s), chrType(ct), tpBegin{bi, bp}, tpEnd{ei, ep}, textType(tt){}
		TextInfo(LPCTSTR s, WORD t) : str(s), chrType(t), tpBegin(), tpEnd(), textType(TextType::TEXT){}
		TextInfo(LPCTSTR s, WORD w, TextType t, TextListPos b) : str(s), chrType(w), textType(t), tpBegin(b), tpEnd(b){}
	};
	//
	struct TextParserPoint : public TextPoint {
		TextParserPoint(int l, const TextListPos &tp) : TextPoint{l, tp.iIndex, tp.iPos}{}
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
	using TextInfoList  = std::vector<TextInfo>;
	using CharPointMap  = std::map<TextPoint, CharPoint >;
	using TextStyleMap  = std::map<TextPoint, TextStyle >;
	using TextOffsetMap = std::map<TextPoint, TextOffset>;
	//
	struct LineInfo {
		int iFileLineNo   = -1; // �t�@�C�����s��
		int iStartCol     = 0;
		int iEndCol       = 0;
		int iFirstIndent  = 0; // �V�t��
		int iSecondIndent = 0; // �V�t��(�Q�s�ڈȍ~)
		int iRIndent      = 0; // �n�t��
		TextInfoList text_list;
		std::vector<TextTurnPos> tlpTurnList;
	};
	using LineList = std::vector<LineInfo>;
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
	using TxtLayoutList = std::vector<TxtLayout>;

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
	using PictPosMap = std::map<TextPoint,PictPos>;
};

#endif // __TXTMIRUDEF_H__
