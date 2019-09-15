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
		TT_RUBY                  , // 《--》
		TT_RUBY_L                , // ［＃「--」の左に「...」のルビ］
		TT_RUBY_L_START          , // ［＃左にルビ付き］
		TT_RUBY_L_END            , // ［＃左に「...」のルビ付き終わり］
		TT_WHITE_DOT             , // ［＃「--」に白ゴマ傍点］
		TT_ROUND_DOT             , // ［＃「--」に丸傍点］
		TT_WHITE_ROUND_DOT       , // ［＃「--」に白丸傍点］
		TT_BLACK_TRIANGLE_DOT    , // ［＃「--」に黒三角傍点］
		TT_WHITE_TRIANGLE_DOT    , // ［＃「--」に白三角傍点］
		TT_DOUBLE_ROUND_DOT      , // ［＃「--」に二重丸傍点］
		TT_BULLS_EYE_DOT         , // ［＃「--」に蛇の目傍点］
		TT_SALTIRE_DOT           , // ［＃「--」にばつ傍点］
		TT_DOT                   , // ［＃「--」に傍点］
		TT_LINE                  , // ［＃「--」に傍線］
		TT_WAVE_LINE             , // ［＃「--」に波線］
		TT_DEL_LINE              , // ［＃「--」に取消線］
		TT_UNDER_LINE            , // ［＃「--」に左傍線］
		TT_SHORT_DASHED_LINE     , // ［＃「--」に破線］
		TT_DOT_LINE              , // ［＃「--」に点線］,［＃「--」に破線］
		TT_DOUBLE_LINE           , // ［＃「--」に二重傍線］
		TT_WHITE_DOT_L           , // ［＃「--」の左に白ゴマ傍点］
		TT_ROUND_DOT_L           , // ［＃「--」の左に丸傍点］
		TT_WHITE_ROUND_DOT_L     , // ［＃「--」の左に白丸傍点］
		TT_BLACK_TRIANGLE_DOT_L  , // ［＃「--」の左に黒三角傍点］
		TT_WHITE_TRIANGLE_DOT_L  , // ［＃「--」の左に白三角傍点］
		TT_DOUBLE_ROUND_DOT_L    , // ［＃「--」の左に二重丸傍点］
		TT_BULLS_EYE_DOT_L       , // ［＃「--」の左に蛇の目傍点］
		TT_SALTIRE_DOT_L         , // ［＃「--」に左にばつ傍点］
		TT_DOT_L                 , // ［＃「--」の左に傍点］
		TT_LINE_L                , // ［＃「--」の左に傍線］
		TT_WAVE_LINE_L           , // ［＃「--」の左に波線］
		TT_SHORT_DASHED_LINE_L   , // ［＃「--」の左に破線］
		TT_DOT_LINE_L            , // ［＃「--」の左に点線］,［＃「--」の左に破線］
		TT_DOUBLE_LINE_L         , // ［＃「--」の左に二重傍線］
		TT_RANGE_END             , // ［＃白ゴマ傍点終わり］［＃丸傍点終わり］［＃白丸傍点終わり］［＃黒三角傍点終わり］［＃白三角傍点終わり］
		/*                       */// ［＃二重丸傍点終わり］［＃蛇の目傍点終わり］［＃傍点終わり］［＃傍線終わり］［＃波線終わり］［＃取消線終わり］
		/*                       */// ［＃左傍線終わり］［＃破線終わり］［＃点線終わり］,［＃破線終わり］［＃二重傍線終わり］
		TT_NOTE                  , // ［＃「--」の注記］
		TT_NOTE_START            , // ［＃注記付き］
		TT_NOTE_END              , // ［＃「--」の注記付き終わり］
		TT_NOTE_L                , // ［＃「--」の左に「...」の注記］
		TT_NOTE_START_L          , // ［＃左に注記付き］
		TT_NOTE_END_L            , // ［＃左に「--」の注記付き終わり］
		TT_SMALL_NOTE            , // ［＃（--）］               :  小書き
		TT_SMALL_NOTE_R          , // ［＃「」は行右小書き］
		TT_SMALL_NOTE_L          , // ［＃「」は行左小書き］
		TT_SUP_NOTE              , // ［＃--」は上付き小文字］
		TT_SUB_NOTE              , // ［＃--」は下付き小文字］
		TT_ERROR                 , //  」は底本では「--」］
		TT_UNKOWN_ERROR          , // ［＃「--」はママ］, ［＃「--」に「ママ」の注記］
		TT_NO_READ               , //  ※［＃判読不可、--］
		TT_INDENT                , // ［＃この行○字下げ］,［＃この行○字下げ、折り返して○字下げ］,［＃天から○字下げ］,［＃天から○字下げ、折り返して○字下げ］（これも可）
		TT_INDENT2               , // ［＃この行天付き、折り返して○字下げ］（行頭字下げが０の場合）
		TT_INDENT3               , // ［＃天から○字下げ］
		TT_INDENT_START          , // ［＃ここから--字下げ］
		TT_INDENT_START2         , // ［＃ここから改行天付き、折り返して○字下げ］～［＃ここで字下げ終わり］
		TT_INDENT_END            , // ［＃ここで字下げ終わり］
		TT_RINDENT               , // ［＃この行地付き］
		TT_RINDENT_START         , // ［＃ここから地付き］,［＃ここから地から○字上げ］
		TT_RINDENT_END           , //  ＃ここで地付き終わり］,［＃ここで字上げ終わり］
		TT_LIMIT_CHAR_START      , // ［＃ここから○字詰め］
		TT_LIMIT_CHAR_END        , // ［＃ここで字詰め終わり］
		TT_PICTURE_LAYOUT        , // 段［＃写真（--）入る］［＃挿絵（--）入る］
		TT_PICTURE_HALF_PAGE     , // 頁
		TT_PICTURE_FULL_PAGE     , // 見開き
		TT_LINE_BOX_START        , // ［＃ここから罫囲み］
		TT_LINE_BOX_END          , // ［＃ここで罫囲み終わり］
		TT_MOVING_BORDER         , // ［＃「」は罫囲み］
		TT_ACCENT                , // 〔---〕［＃--）付き］
		TT_OTHER_NOTE            , // ［＃「--」］
		TT_GUID_MARK             , //  訓点
		TT_OTHER                 , // ［＃--］
		TT_OVERLAP_CHAR          , //  。」, か”
		TT_KU1_CHAR              , // くの字点上
		TT_KU2_CHAR              , // くの字点上（濁点）
		TT_RUBY_SEP              , //  ｜
		TT_ROTATE_NUM            , // ［＃「...」は縦中横］
		TT_ROTATE_NUM_AUTO       , // 縦中横 文字数で判断
		TT_COMMENT_BEGIN         , // コメント開始
		TT_COMMENT               , // コメント
		TT_COMMENT_END           , // コメント終了
		TT_BOLD                  , // 太字
		TT_BOLD_START            , // 太字
		TT_BOLD_END              , // 太字
		TT_NEXT_LINE             , // 改行
		TT_NEXT_LAYOUT           , // 改段
		TT_NEXT_PAGE             , // 改頁
		TT_NEXT_PAPER            , // 改丁 (奇数ページ)
		TT_NEXT_PAPER_FIRST      , // 改見開き (偶数ページ) 
		TT_LINE_CHAR             , // ―
		TT_TITLE                 , // タイトル
		TT_AUTHOR                , // 著者
		TT_SUBTITLE1             , // 大見出し
		TT_SUBTITLE2             , // 中見出し
		TT_SUBTITLE3             , // 小見出し
		TT_SKIP_CHAR             , // スキップ(非表示文字)
		TT_SKIP_CHAR_AUTO        , // スキップ(非表示文字)
		TT_NOMBRE1               , // ノンブル1
		TT_NOMBRE2               , // ノンブル2
		TT_RUNNINGHEADS          , // 柱
		TT_ENDOFCONTENTS         , // 本文終了
		TT_LINK                  , // Link
		TT_TEXT_SIZE             , // ［＃「...」はn段階大きな文字］［＃「...」はn段階小さな文字］
		TT_TEXT_SIZE_L           , // ［＃「...」はn段階大きな文字］
		TT_TEXT_SIZE_S           , // ［＃「...」はn段階小さな文字］
		TT_CENTER                , // ［＃ページの左右中央］ 
		TT_ID                    , // ID ページ内リンク用
		TT_FILE                  , // NextFile用
		TT_TXTMIRU               , // TXTMIRU用タグ
		TT_MaxNum                , // TextTypeの件数
	};
	//
	struct CharPoint {
		int lcol; // 全頁を通しての画面右からの行数 ┏━━┯━━┓┏━━┰──┐┏━━━━━┓┌──┰━━┓
		int x   ; // 画面上の座標(大枠) : 左        ┃    │    ┃┃    ┃    │┃          ┃│    ┃    ┃
		int y   ; //                    : 上        ┃    │    ┃┃    ┃    │┃          ┃│    ┃    ┃
		int h   ; //                    : 高さ      ┠──┼──┨┠──╂──┤┝━━━━━┥├──┼━━┥
		int w   ; //                    : 幅        ┃    │    ┃┃    ┃    ││          ││    │    │
		int ch_h; // 文字の高さ                     ┃    │    ┃┃    ┃    ││          ││    │    │
		int ch_w; //       幅                       ┗━━┷━━┛┗━━┸──┘└─────┘└──┴──┘
		int zx  ; // 描画開始位置 : offset位置
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
		int iRIndent; // 地付き (行の途中で地付きになった場合に必要
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
	typedef std::vector<TextInfo> TextInfoList;
	typedef std::map<TextPoint, CharPoint > CharPointMap ;
	typedef std::map<TextPoint, TextStyle > TextStyleMap ;
	typedef std::map<TextPoint, TextOffset> TextOffsetMap;
	//
	struct LineInfo {
		int iStartCol;
		int iEndCol;
		int iFirstIndent ; // 天付き
		int iSecondIndent; // 天付き(２行目以降)
		int iRIndent     ; // 地付き
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
