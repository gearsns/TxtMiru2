#include "AozoraData.h"

// 以下、MatchPairStrDataの先頭文字の配列
static const TCHAR l_MatchPairFistStrData[] = {_T('※'), _T('《'), _T('」'), _T('［')};
static struct MatchPairStrData
{
	TxtMiru::TextType text_type;
	LPCTSTR const first ;
	LPCTSTR const second;
} MPSD[] = { // GetPairTTList() :  l_PairTTList[PAIRTTLISTLEN]
	{TxtMiru::TT_RUBY                  , _T("《"                                ), _T("》"                  )},
	{TxtMiru::TT_NO_READ               , _T("※［＃判読不可、"                  ), _T("］"                  )},
	{TxtMiru::TT_ERROR                 , _T(" 」は底本では「"                   ), _T("」］"                )},
	{TxtMiru::TT_WHITE_DOT             , _T("［＃「"                            ), _T("」に白ゴマ傍点］"    )},
	{TxtMiru::TT_ROUND_DOT             , _T("［＃「"                            ), _T("」に丸傍点］"        )},
	{TxtMiru::TT_WHITE_ROUND_DOT       , _T("［＃「"                            ), _T("」に白丸傍点］"      )},
	{TxtMiru::TT_BLACK_TRIANGLE_DOT    , _T("［＃「"                            ), _T("」に黒三角傍点］"    )},
	{TxtMiru::TT_WHITE_TRIANGLE_DOT    , _T("［＃「"                            ), _T("」に白三角傍点］"    )},
	{TxtMiru::TT_DOUBLE_ROUND_DOT      , _T("［＃「"                            ), _T("」に二重丸傍点］"    )},
	{TxtMiru::TT_BULLS_EYE_DOT         , _T("［＃「"                            ), _T("」に蛇の目傍点］"    )},
	{TxtMiru::TT_SALTIRE_DOT           , _T("［＃「"                            ), _T("」にばつ傍点］"      )},
	{TxtMiru::TT_DOT                   , _T("［＃「"                            ), _T("」に傍点］"          )},
	{TxtMiru::TT_LINE                  , _T("［＃「"                            ), _T("」に傍線］"          )},
	{TxtMiru::TT_WAVE_LINE             , _T("［＃「"                            ), _T("」に波線］"          )},
	{TxtMiru::TT_WAVE_LINE             , _T("［＃「"                            ), _T("」に波傍線］"        )},
	{TxtMiru::TT_DEL_LINE              , _T("［＃「"                            ), _T("」に取消線］"        )},
	{TxtMiru::TT_UNDER_LINE            , _T("［＃「"                            ), _T("」に左傍線］"        )},
	{TxtMiru::TT_UNDER_LINE            , _T("［＃「"                            ), _T("」の左に傍線］"      )},
	{TxtMiru::TT_UNDER_LINE            , _T("［＃「"                            ), _T("」にアンダーライン］")},
	{TxtMiru::TT_SHORT_DASHED_LINE     , _T("［＃「"                            ), _T("」に破線］"          )},
	{TxtMiru::TT_DOT_LINE              , _T("［＃「"                            ), _T("」に鎖線］"          )},
	{TxtMiru::TT_DOT_LINE              , _T("［＃「"                            ), _T("」に点線］"          )},
	{TxtMiru::TT_DOUBLE_LINE           , _T("［＃「"                            ), _T("」に二重傍線］"      )},
	{TxtMiru::TT_WHITE_DOT_L           , _T("［＃「"                            ), _T("」の左に白ゴマ傍点］")},
	{TxtMiru::TT_ROUND_DOT_L           , _T("［＃「"                            ), _T("」の左に丸傍点］"    )},
	{TxtMiru::TT_WHITE_ROUND_DOT_L     , _T("［＃「"                            ), _T("」の左に白丸傍点］"  )},
	{TxtMiru::TT_BLACK_TRIANGLE_DOT_L  , _T("［＃「"                            ), _T("」の左に黒三角傍点］")},
	{TxtMiru::TT_WHITE_TRIANGLE_DOT_L  , _T("［＃「"                            ), _T("」の左に白三角傍点］")},
	{TxtMiru::TT_DOUBLE_ROUND_DOT_L    , _T("［＃「"                            ), _T("」の左に二重丸傍点］")},
	{TxtMiru::TT_BULLS_EYE_DOT_L       , _T("［＃「"                            ), _T("」の左に蛇の目傍点］")},
	{TxtMiru::TT_SALTIRE_DOT_L         , _T("［＃「"                            ), _T("」の左にばつ傍点］"  )},
	{TxtMiru::TT_DOT_L                 , _T("［＃「"                            ), _T("」の左に傍点］"      )},
	{TxtMiru::TT_LINE_L                , _T("［＃「"                            ), _T("」の左に傍線］"      )},
	{TxtMiru::TT_WAVE_LINE_L           , _T("［＃「"                            ), _T("」の左に波線］"      )},
	{TxtMiru::TT_WAVE_LINE_L           , _T("［＃「"                            ), _T("」の左に波傍線］"    )},
	{TxtMiru::TT_SHORT_DASHED_LINE_L   , _T("［＃「"                            ), _T("」の左に破線］"      )},
	{TxtMiru::TT_DOT_LINE_L            , _T("［＃「"                            ), _T("」の左に鎖線］"      )},
	{TxtMiru::TT_DOT_LINE_L            , _T("［＃「"                            ), _T("」の左に点線］"      )},
	{TxtMiru::TT_DOUBLE_LINE_L         , _T("［＃「"                            ), _T("」の左に二重傍線］"  )},
	{TxtMiru::TT_CAPTION               , _T("［＃「"                            ), _T("」はキャプション］"  )},
	{TxtMiru::TT_CENTER                , _T("［＃ページの左右中央"              ), _T("］"                  )},
	{TxtMiru::TT_WHITE_DOT             , _T("［＃白ゴマ傍点"                    ), _T("］"                  )},
	{TxtMiru::TT_ROUND_DOT             , _T("［＃丸傍点"                        ), _T("］"                  )},
	{TxtMiru::TT_WHITE_ROUND_DOT       , _T("［＃白丸傍点"                      ), _T("］"                  )},
	{TxtMiru::TT_BLACK_TRIANGLE_DOT    , _T("［＃黒三角傍点"                    ), _T("］"                  )},
	{TxtMiru::TT_WHITE_TRIANGLE_DOT    , _T("［＃白三角傍点"                    ), _T("］"                  )},
	{TxtMiru::TT_DOUBLE_ROUND_DOT      , _T("［＃二重丸傍点"                    ), _T("］"                  )},
	{TxtMiru::TT_BULLS_EYE_DOT         , _T("［＃蛇の目傍点"                    ), _T("］"                  )},
	{TxtMiru::TT_SALTIRE_DOT           , _T("［＃ばつ傍点"                      ), _T("］"                  )},
	{TxtMiru::TT_DOT                   , _T("［＃傍点"                          ), _T("］"                  )},
	{TxtMiru::TT_LINE                  , _T("［＃傍線"                          ), _T("］"                  )},
	{TxtMiru::TT_WAVE_LINE             , _T("［＃波線"                          ), _T("］"                  )},
	{TxtMiru::TT_WAVE_LINE             , _T("［＃波傍線"                        ), _T("］"                  )},
	{TxtMiru::TT_DEL_LINE              , _T("［＃取消線"                        ), _T("］"                  )},
	{TxtMiru::TT_UNDER_LINE            , _T("［＃左に傍線"                      ), _T("］"                  )},
	{TxtMiru::TT_UNDER_LINE            , _T("［＃左傍線"                        ), _T("］"                  )},
	{TxtMiru::TT_UNDER_LINE            , _T("［＃アンダーライン"                ), _T("］"                  )},
	{TxtMiru::TT_SHORT_DASHED_LINE     , _T("［＃破線"                          ), _T("］"                  )},
	{TxtMiru::TT_DOT_LINE              , _T("［＃鎖線"                          ), _T("］"                  )},
	{TxtMiru::TT_DOT_LINE              , _T("［＃点線"                          ), _T("］"                  )},
	{TxtMiru::TT_DOUBLE_LINE           , _T("［＃二重傍線"                      ), _T("］"                  )},
	{TxtMiru::TT_WHITE_DOT_L           , _T("［＃左に白ゴマ傍点"                ), _T("］"                  )},
	{TxtMiru::TT_ROUND_DOT_L           , _T("［＃左に丸傍点"                    ), _T("］"                  )},
	{TxtMiru::TT_WHITE_ROUND_DOT_L     , _T("［＃左に白丸傍点"                  ), _T("］"                  )},
	{TxtMiru::TT_BLACK_TRIANGLE_DOT_L  , _T("［＃左に黒三角傍点"                ), _T("］"                  )},
	{TxtMiru::TT_WHITE_TRIANGLE_DOT_L  , _T("［＃左に白三角傍点"                ), _T("］"                  )},
	{TxtMiru::TT_DOUBLE_ROUND_DOT_L    , _T("［＃左に二重丸傍点"                ), _T("］"                  )},
	{TxtMiru::TT_BULLS_EYE_DOT_L       , _T("［＃左に蛇の目傍点"                ), _T("］"                  )},
	{TxtMiru::TT_SALTIRE_DOT           , _T("［＃左にばつ傍点"                  ), _T("］"                  )},
	{TxtMiru::TT_DOT_L                 , _T("［＃左に傍点"                      ), _T("］"                  )},
	{TxtMiru::TT_LINE_L                , _T("［＃左に傍線"                      ), _T("］"                  )},
	{TxtMiru::TT_WAVE_LINE_L           , _T("［＃左に波線"                      ), _T("］"                  )},
	{TxtMiru::TT_WAVE_LINE_L           , _T("［＃左に波傍線"                    ), _T("］"                  )},
	{TxtMiru::TT_SHORT_DASHED_LINE_L   , _T("［＃左に破線"                      ), _T("］"                  )},
	{TxtMiru::TT_DOT_LINE_L            , _T("［＃左に鎖線"                      ), _T("］"                  )},
	{TxtMiru::TT_DOT_LINE_L            , _T("［＃左に点線"                      ), _T("］"                  )},
	{TxtMiru::TT_DOUBLE_LINE_L         , _T("［＃左に二重傍線"                  ), _T("］"                  )},
	{TxtMiru::TT_CAPTION               , _T("［＃キャプション"                  ), _T("］"                  )},
	{TxtMiru::TT_CAPTION               , _T("［＃ここからキャプション"          ), _T("］"                  )},
	{TxtMiru::TT_TXTMIRU               , _T("［＃TxtMiru2:"                     ), _T("］"                  )},
	{TxtMiru::TT_NOTE                  , _T("［＃「"                            ), _T("」の注記］"          )},
	{TxtMiru::TT_SMALL_NOTE            , _T("［＃（"                            ), _T("）］"                )},
	{TxtMiru::TT_BOLD                  , _T("［＃「"                            ), _T("」は太字］"          )},
	{TxtMiru::TT_BOLD                  , _T("［＃「"                            ), _T("」はゴシック体］"    )},
	{TxtMiru::TT_SUP_NOTE              , _T("［＃「"                            ), _T("」は上付き小文字］"  )},
	{TxtMiru::TT_SUP_NOTE              , _T("［＃"                              ), _T("は上付き小文字］"    )},
	{TxtMiru::TT_SUB_NOTE              , _T("［＃「"                            ), _T("」は下付き小文字］"  )},
	{TxtMiru::TT_SUB_NOTE              , _T("［＃"                              ), _T("は下付き小文字］"    )},
	{TxtMiru::TT_MOVING_BORDER         , _T("［＃「"                            ), _T("」は罫囲み］"        )},
	{TxtMiru::TT_LINE_BOX_START        , _T("［＃ここから"                      ), _T("罫囲み］"            )},
	{TxtMiru::TT_LINE_BOX_START        , _T("［＃罫囲み"                        ), _T("］"                  )},
	{TxtMiru::TT_UNKOWN_ERROR          , _T("［＃「"                            ), _T("」はママ］"          )},
	{TxtMiru::TT_UNKOWN_ERROR          , _T("［＃「"                            ), _T("」に「ママ」の注記］")},
	{TxtMiru::TT_INDENT2               , _T("［＃この行天付き、折り返して"      ), _T("字下げ］"            )},
	{TxtMiru::TT_INDENT3               , _T("［＃天から"                        ), _T("字下げ］"            )},
	{TxtMiru::TT_INDENT                , _T("［＃この行"                        ), _T("字下げ］"            )},
	{TxtMiru::TT_INDENT_START2         , _T("［＃ここから改行天付き、折り返して"), _T("字下げ］"            )},
	{TxtMiru::TT_INDENT_START          , _T("［＃ここから"                      ), _T("字下げ］"            )},
	{TxtMiru::TT_INDENT                , _T("［＃"                              ), _T("字下げ］"            )}, // 字下げ 最後に
	{TxtMiru::TT_INDENT_END            , _T("［＃ここで"                        ), _T("字下げ終わり］"      )},
	{TxtMiru::TT_RINDENT               , _T("［＃地付き"                        ), _T("］"                  )},
	{TxtMiru::TT_RINDENT               , _T("［＃この行"                        ), _T("地付き］"            )},
	{TxtMiru::TT_RINDENT               , _T("［＃地から"                        ), _T("字上げ］"            )},
	{TxtMiru::TT_RINDENT_START         , _T("［＃ここから"                      ), _T("地付き］"            )},
	{TxtMiru::TT_RINDENT_START         , _T("［＃ここから地から"                ), _T("字上げ］"            )},
	{TxtMiru::TT_RINDENT_END           , _T("［＃ここで"                        ), _T("地付き終わり］"      )},
	{TxtMiru::TT_RINDENT_END           , _T("［＃ここで"                        ), _T("字上げ終わり］"      )},
	{TxtMiru::TT_RINDENT               , _T("［＃"                              ), _T("地付き］"            )}, // 地付き 最後に
	{TxtMiru::TT_BOLD_START            , _T("［＃太字"                          ), _T("］"                  )},
	{TxtMiru::TT_BOLD_START            , _T("［＃ここから"                      ), _T("太字］"              )},
	{TxtMiru::TT_BOLD_END              , _T("［＃ここまで"                      ), _T("太字］"              )},
	{TxtMiru::TT_BOLD_END              , _T("［＃ここで太字"                    ), _T("終わり］"            )},
	{TxtMiru::TT_BOLD_END              , _T("［＃太字"                          ), _T("終わり］"            )},
	{TxtMiru::TT_LIMIT_CHAR_START      , _T("［＃ここから"                      ), _T("字詰め］"            )},
	{TxtMiru::TT_LIMIT_CHAR_END        , _T("［＃ここで字詰め"                  ), _T("終わり］"            )},
	{TxtMiru::TT_ENDOFCONTENTS         , _T("［＃本文"                          ), _T("終わり］"            )},
	{TxtMiru::TT_TEXT_SIZE_L           , _T("［＃ここから"                      ), _T("段階大きな文字］"    )},
	{TxtMiru::TT_TEXT_SIZE_S           , _T("［＃ここから"                      ), _T("段階小さな文字］"    )},
	{TxtMiru::TT_TEXT_SIZE_L           , _T("［＃大きな文字"                    ), _T("］"                  )},
	{TxtMiru::TT_TEXT_SIZE_S           , _T("［＃小さな文字"                    ), _T("］"                  )},
	{TxtMiru::TT_PICTURE_LAYOUT        , _T("［＃"                              ), _T("）入る］"            )},
	{TxtMiru::TT_OTHER_NOTE            , _T("［＃「"                            ), _T("」］"                )},
	{TxtMiru::TT_ROTATE_NUM            , _T("［＃「"                            ), _T("」は縦中横］"        )},
	{TxtMiru::TT_HORZ                  , _T("［＃「"                            ), _T("」は横組み］"        )}, // ［＃「...」は横組み］
	{TxtMiru::TT_HORZ_START            , _T("［＃ここから"                      ), _T("横組み］"            )}, // ［＃ここから横組み］
	{TxtMiru::TT_HORZ_START            , _T("［＃横組み"                        ), _T("］"                  )}, // ［＃横組み］
	{TxtMiru::TT_SUBTITLE1             , _T("［＃「"                            ), _T("」は大見出し］"      )},
	{TxtMiru::TT_SUBTITLE2             , _T("［＃「"                            ), _T("」は中見出し］"      )},
	{TxtMiru::TT_SUBTITLE3             , _T("［＃「"                            ), _T("」は小見出し］"      )},
	{TxtMiru::TT_SUBTITLE1             , _T("［＃大見出し"                      ), _T("］"                  )},
	{TxtMiru::TT_SUBTITLE2             , _T("［＃中見出し"                      ), _T("］"                  )},
	{TxtMiru::TT_SUBTITLE3             , _T("［＃小見出し"                      ), _T("］"                  )},
	{TxtMiru::TT_SUBTITLE1             , _T("［＃ここから"                      ), _T("大見出し］"          )}, // ここから大見出し
	{TxtMiru::TT_SUBTITLE2             , _T("［＃ここから"                      ), _T("中見出し］"          )}, // ここから中見出し
	{TxtMiru::TT_SUBTITLE3             , _T("［＃ここから"                      ), _T("小見出し］"          )}, // ここから小見出し
	{TxtMiru::TT_SMALL_NOTE_R          , _T("［＃「"                            ), _T("」は行右小書き］"    )},
	{TxtMiru::TT_NOTE_START            , _T("［＃注記付き"                      ), _T("］"                  )},
	{TxtMiru::TT_NOTE_END              , _T("［＃「"                            ), _T("」の注記付き終わり］")},
	{TxtMiru::TT_NOTE_START_L          , _T("［＃左に注記付き"                  ), _T("］"                  )},
	{TxtMiru::TT_NOTE_END_L            , _T("［＃左に「"                        ), _T("」の注記付き終わり］")},
	{TxtMiru::TT_RUBY_L                , _T("［＃「"                            ), _T("」のルビ］"          )},
	{TxtMiru::TT_RUBY_L_START          , _T("［＃左にルビ付き"                  ), _T("］"                  )},
	{TxtMiru::TT_RUBY_L_END            , _T("［＃左に「"                        ), _T("」のルビ付き終わり］")},
	{TxtMiru::TT_NEXT_LINE             , _T("［＃改行"                          ), _T("］"                  )},
	{TxtMiru::TT_NEXT_LAYOUT           , _T("［＃改段"                          ), _T("］"                  )},
	{TxtMiru::TT_NEXT_PAGE             , _T("［＃改頁"                          ), _T("］"                  )},
	{TxtMiru::TT_NEXT_PAGE             , _T("［＃改ページ"                      ), _T("］"                  )},
	{TxtMiru::TT_NEXT_PAPER            , _T("［＃改丁"                          ), _T("］"                  )},
	{TxtMiru::TT_NEXT_PAPER_FIRST      , _T("［＃改見開き"                      ), _T("］"                  )},
	{TxtMiru::TT_RANGE_END             , _T("［＃"                              ), _T("終わり］"            )},
	{TxtMiru::TT_TEXT_SIZE_L           , _T("［＃「"                            ), _T("段階大きな文字］"    )},
	{TxtMiru::TT_TEXT_SIZE_S           , _T("［＃「"                            ), _T("段階小さな文字］"    )},
	{TxtMiru::TT_TEXT_SIZE_L           , _T("［＃"                              ), _T("大きな文字］"        )},
	{TxtMiru::TT_TEXT_SIZE_S           , _T("［＃"                              ), _T("小さな文字］"        )},
	{TxtMiru::TT_PICTURE_LAYOUT        , _T("［＃"                              ), _T("）］"                )},
	{TxtMiru::TT_PICTURE_LAYOUT        , _T("［＃"                              ), _T("入る］"              )},
	{TxtMiru::TT_OTHER                 , _T("※［＃"                            ), _T("］"                  )},
	{TxtMiru::TT_OTHER                 , _T("［＃"                              ), _T("］"                  )},
};
static struct MatchStrMapData
{
	TxtMiru::TextType text_type;
	LPCTSTR const str;
} MSMD[] = {
	{TxtMiru::TT_WHITE_DOT           , _T("白ゴマ傍点"        )},
	{TxtMiru::TT_ROUND_DOT           , _T("丸傍点"            )},
	{TxtMiru::TT_WHITE_ROUND_DOT     , _T("白丸傍点"          )},
	{TxtMiru::TT_BLACK_TRIANGLE_DOT  , _T("黒三角傍点"        )},
	{TxtMiru::TT_WHITE_TRIANGLE_DOT  , _T("白三角傍点"        )},
	{TxtMiru::TT_DOUBLE_ROUND_DOT    , _T("二重丸傍点"        )},
	{TxtMiru::TT_BULLS_EYE_DOT       , _T("蛇の目傍点"        )},
	{TxtMiru::TT_SALTIRE_DOT         , _T("ばつ傍点"          )},
	{TxtMiru::TT_DOT                 , _T("傍点"              )},
	{TxtMiru::TT_LINE                , _T("傍線"              )},
	{TxtMiru::TT_WAVE_LINE           , _T("波線"              )},
	{TxtMiru::TT_DEL_LINE            , _T("取消線"            )},
	{TxtMiru::TT_UNDER_LINE          , _T("左傍線"            )},
	{TxtMiru::TT_SHORT_DASHED_LINE   , _T("破線"              )},
	{TxtMiru::TT_DOT_LINE            , _T("点線"              )},
	{TxtMiru::TT_DOUBLE_LINE         , _T("二重傍線"          )},
	{TxtMiru::TT_WHITE_DOT_L         , _T("左白ゴマ傍点"      )},
	{TxtMiru::TT_ROUND_DOT_L         , _T("左丸傍点"          )},
	{TxtMiru::TT_WHITE_ROUND_DOT_L   , _T("左白丸傍点"        )},
	{TxtMiru::TT_BLACK_TRIANGLE_DOT_L, _T("左黒三角傍点"      )},
	{TxtMiru::TT_WHITE_TRIANGLE_DOT_L, _T("左白三角傍点"      )},
	{TxtMiru::TT_DOUBLE_ROUND_DOT_L  , _T("左二重丸傍点"      )},
	{TxtMiru::TT_BULLS_EYE_DOT_L     , _T("左蛇の目傍点"      )},
	{TxtMiru::TT_SALTIRE_DOT_L       , _T("左ばつ傍点"        )},
	{TxtMiru::TT_DOT_L               , _T("左傍点"            )},
	{TxtMiru::TT_LINE_L              , _T("左傍線"            )},
	{TxtMiru::TT_WAVE_LINE_L         , _T("左波線"            )},
	{TxtMiru::TT_SHORT_DASHED_LINE_L , _T("左破線"            )},
	{TxtMiru::TT_DOT_LINE_L          , _T("左点線"            )},
	{TxtMiru::TT_DOUBLE_LINE_L       , _T("左二重傍線"        )},
	{TxtMiru::TT_INDENT_START        , _T("折り返して"        )},
	{TxtMiru::TT_NOTE                , _T("」に「"            )},
	{TxtMiru::TT_RUBY_L              , _T("」の左に「"        )},
	{TxtMiru::TT_NOTE_L              , _T("」の左に「"        )},
	{TxtMiru::TT_ERROR               , _T("」は底本では「"    )},
	{TxtMiru::TT_ERROR               , _T("」は底本では「"    )},
	{TxtMiru::TT_TEXT_SIZE           , _T("」は"              )},
	{TxtMiru::TT_UNKOWN_ERROR        , _T("ママ"              )},
	{TxtMiru::TT_NO_READ             , _T("※"                )},
	{TxtMiru::TT_LINE_CHAR           , _T("―"                )},
	{TxtMiru::TT_OTHER               , _T("、"                )},
	{TxtMiru::TT_MOVING_BORDER       , _T("罫囲み"            )},
};

