#include "AozoraData.h"

// 以下、MatchPairStrDataの先頭文字の配列
static const TCHAR l_MatchPairFistStrData[] = {_T('※'), _T('《'), _T('」'), _T('［')};
static struct MatchPairStrData
{
	TxtMiru::TextType text_type;
	LPCTSTR const first ;
	LPCTSTR const second;
} MPSD[] = { // GetPairTTList() :  l_PairTTList[PAIRTTLISTLEN]
	{TxtMiru::TextType::RUBY                  , _T("《"                                ), _T("》"                  )},
	{TxtMiru::TextType::NO_READ               , _T("※［＃判読不可、"                  ), _T("］"                  )},
	{TxtMiru::TextType::ERROR_STR             , _T(" 」は底本では「"                   ), _T("」］"                )},
	{TxtMiru::TextType::WHITE_DOT             , _T("［＃「"                            ), _T("」に白ゴマ傍点］"    )},
	{TxtMiru::TextType::ROUND_DOT             , _T("［＃「"                            ), _T("」に丸傍点］"        )},
	{TxtMiru::TextType::WHITE_ROUND_DOT       , _T("［＃「"                            ), _T("」に白丸傍点］"      )},
	{TxtMiru::TextType::BLACK_TRIANGLE_DOT    , _T("［＃「"                            ), _T("」に黒三角傍点］"    )},
	{TxtMiru::TextType::WHITE_TRIANGLE_DOT    , _T("［＃「"                            ), _T("」に白三角傍点］"    )},
	{TxtMiru::TextType::DOUBLE_ROUND_DOT      , _T("［＃「"                            ), _T("」に二重丸傍点］"    )},
	{TxtMiru::TextType::BULLS_EYE_DOT         , _T("［＃「"                            ), _T("」に蛇の目傍点］"    )},
	{TxtMiru::TextType::SALTIRE_DOT           , _T("［＃「"                            ), _T("」にばつ傍点］"      )},
	{TxtMiru::TextType::DOT                   , _T("［＃「"                            ), _T("」に傍点］"          )},
	{TxtMiru::TextType::LINE                  , _T("［＃「"                            ), _T("」に傍線］"          )},
	{TxtMiru::TextType::WAVE_LINE             , _T("［＃「"                            ), _T("」に波線］"          )},
	{TxtMiru::TextType::WAVE_LINE             , _T("［＃「"                            ), _T("」に波傍線］"        )},
	{TxtMiru::TextType::DEL_LINE              , _T("［＃「"                            ), _T("」に取消線］"        )},
	{TxtMiru::TextType::UNDER_LINE            , _T("［＃「"                            ), _T("」に左傍線］"        )},
	{TxtMiru::TextType::UNDER_LINE            , _T("［＃「"                            ), _T("」の左に傍線］"      )},
	{TxtMiru::TextType::UNDER_LINE            , _T("［＃「"                            ), _T("」にアンダーライン］")},
	{TxtMiru::TextType::SHORT_DASHED_LINE     , _T("［＃「"                            ), _T("」に破線］"          )},
	{TxtMiru::TextType::DOT_LINE              , _T("［＃「"                            ), _T("」に鎖線］"          )},
	{TxtMiru::TextType::DOT_LINE              , _T("［＃「"                            ), _T("」に点線］"          )},
	{TxtMiru::TextType::DOUBLE_LINE           , _T("［＃「"                            ), _T("」に二重傍線］"      )},
	{TxtMiru::TextType::WHITE_DOT_L           , _T("［＃「"                            ), _T("」の左に白ゴマ傍点］")},
	{TxtMiru::TextType::ROUND_DOT_L           , _T("［＃「"                            ), _T("」の左に丸傍点］"    )},
	{TxtMiru::TextType::WHITE_ROUND_DOT_L     , _T("［＃「"                            ), _T("」の左に白丸傍点］"  )},
	{TxtMiru::TextType::BLACK_TRIANGLE_DOT_L  , _T("［＃「"                            ), _T("」の左に黒三角傍点］")},
	{TxtMiru::TextType::WHITE_TRIANGLE_DOT_L  , _T("［＃「"                            ), _T("」の左に白三角傍点］")},
	{TxtMiru::TextType::DOUBLE_ROUND_DOT_L    , _T("［＃「"                            ), _T("」の左に二重丸傍点］")},
	{TxtMiru::TextType::BULLS_EYE_DOT_L       , _T("［＃「"                            ), _T("」の左に蛇の目傍点］")},
	{TxtMiru::TextType::SALTIRE_DOT_L         , _T("［＃「"                            ), _T("」の左にばつ傍点］"  )},
	{TxtMiru::TextType::DOT_L                 , _T("［＃「"                            ), _T("」の左に傍点］"      )},
	{TxtMiru::TextType::LINE_L                , _T("［＃「"                            ), _T("」の左に傍線］"      )},
	{TxtMiru::TextType::WAVE_LINE_L           , _T("［＃「"                            ), _T("」の左に波線］"      )},
	{TxtMiru::TextType::WAVE_LINE_L           , _T("［＃「"                            ), _T("」の左に波傍線］"    )},
	{TxtMiru::TextType::SHORT_DASHED_LINE_L   , _T("［＃「"                            ), _T("」の左に破線］"      )},
	{TxtMiru::TextType::DOT_LINE_L            , _T("［＃「"                            ), _T("」の左に鎖線］"      )},
	{TxtMiru::TextType::DOT_LINE_L            , _T("［＃「"                            ), _T("」の左に点線］"      )},
	{TxtMiru::TextType::DOUBLE_LINE_L         , _T("［＃「"                            ), _T("」の左に二重傍線］"  )},
	{TxtMiru::TextType::CAPTION               , _T("［＃「"                            ), _T("」はキャプション］"  )},
	{TxtMiru::TextType::CENTER                , _T("［＃ページの左右中央"              ), _T("］"                  )},
	{TxtMiru::TextType::WHITE_DOT             , _T("［＃白ゴマ傍点"                    ), _T("］"                  )},
	{TxtMiru::TextType::ROUND_DOT             , _T("［＃丸傍点"                        ), _T("］"                  )},
	{TxtMiru::TextType::WHITE_ROUND_DOT       , _T("［＃白丸傍点"                      ), _T("］"                  )},
	{TxtMiru::TextType::BLACK_TRIANGLE_DOT    , _T("［＃黒三角傍点"                    ), _T("］"                  )},
	{TxtMiru::TextType::WHITE_TRIANGLE_DOT    , _T("［＃白三角傍点"                    ), _T("］"                  )},
	{TxtMiru::TextType::DOUBLE_ROUND_DOT      , _T("［＃二重丸傍点"                    ), _T("］"                  )},
	{TxtMiru::TextType::BULLS_EYE_DOT         , _T("［＃蛇の目傍点"                    ), _T("］"                  )},
	{TxtMiru::TextType::SALTIRE_DOT           , _T("［＃ばつ傍点"                      ), _T("］"                  )},
	{TxtMiru::TextType::DOT                   , _T("［＃傍点"                          ), _T("］"                  )},
	{TxtMiru::TextType::LINE                  , _T("［＃傍線"                          ), _T("］"                  )},
	{TxtMiru::TextType::WAVE_LINE             , _T("［＃波線"                          ), _T("］"                  )},
	{TxtMiru::TextType::WAVE_LINE             , _T("［＃波傍線"                        ), _T("］"                  )},
	{TxtMiru::TextType::DEL_LINE              , _T("［＃取消線"                        ), _T("］"                  )},
	{TxtMiru::TextType::UNDER_LINE            , _T("［＃左に傍線"                      ), _T("］"                  )},
	{TxtMiru::TextType::UNDER_LINE            , _T("［＃左傍線"                        ), _T("］"                  )},
	{TxtMiru::TextType::UNDER_LINE            , _T("［＃アンダーライン"                ), _T("］"                  )},
	{TxtMiru::TextType::SHORT_DASHED_LINE     , _T("［＃破線"                          ), _T("］"                  )},
	{TxtMiru::TextType::DOT_LINE              , _T("［＃鎖線"                          ), _T("］"                  )},
	{TxtMiru::TextType::DOT_LINE              , _T("［＃点線"                          ), _T("］"                  )},
	{TxtMiru::TextType::DOUBLE_LINE           , _T("［＃二重傍線"                      ), _T("］"                  )},
	{TxtMiru::TextType::WHITE_DOT_L           , _T("［＃左に白ゴマ傍点"                ), _T("］"                  )},
	{TxtMiru::TextType::ROUND_DOT_L           , _T("［＃左に丸傍点"                    ), _T("］"                  )},
	{TxtMiru::TextType::WHITE_ROUND_DOT_L     , _T("［＃左に白丸傍点"                  ), _T("］"                  )},
	{TxtMiru::TextType::BLACK_TRIANGLE_DOT_L  , _T("［＃左に黒三角傍点"                ), _T("］"                  )},
	{TxtMiru::TextType::WHITE_TRIANGLE_DOT_L  , _T("［＃左に白三角傍点"                ), _T("］"                  )},
	{TxtMiru::TextType::DOUBLE_ROUND_DOT_L    , _T("［＃左に二重丸傍点"                ), _T("］"                  )},
	{TxtMiru::TextType::BULLS_EYE_DOT_L       , _T("［＃左に蛇の目傍点"                ), _T("］"                  )},
	{TxtMiru::TextType::SALTIRE_DOT_L         , _T("［＃左にばつ傍点"                  ), _T("］"                  )},
	{TxtMiru::TextType::DOT_L                 , _T("［＃左に傍点"                      ), _T("］"                  )},
	{TxtMiru::TextType::LINE_L                , _T("［＃左に傍線"                      ), _T("］"                  )},
	{TxtMiru::TextType::WAVE_LINE_L           , _T("［＃左に波線"                      ), _T("］"                  )},
	{TxtMiru::TextType::WAVE_LINE_L           , _T("［＃左に波傍線"                    ), _T("］"                  )},
	{TxtMiru::TextType::SHORT_DASHED_LINE_L   , _T("［＃左に破線"                      ), _T("］"                  )},
	{TxtMiru::TextType::DOT_LINE_L            , _T("［＃左に鎖線"                      ), _T("］"                  )},
	{TxtMiru::TextType::DOT_LINE_L            , _T("［＃左に点線"                      ), _T("］"                  )},
	{TxtMiru::TextType::DOUBLE_LINE_L         , _T("［＃左に二重傍線"                  ), _T("］"                  )},
	{TxtMiru::TextType::CAPTION               , _T("［＃キャプション"                  ), _T("］"                  )},
	{TxtMiru::TextType::CAPTION               , _T("［＃ここからキャプション"          ), _T("］"                  )},
	{TxtMiru::TextType::TXTMIRU               , _T("［＃TxtMiru2:"                     ), _T("］"                  )},
	{TxtMiru::TextType::NOTE                  , _T("［＃「"                            ), _T("」の注記］"          )},
	{TxtMiru::TextType::SMALL_NOTE            , _T("［＃（"                            ), _T("）］"                )},
	{TxtMiru::TextType::BOLD                  , _T("［＃「"                            ), _T("」は太字］"          )},
	{TxtMiru::TextType::BOLD                  , _T("［＃「"                            ), _T("」はゴシック体］"    )},
	{TxtMiru::TextType::SUP_NOTE              , _T("［＃「"                            ), _T("」は上付き小文字］"  )},
	{TxtMiru::TextType::SUP_NOTE              , _T("［＃"                              ), _T("は上付き小文字］"    )},
	{TxtMiru::TextType::SUB_NOTE              , _T("［＃「"                            ), _T("」は下付き小文字］"  )},
	{TxtMiru::TextType::SUB_NOTE              , _T("［＃"                              ), _T("は下付き小文字］"    )},
	{TxtMiru::TextType::MOVING_BORDER         , _T("［＃「"                            ), _T("」は罫囲み］"        )},
	{TxtMiru::TextType::LINE_BOX_START        , _T("［＃ここから"                      ), _T("罫囲み］"            )},
	{TxtMiru::TextType::LINE_BOX_START        , _T("［＃罫囲み"                        ), _T("］"                  )},
	{TxtMiru::TextType::UNKOWN_ERROR          , _T("［＃「"                            ), _T("」はママ］"          )},
	{TxtMiru::TextType::UNKOWN_ERROR          , _T("［＃「"                            ), _T("」に「ママ」の注記］")},
	{TxtMiru::TextType::INDENT2               , _T("［＃この行天付き、折り返して"      ), _T("字下げ］"            )},
	{TxtMiru::TextType::INDENT3               , _T("［＃天から"                        ), _T("字下げ］"            )},
	{TxtMiru::TextType::INDENT                , _T("［＃この行"                        ), _T("字下げ］"            )},
	{TxtMiru::TextType::INDENT_START2         , _T("［＃ここから改行天付き、折り返して"), _T("字下げ］"            )},
	{TxtMiru::TextType::INDENT_START          , _T("［＃ここから"                      ), _T("字下げ］"            )},
	{TxtMiru::TextType::INDENT                , _T("［＃"                              ), _T("字下げ］"            )}, // 字下げ 最後に
	{TxtMiru::TextType::INDENT_END            , _T("［＃ここで"                        ), _T("字下げ終わり］"      )},
	{TxtMiru::TextType::RINDENT               , _T("［＃地付き"                        ), _T("］"                  )},
	{TxtMiru::TextType::RINDENT               , _T("［＃この行"                        ), _T("地付き］"            )},
	{TxtMiru::TextType::RINDENT               , _T("［＃地から"                        ), _T("字上げ］"            )},
	{TxtMiru::TextType::RINDENT_START         , _T("［＃ここから"                      ), _T("地付き］"            )},
	{TxtMiru::TextType::RINDENT_START         , _T("［＃ここから地から"                ), _T("字上げ］"            )},
	{TxtMiru::TextType::RINDENT_END           , _T("［＃ここで"                        ), _T("地付き終わり］"      )},
	{TxtMiru::TextType::RINDENT_END           , _T("［＃ここで"                        ), _T("字上げ終わり］"      )},
	{TxtMiru::TextType::RINDENT               , _T("［＃"                              ), _T("地付き］"            )}, // 地付き 最後に
	{TxtMiru::TextType::BOLD_START            , _T("［＃太字"                          ), _T("］"                  )},
	{TxtMiru::TextType::BOLD_START            , _T("［＃ここから"                      ), _T("太字］"              )},
	{TxtMiru::TextType::BOLD_END              , _T("［＃ここまで"                      ), _T("太字］"              )},
	{TxtMiru::TextType::BOLD_END              , _T("［＃ここで太字"                    ), _T("終わり］"            )},
	{TxtMiru::TextType::BOLD_END              , _T("［＃太字"                          ), _T("終わり］"            )},
	{TxtMiru::TextType::LIMIT_CHAR_START      , _T("［＃ここから"                      ), _T("字詰め］"            )},
	{TxtMiru::TextType::LIMIT_CHAR_END        , _T("［＃ここで字詰め"                  ), _T("終わり］"            )},
	{TxtMiru::TextType::ENDOFCONTENTS         , _T("［＃本文"                          ), _T("終わり］"            )},
	{TxtMiru::TextType::TEXT_SIZE_L           , _T("［＃ここから"                      ), _T("段階大きな文字］"    )},
	{TxtMiru::TextType::TEXT_SIZE_S           , _T("［＃ここから"                      ), _T("段階小さな文字］"    )},
	{TxtMiru::TextType::TEXT_SIZE_L           , _T("［＃大きな文字"                    ), _T("］"                  )},
	{TxtMiru::TextType::TEXT_SIZE_S           , _T("［＃小さな文字"                    ), _T("］"                  )},
	{TxtMiru::TextType::PICTURE_LAYOUT        , _T("［＃"                              ), _T("）入る］"            )},
	{TxtMiru::TextType::OTHER_NOTE            , _T("［＃「"                            ), _T("」］"                )},
	{TxtMiru::TextType::ROTATE_NUM            , _T("［＃「"                            ), _T("」は縦中横］"        )},
	{TxtMiru::TextType::HORZ                  , _T("［＃「"                            ), _T("」は横組み］"        )}, // ［＃「...」は横組み］
	{TxtMiru::TextType::HORZ_START            , _T("［＃ここから"                      ), _T("横組み］"            )}, // ［＃ここから横組み］
	{TxtMiru::TextType::HORZ_START            , _T("［＃横組み"                        ), _T("］"                  )}, // ［＃横組み］
	{TxtMiru::TextType::SUBTITLE1             , _T("［＃「"                            ), _T("」は大見出し］"      )},
	{TxtMiru::TextType::SUBTITLE2             , _T("［＃「"                            ), _T("」は中見出し］"      )},
	{TxtMiru::TextType::SUBTITLE3             , _T("［＃「"                            ), _T("」は小見出し］"      )},
	{TxtMiru::TextType::SUBTITLE1             , _T("［＃大見出し"                      ), _T("］"                  )},
	{TxtMiru::TextType::SUBTITLE2             , _T("［＃中見出し"                      ), _T("］"                  )},
	{TxtMiru::TextType::SUBTITLE3             , _T("［＃小見出し"                      ), _T("］"                  )},
	{TxtMiru::TextType::SUBTITLE1             , _T("［＃ここから"                      ), _T("大見出し］"          )}, // ここから大見出し
	{TxtMiru::TextType::SUBTITLE2             , _T("［＃ここから"                      ), _T("中見出し］"          )}, // ここから中見出し
	{TxtMiru::TextType::SUBTITLE3             , _T("［＃ここから"                      ), _T("小見出し］"          )}, // ここから小見出し
	{TxtMiru::TextType::SMALL_NOTE_R          , _T("［＃「"                            ), _T("」は行右小書き］"    )},
	{TxtMiru::TextType::NOTE_START            , _T("［＃注記付き"                      ), _T("］"                  )},
	{TxtMiru::TextType::NOTE_END              , _T("［＃「"                            ), _T("」の注記付き終わり］")},
	{TxtMiru::TextType::NOTE_START_L          , _T("［＃左に注記付き"                  ), _T("］"                  )},
	{TxtMiru::TextType::NOTE_END_L            , _T("［＃左に「"                        ), _T("」の注記付き終わり］")},
	{TxtMiru::TextType::RUBY_L                , _T("［＃「"                            ), _T("」のルビ］"          )},
	{TxtMiru::TextType::RUBY_L_START          , _T("［＃左にルビ付き"                  ), _T("］"                  )},
	{TxtMiru::TextType::RUBY_L_END            , _T("［＃左に「"                        ), _T("」のルビ付き終わり］")},
	{TxtMiru::TextType::NEXT_LINE             , _T("［＃改行"                          ), _T("］"                  )},
	{TxtMiru::TextType::NEXT_LAYOUT           , _T("［＃改段"                          ), _T("］"                  )},
	{TxtMiru::TextType::NEXT_PAGE             , _T("［＃改頁"                          ), _T("］"                  )},
	{TxtMiru::TextType::NEXT_PAGE             , _T("［＃改ページ"                      ), _T("］"                  )},
	{TxtMiru::TextType::NEXT_PAPER            , _T("［＃改丁"                          ), _T("］"                  )},
	{TxtMiru::TextType::NEXT_PAPER_FIRST      , _T("［＃改見開き"                      ), _T("］"                  )},
	{TxtMiru::TextType::RANGE_END             , _T("［＃"                              ), _T("終わり］"            )},
	{TxtMiru::TextType::TEXT_SIZE_L           , _T("［＃「"                            ), _T("段階大きな文字］"    )},
	{TxtMiru::TextType::TEXT_SIZE_S           , _T("［＃「"                            ), _T("段階小さな文字］"    )},
	{TxtMiru::TextType::TEXT_SIZE_L           , _T("［＃"                              ), _T("大きな文字］"        )},
	{TxtMiru::TextType::TEXT_SIZE_S           , _T("［＃"                              ), _T("小さな文字］"        )},
	{TxtMiru::TextType::PICTURE_LAYOUT        , _T("［＃"                              ), _T("）］"                )},
	{TxtMiru::TextType::PICTURE_LAYOUT        , _T("［＃"                              ), _T("入る］"              )},
	{TxtMiru::TextType::OTHER                 , _T("※［＃"                            ), _T("］"                  )},
	{TxtMiru::TextType::OTHER                 , _T("［＃"                              ), _T("］"                  )},
};
static struct MatchStrMapData
{
	TxtMiru::TextType text_type;
	LPCTSTR const str;
} MSMD[] = {
	{TxtMiru::TextType::WHITE_DOT           , _T("白ゴマ傍点"        )},
	{TxtMiru::TextType::ROUND_DOT           , _T("丸傍点"            )},
	{TxtMiru::TextType::WHITE_ROUND_DOT     , _T("白丸傍点"          )},
	{TxtMiru::TextType::BLACK_TRIANGLE_DOT  , _T("黒三角傍点"        )},
	{TxtMiru::TextType::WHITE_TRIANGLE_DOT  , _T("白三角傍点"        )},
	{TxtMiru::TextType::DOUBLE_ROUND_DOT    , _T("二重丸傍点"        )},
	{TxtMiru::TextType::BULLS_EYE_DOT       , _T("蛇の目傍点"        )},
	{TxtMiru::TextType::SALTIRE_DOT         , _T("ばつ傍点"          )},
	{TxtMiru::TextType::DOT                 , _T("傍点"              )},
	{TxtMiru::TextType::LINE                , _T("傍線"              )},
	{TxtMiru::TextType::WAVE_LINE           , _T("波線"              )},
	{TxtMiru::TextType::DEL_LINE            , _T("取消線"            )},
	{TxtMiru::TextType::UNDER_LINE          , _T("左傍線"            )},
	{TxtMiru::TextType::SHORT_DASHED_LINE   , _T("破線"              )},
	{TxtMiru::TextType::DOT_LINE            , _T("点線"              )},
	{TxtMiru::TextType::DOUBLE_LINE         , _T("二重傍線"          )},
	{TxtMiru::TextType::WHITE_DOT_L         , _T("左白ゴマ傍点"      )},
	{TxtMiru::TextType::ROUND_DOT_L         , _T("左丸傍点"          )},
	{TxtMiru::TextType::WHITE_ROUND_DOT_L   , _T("左白丸傍点"        )},
	{TxtMiru::TextType::BLACK_TRIANGLE_DOT_L, _T("左黒三角傍点"      )},
	{TxtMiru::TextType::WHITE_TRIANGLE_DOT_L, _T("左白三角傍点"      )},
	{TxtMiru::TextType::DOUBLE_ROUND_DOT_L  , _T("左二重丸傍点"      )},
	{TxtMiru::TextType::BULLS_EYE_DOT_L     , _T("左蛇の目傍点"      )},
	{TxtMiru::TextType::SALTIRE_DOT_L       , _T("左ばつ傍点"        )},
	{TxtMiru::TextType::DOT_L               , _T("左傍点"            )},
	{TxtMiru::TextType::LINE_L              , _T("左傍線"            )},
	{TxtMiru::TextType::WAVE_LINE_L         , _T("左波線"            )},
	{TxtMiru::TextType::SHORT_DASHED_LINE_L , _T("左破線"            )},
	{TxtMiru::TextType::DOT_LINE_L          , _T("左点線"            )},
	{TxtMiru::TextType::DOUBLE_LINE_L       , _T("左二重傍線"        )},
	{TxtMiru::TextType::INDENT_START        , _T("折り返して"        )},
	{TxtMiru::TextType::NOTE                , _T("」に「"            )},
	{TxtMiru::TextType::RUBY_L              , _T("」の左に「"        )},
	{TxtMiru::TextType::NOTE_L              , _T("」の左に「"        )},
	{TxtMiru::TextType::ERROR_STR           , _T("」は底本では「"    )},
	{TxtMiru::TextType::ERROR_STR           , _T("」は底本では「"    )},
	{TxtMiru::TextType::TEXT_SIZE           , _T("」は"              )},
	{TxtMiru::TextType::UNKOWN_ERROR        , _T("ママ"              )},
	{TxtMiru::TextType::NO_READ             , _T("※"                )},
	{TxtMiru::TextType::LINE_CHAR           , _T("―"                )},
	{TxtMiru::TextType::OTHER               , _T("、"                )},
	{TxtMiru::TextType::MOVING_BORDER       , _T("罫囲み"            )},
};

