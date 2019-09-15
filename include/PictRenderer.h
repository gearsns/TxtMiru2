#ifndef __PICTRENDERER_H__
#define __PICTRENDERER_H__
#include "stltchar.h"
#include "Bitmap.h"
#include <map>
#include <vector>
#include "stlutil.h"

class CGrPictRenderer
{
public:
	CGrPictRenderer(){};
	virtual ~CGrPictRenderer(){};
	struct PictFileType {
		std::tstring ext;
		std::tstring name;
	};
	typedef std::vector<PictFileType> PictFileTypeList;
public:
	virtual bool IsSupported(LPCTSTR fileName) = 0;
	virtual void Draw(HDC hdc, int x, int y, int w, int h, LPCTSTR fileName) = 0;
	virtual void Draw(CGrBitmap &canvas, int x, int y, int w, int h, LPCTSTR fileName) = 0;
	virtual void StretchBlt(CGrBitmap &canvas, int x, int y, int w, int h, LPCTSTR fileName) = 0;
	virtual void Clear(){};
	virtual LPCTSTR Name() const = 0;
	virtual void GetFileTypeList(PictFileTypeList &pftl) = 0;

	virtual bool SetParam(LPCTSTR name, LPCTSTR val){ return false; }
	virtual bool GetParam(LPCTSTR name, LPTSTR val, int len){ return false; }
};

class CGrPictOleRenderer : public CGrPictRenderer
{
public:
	CGrPictOleRenderer();
	virtual ~CGrPictOleRenderer();
public:
	virtual bool IsSupported(LPCTSTR fileName);
	virtual void Draw(HDC hdc, int x, int y, int w, int h, LPCTSTR fileName);
	virtual void Draw(CGrBitmap &canvas, int x, int y, int w, int h, LPCTSTR fileName);
	virtual void StretchBlt(CGrBitmap &canvas, int x, int y, int w, int h, LPCTSTR fileName);
	virtual LPCTSTR Name() const { return _T("CGrPictOleRenderer"); }
	virtual void Clear();
	virtual void GetFileTypeList(PictFileTypeList &pftl);

	virtual bool SetParam(LPCTSTR name, LPCTSTR val);
	virtual bool GetParam(LPCTSTR name, LPTSTR val, int len);
public:
	bool Open(LPCTSTR fileName, CGrBitmap *lpBmp = NULL);
private:
	std::tstring m_curDir;
};

class CGrSPIFile;
class CGrPictSPIRenderer : public CGrPictRenderer
{
private:
	bool m_bLoadPlugin;
	std::vector<CGrSPIFile*> m_plugins;
	std::map<std::tstring,CGrSPIFile*> m_files;
	std::tstring m_curDir;
	std::tstring m_pluginDir;
private:
	bool loadPlugin();
	CGrSPIFile *getSPI(LPCTSTR fileName);
public:
	CGrPictSPIRenderer();
	virtual ~CGrPictSPIRenderer();
public:
	virtual bool IsSupported(LPCTSTR fileName);
	virtual void Draw(HDC hdc, int x, int y, int w, int h, LPCTSTR fileName);
	virtual void Draw(CGrBitmap &canvas, int x, int y, int w, int h, LPCTSTR fileName);
	virtual void StretchBlt(CGrBitmap &canvas, int x, int y, int w, int h, LPCTSTR fileName);
	virtual LPCTSTR Name() const { return _T("CGrPictSPIRenderer"); }
	virtual void Clear();
	virtual void GetFileTypeList(PictFileTypeList &pftl);

	virtual bool SetParam(LPCTSTR name, LPCTSTR val);
	virtual bool GetParam(LPCTSTR name, LPTSTR val, int len);
};

class CGrPictEmfRenderer : public CGrPictRenderer
{
public:
	CGrPictEmfRenderer();
	virtual ~CGrPictEmfRenderer();
public:
	virtual bool IsSupported(LPCTSTR fileName);
	virtual void Draw(HDC hdc, int x, int y, int w, int h, LPCTSTR fileName);
	virtual void Draw(CGrBitmap &canvas, int x, int y, int w, int h, LPCTSTR fileName);
	virtual void StretchBlt(CGrBitmap &canvas, int x, int y, int w, int h, LPCTSTR fileName);
	virtual LPCTSTR Name() const { return _T("CGrPictEmfRenderer"); }
	virtual void Clear();
	virtual void GetFileTypeList(PictFileTypeList &pftl);

	virtual bool SetParam(LPCTSTR name, LPCTSTR val);
	virtual bool GetParam(LPCTSTR name, LPTSTR val, int len);
private:
	bool Open(LPCTSTR fileName);
private:
	std::tstring m_curDir;
	std::simple_array<BYTE> m_emfData;
	HPALETTE m_hPal = NULL;
	int m_iWidth  = 0;
	int m_iHeight = 0;
};

#endif // __PICTRENDERER_H__
