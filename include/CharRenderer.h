#ifndef __CHARRENDERER_H__
#define __CHARRENDERER_H__

#include <windows.h>
#include <windowsx.h>
#include "Font.h"
#include "Bitmap.h"

#include <map>

class CGrCharRenderer
{
public:
	CGrCharRenderer(){};
	virtual ~CGrCharRenderer(){};

	virtual bool Draw(CGrBitmap &canvas, int x, int y, LPCTSTR str, int len) = 0;
	virtual void PatternFill(CGrBitmap &canvas, int x, int y, int width, int height, LPCTSTR str, int len) = 0;

	virtual bool SetFont(CGrFont &font) = 0;
	virtual CGrFont &GetFont() = 0;

	virtual void SetTextColor(COLORREF color) = 0;
	virtual COLORREF GetTextColor() = 0;

	virtual void SetCacheCharList(LPCTSTR str){};
	virtual LPCTSTR Name() const = 0;

	virtual bool SetParam(LPCTSTR name, LPCTSTR val){ return false; }
	virtual bool GetParam(LPCTSTR name, LPTSTR val, int len){ return false; }
};

// k-nearest neighbor algorithmÅAk-NN
class CGrCharKNNRenderer : public CGrCharRenderer
{
public:
	CGrCharKNNRenderer(int k);
	virtual ~CGrCharKNNRenderer();

	virtual bool Draw(CGrBitmap &canvas, int x, int y, LPCTSTR str, int len) = 0;
	virtual void PatternFill(CGrBitmap &canvas, int x, int y, int width, int height, LPCTSTR str, int len);

	virtual bool SetFont(CGrFont &font);
	virtual CGrFont &GetFont();

	virtual void SetTextColor(COLORREF color);
	virtual COLORREF GetTextColor();

	virtual void SetCacheCharList(LPCTSTR str);
	virtual LPCTSTR Name() const = NULL;

	virtual bool SetParam(LPCTSTR name, LPCTSTR val);
	virtual bool GetParam(LPCTSTR name, LPTSTR val, int len);
protected:
	virtual void makeColorMap();
	void createWorkDC();
	bool getCache(LPCTSTR str, int len, CGrMonoBitmap **lppWorkBitmap);
	bool drawText(LPCTSTR str, int len, CGrMonoBitmap *lpWorkBitmap);
	void clearCache();
protected:
	int       m_iHeight = 0;
	int       m_iWidth = 0;
	// çÏã∆óp
	HDC           m_hWorkDC = NULL;
	CGrMonoBitmap m_WorkBitmap;
	//
	HDC           m_hBackDC = NULL;
	CGrBitmap     m_BackBitmap;
	RGBQUAD       m_TextColor = {};
	DWORD         m_TextColorMapODD [256] = {};
	DWORD         m_TextColorMapEVEN[256] = {};
	CGrFont       m_BaseFont;
	CGrFont       m_DrawFont;
	int           m_k_nearest = 0;
	double        m_FontSpacing = 0;
	int           m_FontSpacingOffset = 0;
	bool          m_bCentering = false;
	//
	std::map<std::tstring,CGrMonoBitmap*> m_FontCache;
};

#define CharRendererName_4NNR _T("CGrChar4NNRenderer")
class CGrChar4NNRenderer : public CGrCharKNNRenderer
{
public:
	CGrChar4NNRenderer();
	virtual ~CGrChar4NNRenderer();

	virtual bool Draw(CGrBitmap &canvas, int x, int y, LPCTSTR str, int len);
	virtual void PatternFill(CGrBitmap &canvas, int x, int y, int width, int height, LPCTSTR str, int len);

	virtual LPCTSTR Name() const { return CharRendererName_4NNR; };
protected:
	virtual void makeColorMap();
private:
	BYTE m_ColorMap[17] = {};
};

#define CharRendererName_8NNR _T("CGrChar8NNRenderer")
class CGrChar8NNRenderer : public CGrCharKNNRenderer
{
public:
	CGrChar8NNRenderer();
	virtual ~CGrChar8NNRenderer();

	virtual bool Draw(CGrBitmap &canvas, int x, int y, LPCTSTR str, int len);
	virtual void PatternFill(CGrBitmap &canvas, int x, int y, int width, int height, LPCTSTR str, int len);

	virtual LPCTSTR Name() const { return CharRendererName_8NNR; };
protected:
	virtual void makeColorMap();
private:
	BYTE m_ColorMap[65] = {};
};

#define CharRendererName_LCDR _T("CGrCharLCDRenderer")
class CGrCharLCDRenderer : public CGrCharKNNRenderer
{
public:
	CGrCharLCDRenderer();
	virtual ~CGrCharLCDRenderer();

	virtual bool Draw(CGrBitmap &canvas, int x, int y, LPCTSTR str, int len);
	virtual void PatternFill(CGrBitmap &canvas, int x, int y, int width, int height, LPCTSTR str, int len);

	virtual void SetTextColor(COLORREF color);
	virtual LPCTSTR Name() const { return CharRendererName_LCDR; };
protected:
	virtual void makeColorMap();
private:
	BYTE m_ColorMap[17] = {};
	int m_iRed  [256] = {};
	int m_iGreen[256] = {};
	int m_iBlue [256] = {};
};

#define CharRendererName_Ext _T("CGrCharExtRenderer")
class CGrCharExtRenderer : public CGrCharRenderer
{
public:
	CGrCharExtRenderer();
	virtual ~CGrCharExtRenderer();

	virtual bool Draw(CGrBitmap &canvas, int x, int y, LPCTSTR str, int len);
	virtual void PatternFill(CGrBitmap &canvas, int x, int y, int width, int height, LPCTSTR str, int len);

	virtual bool SetFont(CGrFont &font);
	virtual CGrFont &GetFont();

	virtual void SetTextColor(COLORREF color);
	virtual COLORREF GetTextColor();
	virtual LPCTSTR Name() const { return CharRendererName_Ext; };
	//
	void Unload();
	bool isLoaded(){ return m_hDLL != NULL; }
	FARPROC GetProcAddress(LPCSTR funcname)
	{
		if(!m_hDLL){
			m_hDLL = ::LoadLibrary(m_fileName.c_str());
		}
		if(m_hDLL){
			return ::GetProcAddress(m_hDLL, funcname);
		}
		return nullptr;
	}

private:
	HMODULE m_hDLL = NULL;
	std::tstring m_fileName;
	CGrCharRenderer *m_pCharRenderer = nullptr;
	CGrFont m_defFont;
};

#endif // __CHARRENDERER_H__