static struct MatchStrData
{
	TxtMiru::TextType text_type;
	LPCTSTR const str;
} MSD[] = { // GetTTList() : l_TTList [TTLISTLEN]
	{TxtMiru::TextType::RUBY_SEP    , _T("｜"    )},
	{TxtMiru::TextType::KU1_CHAR    , _T("／＼"  )},
	{TxtMiru::TextType::KU2_CHAR    , _T("／″＼")},
	{TxtMiru::TextType::OVERLAP_CHAR, _T("゜"    )},
	{TxtMiru::TextType::OVERLAP_CHAR, _T("゛"    )},
	{TxtMiru::TextType::LINE_CHAR   , _T("―"    )},
};

#define PAIRTTLISTLEN (sizeof(MPSD)/sizeof(MatchPairStrData))
#define TTLEN         (sizeof(MSMD)/sizeof(MatchStrData    ))
#define TTLISTLEN     (sizeof(MSD )/sizeof(MatchStrData    ))

static MatchPairStr l_PairTTList[sizeof(MPSD)/sizeof(MatchPairStrData)];
static MatchStr     l_TTList    [sizeof(MSD )/sizeof(MatchStrData    )];
static MatchStr     l_TT        [static_cast<int>(TxtMiru::TextType::MaxNum)];


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
		l_TT[static_cast<int>(item.text_type)].Set(item.str);
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
