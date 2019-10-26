#ifndef __TXTMIRUTEXTTYPE_H__
#define __TXTMIRUTEXTTYPE_H__

#include "TxtMiruDef.h"

namespace TxtMiruType
{
	extern UINT Attr[static_cast<int>(TxtMiru::TextType::MaxNum)];
	const UINT Text      = 0b0'00000000'00000000'00000000'00000001; // TT_TEXT, TT_OTHER, TT_LINE_CHAR
	const UINT SmallNote = 0b0'00000000'00000000'00000000'00000010; // TT_RUBY, TT_RUBY_L, TT_SMALL_NOTE, TT_SMALL_NOTE_R, TT_SMALL_NOTE_L, TT_SUP_NOTE, TT_SUB_NOTE, TT_UNKOWN_ERROR, TT_GUID_MARK, TT_NOTE_L
	const UINT Comment   = 0b0'00000000'00000000'00000000'00000100; // TT_COMMENT_BEGIN, TT_COMMENT
	const UINT RotateNum = 0b0'00000000'00000000'00000000'00001000; // TT_ROTATE_NUM, TT_ROTATE_NUM_AUTO
	const UINT KuChar    = 0b0'00000000'00000000'00000000'00010000; // TT_KU1_CHAR, TT_KU2_CHAR
	const UINT SkipChar  = 0b0'00000000'00000000'00000000'00100000; // TT_SKIP_CHAR, TT_SKIP_CHAR_AUTO
	const UINT Picture   = 0b0'00000000'00000000'00000000'01000000; // TT_PICTURE_LAYOUT, TT_PICTURE_HALF_PAGE, TT_PICTURE_FULL_PAGE
	const UINT Subtitle  = 0b0'00000000'00000000'00000000'11000000; // TT_SUBTITLE1, TT_SUBTITLE2, TT_SUBTITLE3

	inline bool isText           (TxtMiru::TextType tt){ return 0 != (Attr[static_cast<int>(tt)] & Text); }
	inline bool isSmallNote      (TxtMiru::TextType tt){ return 0 != (Attr[static_cast<int>(tt)] & SmallNote); }
	inline bool isComment        (TxtMiru::TextType tt){ return 0 != (Attr[static_cast<int>(tt)] & Comment); }
	inline bool isRotateNum      (TxtMiru::TextType tt){ return 0 != (Attr[static_cast<int>(tt)] & RotateNum); }
	inline bool isKuChart        (TxtMiru::TextType tt){ return 0 != (Attr[static_cast<int>(tt)] & KuChar); }
	inline bool isSkipChar       (TxtMiru::TextType tt){ return 0 != (Attr[static_cast<int>(tt)] & SkipChar); }
	inline bool isPicture        (TxtMiru::TextType tt){ return 0 != (Attr[static_cast<int>(tt)] & Picture); }
	inline bool isSubtitle       (TxtMiru::TextType tt){ return 0 != (Attr[static_cast<int>(tt)] & Subtitle); }
	inline bool isTextOrSkip     (TxtMiru::TextType tt){ return 0 != (Attr[static_cast<int>(tt)] & (Text | SkipChar)); }
	inline bool isTextOrSkipOrRotateNum
		/*                     */(TxtMiru::TextType tt){ return 0 != (Attr[static_cast<int>(tt)] & (Text | SkipChar | RotateNum)); }
	inline bool isTextOrSkipOrRotateNumOrKuChar
		/*                     */(TxtMiru::TextType tt){ return 0 != (Attr[static_cast<int>(tt)] & (Text | SkipChar | RotateNum | KuChar)); }
	inline bool isTextOrRotateNumOrKuChar
		/*                     */(TxtMiru::TextType tt){ return 0 != (Attr[static_cast<int>(tt)] & (Text | RotateNum | KuChar)); }
	inline bool isTextOrRotateNumOrComment
		/*                     */(TxtMiru::TextType tt){ return 0 != (Attr[static_cast<int>(tt)] & (Text | RotateNum | Comment)); }
};

#endif