static struct MatchStrData
{
	TxtMiru::TextType text_type;
	LPCTSTR const str;
} MSD[] = { // GetTTList() : l_TTList [TTLISTLEN]
	{TxtMiru::TT_RUBY_SEP    , _T("｜"    )},
	{TxtMiru::TT_KU1_CHAR    , _T("／＼"  )},
	{TxtMiru::TT_KU2_CHAR    , _T("／″＼")},
	{TxtMiru::TT_OVERLAP_CHAR, _T("゜"    )},
	{TxtMiru::TT_OVERLAP_CHAR, _T("゛"    )},
	{TxtMiru::TT_LINE_CHAR   , _T("―"    )},
};

#define PAIRTTLISTLEN (sizeof(MPSD)/sizeof(MatchPairStrData))
#define TTLEN         (sizeof(MSMD)/sizeof(MatchStrData    ))
#define TTLISTLEN     (sizeof(MSD )/sizeof(MatchStrData    ))

static MatchPairStr l_PairTTList[sizeof(MPSD)/sizeof(MatchPairStrData)];
static MatchStr     l_TTList    [sizeof(MSD )/sizeof(MatchStrData    )];
static MatchStr     l_TT        [TxtMiru::TT_MaxNum                   ];


void MatchStr::Set(LPCTSTR p, TxtMiru::TextType t)
{
	str = p;
	str_len = CGrText::CharLen(p);
	str_size = _tcslen(p);
	text_type = t;
}

