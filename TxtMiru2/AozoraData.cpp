#include "AozoraData.h"

// �ȉ��AMatchPairStrData�̐擪�����̔z��
static const TCHAR l_MatchPairFistStrData[] = {_T('��'), _T('�s'), _T('�v'), _T('�m')};
static struct MatchPairStrData
{
	TxtMiru::TextType text_type;
	LPCTSTR const first ;
	LPCTSTR const second;
} MPSD[] = { // GetPairTTList() :  l_PairTTList[PAIRTTLISTLEN]
	{TxtMiru::TT_RUBY                  , _T("�s"                                ), _T("�t"                  )},
	{TxtMiru::TT_NO_READ               , _T("���m�����Ǖs�A"                  ), _T("�n"                  )},
	{TxtMiru::TT_ERROR                 , _T(" �v�͒�{�ł́u"                   ), _T("�v�n"                )},
	{TxtMiru::TT_WHITE_DOT             , _T("�m���u"                            ), _T("�v�ɔ��S�}�T�_�n"    )},
	{TxtMiru::TT_ROUND_DOT             , _T("�m���u"                            ), _T("�v�ɊۖT�_�n"        )},
	{TxtMiru::TT_WHITE_ROUND_DOT       , _T("�m���u"                            ), _T("�v�ɔ��ۖT�_�n"      )},
	{TxtMiru::TT_BLACK_TRIANGLE_DOT    , _T("�m���u"                            ), _T("�v�ɍ��O�p�T�_�n"    )},
	{TxtMiru::TT_WHITE_TRIANGLE_DOT    , _T("�m���u"                            ), _T("�v�ɔ��O�p�T�_�n"    )},
	{TxtMiru::TT_DOUBLE_ROUND_DOT      , _T("�m���u"                            ), _T("�v�ɓ�d�ۖT�_�n"    )},
	{TxtMiru::TT_BULLS_EYE_DOT         , _T("�m���u"                            ), _T("�v�Ɏւ̖ږT�_�n"    )},
	{TxtMiru::TT_SALTIRE_DOT           , _T("�m���u"                            ), _T("�v�ɂ΂T�_�n"      )},
	{TxtMiru::TT_DOT                   , _T("�m���u"                            ), _T("�v�ɖT�_�n"          )},
	{TxtMiru::TT_LINE                  , _T("�m���u"                            ), _T("�v�ɖT���n"          )},
	{TxtMiru::TT_WAVE_LINE             , _T("�m���u"                            ), _T("�v�ɔg���n"          )},
	{TxtMiru::TT_WAVE_LINE             , _T("�m���u"                            ), _T("�v�ɔg�T���n"        )},
	{TxtMiru::TT_DEL_LINE              , _T("�m���u"                            ), _T("�v�Ɏ�����n"        )},
	{TxtMiru::TT_UNDER_LINE            , _T("�m���u"                            ), _T("�v�ɍ��T���n"        )},
	{TxtMiru::TT_UNDER_LINE            , _T("�m���u"                            ), _T("�v�̍��ɖT���n"      )},
	{TxtMiru::TT_UNDER_LINE            , _T("�m���u"                            ), _T("�v�ɃA���_�[���C���n")},
	{TxtMiru::TT_SHORT_DASHED_LINE     , _T("�m���u"                            ), _T("�v�ɔj���n"          )},
	{TxtMiru::TT_DOT_LINE              , _T("�m���u"                            ), _T("�v�ɍ����n"          )},
	{TxtMiru::TT_DOT_LINE              , _T("�m���u"                            ), _T("�v�ɓ_���n"          )},
	{TxtMiru::TT_DOUBLE_LINE           , _T("�m���u"                            ), _T("�v�ɓ�d�T���n"      )},
	{TxtMiru::TT_WHITE_DOT_L           , _T("�m���u"                            ), _T("�v�̍��ɔ��S�}�T�_�n")},
	{TxtMiru::TT_ROUND_DOT_L           , _T("�m���u"                            ), _T("�v�̍��ɊۖT�_�n"    )},
	{TxtMiru::TT_WHITE_ROUND_DOT_L     , _T("�m���u"                            ), _T("�v�̍��ɔ��ۖT�_�n"  )},
	{TxtMiru::TT_BLACK_TRIANGLE_DOT_L  , _T("�m���u"                            ), _T("�v�̍��ɍ��O�p�T�_�n")},
	{TxtMiru::TT_WHITE_TRIANGLE_DOT_L  , _T("�m���u"                            ), _T("�v�̍��ɔ��O�p�T�_�n")},
	{TxtMiru::TT_DOUBLE_ROUND_DOT_L    , _T("�m���u"                            ), _T("�v�̍��ɓ�d�ۖT�_�n")},
	{TxtMiru::TT_BULLS_EYE_DOT_L       , _T("�m���u"                            ), _T("�v�̍��Ɏւ̖ږT�_�n")},
	{TxtMiru::TT_SALTIRE_DOT_L         , _T("�m���u"                            ), _T("�v�̍��ɂ΂T�_�n"  )},
	{TxtMiru::TT_DOT_L                 , _T("�m���u"                            ), _T("�v�̍��ɖT�_�n"      )},
	{TxtMiru::TT_LINE_L                , _T("�m���u"                            ), _T("�v�̍��ɖT���n"      )},
	{TxtMiru::TT_WAVE_LINE_L           , _T("�m���u"                            ), _T("�v�̍��ɔg���n"      )},
	{TxtMiru::TT_WAVE_LINE_L           , _T("�m���u"                            ), _T("�v�̍��ɔg�T���n"    )},
	{TxtMiru::TT_SHORT_DASHED_LINE_L   , _T("�m���u"                            ), _T("�v�̍��ɔj���n"      )},
	{TxtMiru::TT_DOT_LINE_L            , _T("�m���u"                            ), _T("�v�̍��ɍ����n"      )},
	{TxtMiru::TT_DOT_LINE_L            , _T("�m���u"                            ), _T("�v�̍��ɓ_���n"      )},
	{TxtMiru::TT_DOUBLE_LINE_L         , _T("�m���u"                            ), _T("�v�̍��ɓ�d�T���n"  )},
	{TxtMiru::TT_CAPTION               , _T("�m���u"                            ), _T("�v�̓L���v�V�����n"  )},
	{TxtMiru::TT_CENTER                , _T("�m���y�[�W�̍��E����"              ), _T("�n"                  )},
	{TxtMiru::TT_WHITE_DOT             , _T("�m�����S�}�T�_"                    ), _T("�n"                  )},
	{TxtMiru::TT_ROUND_DOT             , _T("�m���ۖT�_"                        ), _T("�n"                  )},
	{TxtMiru::TT_WHITE_ROUND_DOT       , _T("�m�����ۖT�_"                      ), _T("�n"                  )},
	{TxtMiru::TT_BLACK_TRIANGLE_DOT    , _T("�m�����O�p�T�_"                    ), _T("�n"                  )},
	{TxtMiru::TT_WHITE_TRIANGLE_DOT    , _T("�m�����O�p�T�_"                    ), _T("�n"                  )},
	{TxtMiru::TT_DOUBLE_ROUND_DOT      , _T("�m����d�ۖT�_"                    ), _T("�n"                  )},
	{TxtMiru::TT_BULLS_EYE_DOT         , _T("�m���ւ̖ږT�_"                    ), _T("�n"                  )},
	{TxtMiru::TT_SALTIRE_DOT           , _T("�m���΂T�_"                      ), _T("�n"                  )},
	{TxtMiru::TT_DOT                   , _T("�m���T�_"                          ), _T("�n"                  )},
	{TxtMiru::TT_LINE                  , _T("�m���T��"                          ), _T("�n"                  )},
	{TxtMiru::TT_WAVE_LINE             , _T("�m���g��"                          ), _T("�n"                  )},
	{TxtMiru::TT_WAVE_LINE             , _T("�m���g�T��"                        ), _T("�n"                  )},
	{TxtMiru::TT_DEL_LINE              , _T("�m�������"                        ), _T("�n"                  )},
	{TxtMiru::TT_UNDER_LINE            , _T("�m�����ɖT��"                      ), _T("�n"                  )},
	{TxtMiru::TT_UNDER_LINE            , _T("�m�����T��"                        ), _T("�n"                  )},
	{TxtMiru::TT_UNDER_LINE            , _T("�m���A���_�[���C��"                ), _T("�n"                  )},
	{TxtMiru::TT_SHORT_DASHED_LINE     , _T("�m���j��"                          ), _T("�n"                  )},
	{TxtMiru::TT_DOT_LINE              , _T("�m������"                          ), _T("�n"                  )},
	{TxtMiru::TT_DOT_LINE              , _T("�m���_��"                          ), _T("�n"                  )},
	{TxtMiru::TT_DOUBLE_LINE           , _T("�m����d�T��"                      ), _T("�n"                  )},
	{TxtMiru::TT_WHITE_DOT_L           , _T("�m�����ɔ��S�}�T�_"                ), _T("�n"                  )},
	{TxtMiru::TT_ROUND_DOT_L           , _T("�m�����ɊۖT�_"                    ), _T("�n"                  )},
	{TxtMiru::TT_WHITE_ROUND_DOT_L     , _T("�m�����ɔ��ۖT�_"                  ), _T("�n"                  )},
	{TxtMiru::TT_BLACK_TRIANGLE_DOT_L  , _T("�m�����ɍ��O�p�T�_"                ), _T("�n"                  )},
	{TxtMiru::TT_WHITE_TRIANGLE_DOT_L  , _T("�m�����ɔ��O�p�T�_"                ), _T("�n"                  )},
	{TxtMiru::TT_DOUBLE_ROUND_DOT_L    , _T("�m�����ɓ�d�ۖT�_"                ), _T("�n"                  )},
	{TxtMiru::TT_BULLS_EYE_DOT_L       , _T("�m�����Ɏւ̖ږT�_"                ), _T("�n"                  )},
	{TxtMiru::TT_SALTIRE_DOT           , _T("�m�����ɂ΂T�_"                  ), _T("�n"                  )},
	{TxtMiru::TT_DOT_L                 , _T("�m�����ɖT�_"                      ), _T("�n"                  )},
	{TxtMiru::TT_LINE_L                , _T("�m�����ɖT��"                      ), _T("�n"                  )},
	{TxtMiru::TT_WAVE_LINE_L           , _T("�m�����ɔg��"                      ), _T("�n"                  )},
	{TxtMiru::TT_WAVE_LINE_L           , _T("�m�����ɔg�T��"                    ), _T("�n"                  )},
	{TxtMiru::TT_SHORT_DASHED_LINE_L   , _T("�m�����ɔj��"                      ), _T("�n"                  )},
	{TxtMiru::TT_DOT_LINE_L            , _T("�m�����ɍ���"                      ), _T("�n"                  )},
	{TxtMiru::TT_DOT_LINE_L            , _T("�m�����ɓ_��"                      ), _T("�n"                  )},
	{TxtMiru::TT_DOUBLE_LINE_L         , _T("�m�����ɓ�d�T��"                  ), _T("�n"                  )},
	{TxtMiru::TT_CAPTION               , _T("�m���L���v�V����"                  ), _T("�n"                  )},
	{TxtMiru::TT_CAPTION               , _T("�m����������L���v�V����"          ), _T("�n"                  )},
	{TxtMiru::TT_TXTMIRU               , _T("�m��TxtMiru2:"                     ), _T("�n"                  )},
	{TxtMiru::TT_NOTE                  , _T("�m���u"                            ), _T("�v�̒��L�n"          )},
	{TxtMiru::TT_SMALL_NOTE            , _T("�m���i"                            ), _T("�j�n"                )},
	{TxtMiru::TT_BOLD                  , _T("�m���u"                            ), _T("�v�͑����n"          )},
	{TxtMiru::TT_BOLD                  , _T("�m���u"                            ), _T("�v�̓S�V�b�N�́n"    )},
	{TxtMiru::TT_SUP_NOTE              , _T("�m���u"                            ), _T("�v�͏�t���������n"  )},
	{TxtMiru::TT_SUP_NOTE              , _T("�m��"                              ), _T("�͏�t���������n"    )},
	{TxtMiru::TT_SUB_NOTE              , _T("�m���u"                            ), _T("�v�͉��t���������n"  )},
	{TxtMiru::TT_SUB_NOTE              , _T("�m��"                              ), _T("�͉��t���������n"    )},
	{TxtMiru::TT_MOVING_BORDER         , _T("�m���u"                            ), _T("�v�͌r�͂݁n"        )},
	{TxtMiru::TT_LINE_BOX_START        , _T("�m����������"                      ), _T("�r�͂݁n"            )},
	{TxtMiru::TT_LINE_BOX_START        , _T("�m���r�͂�"                        ), _T("�n"                  )},
	{TxtMiru::TT_UNKOWN_ERROR          , _T("�m���u"                            ), _T("�v�̓}�}�n"          )},
	{TxtMiru::TT_UNKOWN_ERROR          , _T("�m���u"                            ), _T("�v�Ɂu�}�}�v�̒��L�n")},
	{TxtMiru::TT_INDENT2               , _T("�m�����̍s�V�t���A�܂�Ԃ���"      ), _T("�������n"            )},
	{TxtMiru::TT_INDENT3               , _T("�m���V����"                        ), _T("�������n"            )},
	{TxtMiru::TT_INDENT                , _T("�m�����̍s"                        ), _T("�������n"            )},
	{TxtMiru::TT_INDENT_START2         , _T("�m������������s�V�t���A�܂�Ԃ���"), _T("�������n"            )},
	{TxtMiru::TT_INDENT_START          , _T("�m����������"                      ), _T("�������n"            )},
	{TxtMiru::TT_INDENT                , _T("�m��"                              ), _T("�������n"            )}, // ������ �Ō��
	{TxtMiru::TT_INDENT_END            , _T("�m��������"                        ), _T("�������I���n"      )},
	{TxtMiru::TT_RINDENT               , _T("�m���n�t��"                        ), _T("�n"                  )},
	{TxtMiru::TT_RINDENT               , _T("�m�����̍s"                        ), _T("�n�t���n"            )},
	{TxtMiru::TT_RINDENT               , _T("�m���n����"                        ), _T("���グ�n"            )},
	{TxtMiru::TT_RINDENT_START         , _T("�m����������"                      ), _T("�n�t���n"            )},
	{TxtMiru::TT_RINDENT_START         , _T("�m����������n����"                ), _T("���グ�n"            )},
	{TxtMiru::TT_RINDENT_END           , _T("�m��������"                        ), _T("�n�t���I���n"      )},
	{TxtMiru::TT_RINDENT_END           , _T("�m��������"                        ), _T("���グ�I���n"      )},
	{TxtMiru::TT_RINDENT               , _T("�m��"                              ), _T("�n�t���n"            )}, // �n�t�� �Ō��
	{TxtMiru::TT_BOLD_START            , _T("�m������"                          ), _T("�n"                  )},
	{TxtMiru::TT_BOLD_START            , _T("�m����������"                      ), _T("�����n"              )},
	{TxtMiru::TT_BOLD_END              , _T("�m�������܂�"                      ), _T("�����n"              )},
	{TxtMiru::TT_BOLD_END              , _T("�m�������ő���"                    ), _T("�I���n"            )},
	{TxtMiru::TT_BOLD_END              , _T("�m������"                          ), _T("�I���n"            )},
	{TxtMiru::TT_LIMIT_CHAR_START      , _T("�m����������"                      ), _T("���l�߁n"            )},
	{TxtMiru::TT_LIMIT_CHAR_END        , _T("�m�������Ŏ��l��"                  ), _T("�I���n"            )},
	{TxtMiru::TT_ENDOFCONTENTS         , _T("�m���{��"                          ), _T("�I���n"            )},
	{TxtMiru::TT_TEXT_SIZE_L           , _T("�m����������"                      ), _T("�i�K�傫�ȕ����n"    )},
	{TxtMiru::TT_TEXT_SIZE_S           , _T("�m����������"                      ), _T("�i�K�����ȕ����n"    )},
	{TxtMiru::TT_TEXT_SIZE_L           , _T("�m���傫�ȕ���"                    ), _T("�n"                  )},
	{TxtMiru::TT_TEXT_SIZE_S           , _T("�m�������ȕ���"                    ), _T("�n"                  )},
	{TxtMiru::TT_PICTURE_LAYOUT        , _T("�m��"                              ), _T("�j����n"            )},
	{TxtMiru::TT_OTHER_NOTE            , _T("�m���u"                            ), _T("�v�n"                )},
	{TxtMiru::TT_ROTATE_NUM            , _T("�m���u"                            ), _T("�v�͏c�����n"        )},
	{TxtMiru::TT_HORZ                  , _T("�m���u"                            ), _T("�v�͉��g�݁n"        )}, // �m���u...�v�͉��g�݁n
	{TxtMiru::TT_HORZ_START            , _T("�m����������"                      ), _T("���g�݁n"            )}, // �m���������牡�g�݁n
	{TxtMiru::TT_HORZ_START            , _T("�m�����g��"                        ), _T("�n"                  )}, // �m�����g�݁n
	{TxtMiru::TT_SUBTITLE1             , _T("�m���u"                            ), _T("�v�͑匩�o���n"      )},
	{TxtMiru::TT_SUBTITLE2             , _T("�m���u"                            ), _T("�v�͒����o���n"      )},
	{TxtMiru::TT_SUBTITLE3             , _T("�m���u"                            ), _T("�v�͏����o���n"      )},
	{TxtMiru::TT_SUBTITLE1             , _T("�m���匩�o��"                      ), _T("�n"                  )},
	{TxtMiru::TT_SUBTITLE2             , _T("�m�������o��"                      ), _T("�n"                  )},
	{TxtMiru::TT_SUBTITLE3             , _T("�m�������o��"                      ), _T("�n"                  )},
	{TxtMiru::TT_SUBTITLE1             , _T("�m����������"                      ), _T("�匩�o���n"          )}, // ��������匩�o��
	{TxtMiru::TT_SUBTITLE2             , _T("�m����������"                      ), _T("�����o���n"          )}, // �������璆���o��
	{TxtMiru::TT_SUBTITLE3             , _T("�m����������"                      ), _T("�����o���n"          )}, // �������珬���o��
	{TxtMiru::TT_SMALL_NOTE_R          , _T("�m���u"                            ), _T("�v�͍s�E�������n"    )},
	{TxtMiru::TT_NOTE_START            , _T("�m�����L�t��"                      ), _T("�n"                  )},
	{TxtMiru::TT_NOTE_END              , _T("�m���u"                            ), _T("�v�̒��L�t���I���n")},
	{TxtMiru::TT_NOTE_START_L          , _T("�m�����ɒ��L�t��"                  ), _T("�n"                  )},
	{TxtMiru::TT_NOTE_END_L            , _T("�m�����Ɂu"                        ), _T("�v�̒��L�t���I���n")},
	{TxtMiru::TT_RUBY_L                , _T("�m���u"                            ), _T("�v�̃��r�n"          )},
	{TxtMiru::TT_RUBY_L_START          , _T("�m�����Ƀ��r�t��"                  ), _T("�n"                  )},
	{TxtMiru::TT_RUBY_L_END            , _T("�m�����Ɂu"                        ), _T("�v�̃��r�t���I���n")},
	{TxtMiru::TT_NEXT_LINE             , _T("�m�����s"                          ), _T("�n"                  )},
	{TxtMiru::TT_NEXT_LAYOUT           , _T("�m�����i"                          ), _T("�n"                  )},
	{TxtMiru::TT_NEXT_PAGE             , _T("�m������"                          ), _T("�n"                  )},
	{TxtMiru::TT_NEXT_PAGE             , _T("�m�����y�[�W"                      ), _T("�n"                  )},
	{TxtMiru::TT_NEXT_PAPER            , _T("�m������"                          ), _T("�n"                  )},
	{TxtMiru::TT_NEXT_PAPER_FIRST      , _T("�m�������J��"                      ), _T("�n"                  )},
	{TxtMiru::TT_RANGE_END             , _T("�m��"                              ), _T("�I���n"            )},
	{TxtMiru::TT_TEXT_SIZE_L           , _T("�m���u"                            ), _T("�i�K�傫�ȕ����n"    )},
	{TxtMiru::TT_TEXT_SIZE_S           , _T("�m���u"                            ), _T("�i�K�����ȕ����n"    )},
	{TxtMiru::TT_TEXT_SIZE_L           , _T("�m��"                              ), _T("�傫�ȕ����n"        )},
	{TxtMiru::TT_TEXT_SIZE_S           , _T("�m��"                              ), _T("�����ȕ����n"        )},
	{TxtMiru::TT_PICTURE_LAYOUT        , _T("�m��"                              ), _T("�j�n"                )},
	{TxtMiru::TT_PICTURE_LAYOUT        , _T("�m��"                              ), _T("����n"              )},
	{TxtMiru::TT_OTHER                 , _T("���m��"                            ), _T("�n"                  )},
	{TxtMiru::TT_OTHER                 , _T("�m��"                              ), _T("�n"                  )},
};
static struct MatchStrMapData
{
	TxtMiru::TextType text_type;
	LPCTSTR const str;
} MSMD[] = {
	{TxtMiru::TT_WHITE_DOT           , _T("���S�}�T�_"        )},
	{TxtMiru::TT_ROUND_DOT           , _T("�ۖT�_"            )},
	{TxtMiru::TT_WHITE_ROUND_DOT     , _T("���ۖT�_"          )},
	{TxtMiru::TT_BLACK_TRIANGLE_DOT  , _T("���O�p�T�_"        )},
	{TxtMiru::TT_WHITE_TRIANGLE_DOT  , _T("���O�p�T�_"        )},
	{TxtMiru::TT_DOUBLE_ROUND_DOT    , _T("��d�ۖT�_"        )},
	{TxtMiru::TT_BULLS_EYE_DOT       , _T("�ւ̖ږT�_"        )},
	{TxtMiru::TT_SALTIRE_DOT         , _T("�΂T�_"          )},
	{TxtMiru::TT_DOT                 , _T("�T�_"              )},
	{TxtMiru::TT_LINE                , _T("�T��"              )},
	{TxtMiru::TT_WAVE_LINE           , _T("�g��"              )},
	{TxtMiru::TT_DEL_LINE            , _T("�����"            )},
	{TxtMiru::TT_UNDER_LINE          , _T("���T��"            )},
	{TxtMiru::TT_SHORT_DASHED_LINE   , _T("�j��"              )},
	{TxtMiru::TT_DOT_LINE            , _T("�_��"              )},
	{TxtMiru::TT_DOUBLE_LINE         , _T("��d�T��"          )},
	{TxtMiru::TT_WHITE_DOT_L         , _T("�����S�}�T�_"      )},
	{TxtMiru::TT_ROUND_DOT_L         , _T("���ۖT�_"          )},
	{TxtMiru::TT_WHITE_ROUND_DOT_L   , _T("�����ۖT�_"        )},
	{TxtMiru::TT_BLACK_TRIANGLE_DOT_L, _T("�����O�p�T�_"      )},
	{TxtMiru::TT_WHITE_TRIANGLE_DOT_L, _T("�����O�p�T�_"      )},
	{TxtMiru::TT_DOUBLE_ROUND_DOT_L  , _T("����d�ۖT�_"      )},
	{TxtMiru::TT_BULLS_EYE_DOT_L     , _T("���ւ̖ږT�_"      )},
	{TxtMiru::TT_SALTIRE_DOT_L       , _T("���΂T�_"        )},
	{TxtMiru::TT_DOT_L               , _T("���T�_"            )},
	{TxtMiru::TT_LINE_L              , _T("���T��"            )},
	{TxtMiru::TT_WAVE_LINE_L         , _T("���g��"            )},
	{TxtMiru::TT_SHORT_DASHED_LINE_L , _T("���j��"            )},
	{TxtMiru::TT_DOT_LINE_L          , _T("���_��"            )},
	{TxtMiru::TT_DOUBLE_LINE_L       , _T("����d�T��"        )},
	{TxtMiru::TT_INDENT_START        , _T("�܂�Ԃ���"        )},
	{TxtMiru::TT_NOTE                , _T("�v�Ɂu"            )},
	{TxtMiru::TT_RUBY_L              , _T("�v�̍��Ɂu"        )},
	{TxtMiru::TT_NOTE_L              , _T("�v�̍��Ɂu"        )},
	{TxtMiru::TT_ERROR               , _T("�v�͒�{�ł́u"    )},
	{TxtMiru::TT_ERROR               , _T("�v�͒�{�ł́u"    )},
	{TxtMiru::TT_TEXT_SIZE           , _T("�v��"              )},
	{TxtMiru::TT_UNKOWN_ERROR        , _T("�}�}"              )},
	{TxtMiru::TT_NO_READ             , _T("��"                )},
	{TxtMiru::TT_LINE_CHAR           , _T("�\"                )},
	{TxtMiru::TT_OTHER               , _T("�A"                )},
	{TxtMiru::TT_MOVING_BORDER       , _T("�r�͂�"            )},
};

static struct MatchStrData
{
	TxtMiru::TextType text_type;
	LPCTSTR const str;
} MSD[] = { // GetTTList() : l_TTList [TTLISTLEN]
	{TxtMiru::TT_RUBY_SEP    , _T("�b"    )},
	{TxtMiru::TT_KU1_CHAR    , _T("�^�_"  )},
	{TxtMiru::TT_KU2_CHAR    , _T("�^���_")},
	{TxtMiru::TT_OVERLAP_CHAR, _T("�K"    )},
	{TxtMiru::TT_OVERLAP_CHAR, _T("�J"    )},
	{TxtMiru::TT_LINE_CHAR   , _T("�\"    )},
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
	for(const auto ch : {_T('�b'),_T('�^'),_T('�K'),_T('�J'),_T('�\')}){
		if(ch == *lpSrc){
			return true;
		}
	}
	return false;
}
