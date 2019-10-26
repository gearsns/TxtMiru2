#ifndef __TXTMIRUDEF_H__
#define __TXTMIRUDEF_H__

#include <windows.h>
#include "stltchar.h"
#include <vector>
#include <map>

#define ARCFILE_SPLIT_CHAR _T("|") // URI, ファイル名で使用できない文字を区切り文字にする
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
		RUBY                  , // 《--》
		RUBY_L                , // ［＃「--」の左に「...」のルビ］
		RUBY_L_START          , // ［＃左にルビ付き］
		RUBY_L_END            , // ［＃左に「...」のルビ付き終わり］
		WHITE_DOT             , // ［＃「--」に白ゴマ傍点］
		ROUND_DOT             , // ［＃「--」に丸傍点］
		WHITE_ROUND_DOT       , // ［＃「--」に白丸傍点］
		BLACK_TRIANGLE_DOT    , // ［＃「--」に黒三角傍点］
		WHITE_TRIANGLE_DOT    , // ［＃「--」に白三角傍点］
		DOUBLE_ROUND_DOT      , // ［＃「--」に二重丸傍点］
		BULLS_EYE_DOT         , // ［＃「--」に蛇の目傍点］
		SALTIRE_DOT           , // ［＃「--」にばつ傍点］
		DOT                   , // ［＃「--」に傍点］
		LINE                  , // ［＃「--」に傍線］
		WAVE_LINE             , // ［＃「--」に波線］
		DEL_LINE              , // ［＃「--」に取消線］
		UNDER_LINE            , // ［＃「--」に左傍線］
		SHORT_DASHED_LINE     , // ［＃「--」に破線］
		DOT_LINE              , // ［＃「--」に点線］,［＃「--」に破線］
		DOUBLE_LINE           , // ［＃「--」に二重傍線］
		WHITE_DOT_L           , // ［＃「--」の左に白ゴマ傍点］
		ROUND_DOT_L           , // ［＃「--」の左に丸傍点］
		WHITE_ROUND_DOT_L     , // ［＃「--」の左に白丸傍点］
		BLACK_TRIANGLE_DOT_L  , // ［＃「--」の左に黒三角傍点］
		WHITE_TRIANGLE_DOT_L  , // ［＃「--」の左に白三角傍点］
		DOUBLE_ROUND_DOT_L    , // ［＃「--」の左に二重丸傍点］
		BULLS_EYE_DOT_L       , // ［＃「--」の左に蛇の目傍点］
		SALTIRE_DOT_L         , // ［＃「--」に左にばつ傍点］
		DOT_L                 , // ［＃「--」の左に傍点］
		LINE_L                , // ［＃「--」の左に傍線］
		WAVE_LINE_L           , // ［＃「--」の左に波線］
		SHORT_DASHED_LINE_L   , // ［＃「--」の左に破線］
		DOT_LINE_L            , // ［＃「--」の左に点線］,［＃「--」の左に破線］
		DOUBLE_LINE_L         , // ［＃「--」の左に二重傍線］
		RANGE_END             , // ［＃白ゴマ傍点終わり］［＃丸傍点終わり］［＃白丸傍点終わり］［＃黒三角傍点終わり］［＃白三角傍点終わり］/*                       */// ［＃二重丸傍点終わり］［＃蛇の目傍点終わり］［＃傍点終わり］［＃傍線終わり］［＃波線終わり］［＃取消線終わり］/*                       */// ［＃左傍線終わり］［＃破線終わり］［＃点線終わり］,［＃破線終わり］［＃二重傍線終わり］
		NOTE                  , // ［＃「--」の注記］
		NOTE_START            , // ［＃注記付き］
		NOTE_END              , // ［＃「--」の注記付き終わり］
		NOTE_L                , // ［＃「--」の左に「...」の注記］
		NOTE_START_L          , // ［＃左に注記付き］
		NOTE_END_L            , // ［＃左に「--」の注記付き終わり］
		SMALL_NOTE            , // ［＃（--）］               :  小書き
		SMALL_NOTE_R          , // ［＃「」は行右小書き］
		SMALL_NOTE_L          , // ［＃「」は行左小書き］
		SUP_NOTE              , // ［＃--」は上付き小文字］
		SUB_NOTE              , // ［＃--」は下付き小文字］
		ERROR_STR             , //  」は底本では「--」］
		UNKOWN_ERROR          , // ［＃「--」はママ］, ［＃「--」に「ママ」の注記］
		NO_READ               , //  ※［＃判読不可、--］
		INDENT                , // ［＃この行○字下げ］,［＃この行○字下げ、折り返して○字下げ］,［＃天から○字下げ］,［＃天から○字下げ、折り返して○字下げ］（これも可）
		INDENT2               , // ［＃この行天付き、折り返して○字下げ］（行頭字下げが０の場合）
		INDENT3               , // ［＃天から○字下げ］
		INDENT_START          , // ［＃ここから--字下げ］
		INDENT_START2         , // ［＃ここから改行天付き、折り返して○字下げ］～［＃ここで字下げ終わり］
		INDENT_END            , // ［＃ここで字下げ終わり］
		RINDENT               , // ［＃この行地付き］
		RINDENT_START         , // ［＃ここから地付き］,［＃ここから地から○字上げ］
		RINDENT_END           , //  ＃ここで地付き終わり］,［＃ここで字上げ終わり］
		LIMIT_CHAR_START      , // ［＃ここから○字詰め］
		LIMIT_CHAR_END        , // ［＃ここで字詰め終わり］
		PICTURE_LAYOUT        , // 段［＃写真（--）入る］［＃挿絵（--）入る］
		PICTURE_HALF_PAGE     , // 頁
		PICTURE_FULL_PAGE     , // 見開き
		LINE_BOX_START        , // ［＃ここから罫囲み］
		LINE_BOX_END          , // ［＃ここで罫囲み終わり］
		MOVING_BORDER         , // ［＃「」は罫囲み］
		ACCENT                , // 〔---〕［＃--）付き］
		OTHER_NOTE            , // ［＃「--」］
		GUID_MARK             , //  訓点
		OTHER                 , // ［＃--］
		OVERLAP_CHAR          , //  。」, か”
		KU1_CHAR              , // くの字点上
		KU2_CHAR              , // くの字点上（濁点）
		RUBY_SEP              , //  ｜
		ROTATE_NUM            , // ［＃「...」は縦中横］
		ROTATE_NUM_AUTO       , // 縦中横 文字数で判断
		HORZ                  , // ［＃「...」は横組み］
		HORZ_START            , // ［＃ここから横組み］
		HORZ_END              , // ［＃ここで横組み終わり］
		COMMENT_BEGIN         , // コメント開始
		COMMENT               , // コメント
		COMMENT_END           , // コメント終了
		BOLD                  , // 太字
		BOLD_START            , // 太字
		BOLD_END              , // 太字
		NEXT_LINE             , // 改行
		NEXT_LAYOUT           , // 改段
		NEXT_PAGE             , // 改頁
		NEXT_PAPER            , // 改丁 (奇数ページ)
		NEXT_PAPER_FIRST      , // 改見開き (偶数ページ)
		LINE_CHAR             , // ―
		TITLE                 , // タイトル
		AUTHOR                , // 著者
		SUBTITLE1             , // 大見出し
		SUBTITLE2             , // 中見出し
		SUBTITLE3             , // 小見出し
		SKIP_CHAR             , // スキップ(非表示文字)
		SKIP_CHAR_AUTO        , // スキップ(非表示文字)
		NOMBRE1               , // ノンブル1
		NOMBRE2               , // ノンブル2
		RUNNINGHEADS          , // 柱
		ENDOFCONTENTS         , // 本文終了
		LINK                  , // Link
		TEXT_SIZE             , // ［＃「...」はn段階大きな文字］［＃「...」はn段階小さな文字］
		TEXT_SIZE_L           , // ［＃「...」はn段階大きな文字］
		TEXT_SIZE_S           , // ［＃「...」はn段階小さな文字］
		CENTER                , // ［＃ページの左右中央］
		CAPTION               , // ［＃「...」はキャプション］
		ID                    , // ID ページ内リンク用
		FILE                  , // NextFile用
		TXTMIRU               , // TXTMIRU用タグ
		MaxNum                , // TextTypeの件数
	};
	//
	struct CharPoint {
		int lcol = 0; // 全頁を通しての画面右からの行数 ┏━━┯━━┓┏━━┰──┐┏━━━━━┓┌──┰━━┓
		int x    = 0; // 画面上の座標(大枠) : 左        ┃    │    ┃┃    ┃    │┃          ┃│    ┃    ┃
		int y    = 0; //                    : 上        ┃    │    ┃┃    ┃    │┃          ┃│    ┃    ┃
		int h    = 0; //                    : 高さ      ┠──┼──┨┠──╂──┤┝━━━━━┥├──┼━━┥
		int w    = 0; //                    : 幅        ┃    │    ┃┃    ┃    ││          ││    │    │
		int ch_h = 0; // 文字の高さ                     ┃    │    ┃┃    ┃    ││          ││    │    │
		int ch_w = 0; //       幅                       ┗━━┷━━┛┗━━┸──┘└─────┘└──┴──┘
		int zx   = 0; // 描画開始位置 : offset位置
		int zy   = 0; //
	};
	// TxtMiru::TT_LINE_BOXの時はch_wは枠の指定に使用する
	const UINT CPBorderNoSide = 0b0000; // CharPoint.ch_w : 枠なし
	const UINT CPBorderRight  = 0b0001; // CharPoint.ch_w : 右枠
	const UINT CPBorderLeft   = 0b0010; // CharPoint.ch_w : 左枠
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
		int iRIndent     = -1; // 地付き (行の途中で地付きになった場合に必要
		bool operator < (const TextTurnPos &ttp) const { return iIndex < ttp.iIndex || (iIndex == ttp.iIndex && iPos <  ttp.iPos); }
		bool operator <=(const TextTurnPos &ttp) const { return iIndex < ttp.iIndex || (iIndex == ttp.iIndex && iPos <= ttp.iPos); }
		bool operator < (const TextListPos &pos) const { return iIndex < pos.iIndex || (iIndex == pos.iIndex && iPos <  pos.iPos); }
		bool operator <=(const TextListPos &pos) const { return iIndex < pos.iIndex || (iIndex == pos.iIndex && iPos <= pos.iPos); }
		bool operator ==(const TextListPos &pos) const { return pos.iIndex == iIndex && pos.iPos == iPos; }
	};
	const UINT TTPStatusNone = 0b0000; // なし
	const UINT TTPStatusBold = 0b0001; // 太字
	const UINT TTPStatusHorz = 0b0010; // 横組み
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
	//    ┌──┐                        ┌──┐┌──┐┌──┐
	//  1 │    │┌──┐                │    ││    ││    │┌──┐┌──┐
	//  2 │    ││    │┌──┐        └──┘│    ││    ││    ││    │┌──┐
	//  3 │    ││    ││    │┌──┐        └──┘│    ││    │└──┘│    │
	//  4 │    ││    ││    ││    │                └──┘└──┘        └──┘
	//    └──┘└──┘└──┘└──┘
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
		int iFileLineNo   = -1; // ファイル内行番
		int iStartCol     = 0;
		int iEndCol       = 0;
		int iFirstIndent  = 0; // 天付き
		int iSecondIndent = 0; // 天付き(２行目以降)
		int iRIndent      = 0; // 地付き
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
