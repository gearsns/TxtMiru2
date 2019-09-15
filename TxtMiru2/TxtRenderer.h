#ifndef __TXTRENDERER_H__
#define __TXTRENDERER_H__

#include "CharRenderer.h"
#include "PictRenderer.h"
#include "Bitmap.h"
#include "TxtParam.h"

class CGrTxtDocument;
class CGrTxtRendererMgr
{
public:
	enum error_code {err_char_renderer_notfound};
	enum FontType {
		Text    , HalfText    , RotateText    , HalfRotateText    , TurnText    , HalfTurnText    ,
		BoldText, BoldHalfText, BoldRotateText, BoldHalfRotateText, BoldTurnText, BoldHalfTurnText,
		RubyText, TurnRubyText, Note, Nombre, RunningHeads,
		FT_MaxNum };
	enum PictRenderType { PRT_Ole, PRT_Spi, PRT_Emf, PRT_MaxNum };
	struct BorderType {
		bool top;
		bool left;
		bool bottom;
		bool right;
	};
	// キャッシュ
	struct CacheBitmap {
		CGrBitmap bmp;
		SIZE      size = {};
		int       iRefCount = 0;
	};
	using LPCacheBitmap = CacheBitmap *;
	using CacheBitmapMap = std::map<std::tstring, LPCacheBitmap>;
public:
	CGrTxtRendererMgr();
	virtual ~CGrTxtRendererMgr();

	void Create(CGrTxtDocument &doc, int text_size, int ruby_size, int note_size, int nombre_size, int running_heads_size);
	void Initialize(CGrTxtDocument &doc);
	void Clear();
	void ClearPict();
	void ClearChar();
	void ClearFlexChar();
	void ClearCache();
	void SetBitmap(CGrBitmap &bmp);
	void SetTextColor(FontType ft, COLORREF color);
	void SetTextColor(COLORREF color);
	void ResetTextColor();
	COLORREF GetTextColor(FontType ft);

	bool DrawText(FontType ft, int x, int y, LPCTSTR lpSrc, LPCTSTR lpSrcEnd);
	bool DrawText(FontType ft, int x, int y, LPCTSTR lpSrc, int len);
	void PatternFill(FontType ft, int x, int y, int w, int h, LPCTSTR lpSrc, int len);
	void DrawPict(int x, int y, int w, int h, LPCTSTR lpFileName);
	bool DrawBorder(int x, int y, int w, int h, BorderType borderType);
	void StretchBlt(int x, int y, int w, int h, LPCTSTR lpFileName);
	int GetFontHeight(FontType ft);
	void SetAntialias(int iAntialias);
	int GetAntialias();

	SIZE GetCachePictSize(LPCTSTR lpFileName);

	void SetFontSize(int size, int text_size, int ruby_size, int note_size, int nombre_size, int running_heads_size);
	int GetFontSize(){ return m_iFontSize; }
private:
	using LPCGrCharRenderer = CGrCharRenderer *;
	using LPCGrPictRenderer = CGrPictRenderer *;
	LPCGrPictRenderer getPictRenderer(LPCTSTR lpFileName);
	// フォントサイズに応じた最適な CharRenderer を選択
	CGrCharRenderer &selectCharRenderer(int font_size, LPCGrCharRenderer *pcr);
	void createCharRenderer(int text_size, int ruby_size, int note_size, int nombre_size, int running_heads_size);
	void createCharRenderer(FontType ft, CGrTxtParam &param, CGrTxtParam::CharType ct, int size, int iRotate);
	void createFlexCharRenderer(FontType ft, CGrTxtParam &param, CGrTxtParam::CharType ct, int size, int iRotate);
private:
	LPCGrCharRenderer m_charRendererMap    [FT_MaxNum ] = {};
	LPCGrCharRenderer m_flexCharRendererMap[FT_MaxNum ] = {};
	LPCGrPictRenderer m_pictRendererMap    [PRT_MaxNum] = {};
	LPCGrCharRenderer *m_currentCharRendererMap = m_charRendererMap;
	std::map<std::tstring,LPCGrPictRenderer> m_pictFileRendererMap;  // ファイル名から m_pictRendererMap で Loadした CGrPictRenderer のマップ
	CGrBitmap *m_pBmpCanvas = nullptr;
	std::tstring m_curDir;
	int m_iAntialias = 1;
	int m_iFontSize = 0;
	CacheBitmapMap m_bitmapMapCache; // 画像展開済みデータのキャッシュ(Bitmap)
	bool drawCacheBlt(int x, int y, int w, int h, LPCTSTR fileName);
	void clearOldCache();
};

#endif // __TXTRENDERER_H__
