#ifndef __PICTRENDERERMGR_H__
#define __PICTRENDERERMGR_H__

#include "Bitmap.h"
#include "PictRenderer.h"

class CGrPictRendererMgr
{
	enum PictRenderType { PRT_Ole, PRT_Spi, PRT_Emf, PRT_MaxNum };
public:
	CGrPictRendererMgr();
	virtual ~CGrPictRendererMgr();
	void Initialize();
	void Clear();
	bool Draw(CGrBitmap &bmp, int x, int y, int w, int h, LPCTSTR lpFileName);
	bool Draw(HDC hdc, int x, int y, int w, int h, LPCTSTR lpFileName);
	bool IsSupported(LPCTSTR lpFileName);
	bool GetSupportedFile(std::tstring &out_filename, LPCTSTR lpFileName);
private:
	CGrPictRenderer *getPictRenderer(LPCTSTR lpFileName);
	typedef CGrPictRenderer *LPCGrPictRenderer;
	LPCGrPictRenderer m_pictRendererMap[PRT_MaxNum];
	std::tstring m_filename;
	LPCGrPictRenderer m_curPictRenderer = nullptr;
};

#endif // __PICTRENDERERMGR_H__