void MatchPairStr::Set(TxtMiru::TextType t, LPCTSTR f, LPCTSTR s)
{
	first.Set(f, t);
	second.Set(s, t);
}

CGrAozoraData::CGrAozoraData()
{
	auto *pmps = l_PairTTList;
	for(const auto &item : MPSD){
		pmps->Set(item.text_type, item.first, item.second);
		++pmps;
	}
	for(const auto &item : MSMD){
		l_TT[item.text_type].Set(item.str);
	}
	auto *pms = l_TTList;
	for(const auto &item : MSD){
		pms->Set(item.str, item.text_type);
		++pms;
	}
}

CGrAozoraData::~CGrAozoraData()
{
}

CGrAozoraData &CGrAozoraData::Data(){
	static CGrAozoraData data;
	return data;
}

const MatchPairStr* CGrAozoraData::GetPairTTList() const
{
	return l_PairTTList;
}
const MatchStr* CGrAozoraData::GetTTList() const
{
	return l_TTList;
}
const MatchStr* CGrAozoraData::GetTT() const
{
	return l_TT;
}

int CGrAozoraData::GetPairTTListLength() const { return PAIRTTLISTLEN; }
int CGrAozoraData::GetTTLength() const { return TTLEN; }
int CGrAozoraData::GetTTListLength() const { return TTLISTLEN; }

bool CGrAozoraData::IsPairTTListFistChar(LPCTSTR lpSrc) const
{
	for(auto ch : l_MatchPairFistStrData){
		if(ch == *lpSrc){
			return true;
		}
	}
	return false;
}

bool CGrAozoraData::IsTTListFistChar(LPCTSTR lpSrc) const
{
	for(const auto ch : {_T('｜'),_T('／'),_T('゜'),_T('゛'),_T('―')}){
		if(ch == *lpSrc){
			return true;
		}
	}
	return false;
}
