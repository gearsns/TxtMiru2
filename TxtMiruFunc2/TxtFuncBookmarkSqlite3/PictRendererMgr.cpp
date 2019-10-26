#pragma warning( disable : 4786 )

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "PictRendererMgr.h"
#include "stltchar.h"
#include "Image.h"
#include "TxtFunc.h"
#include "TxtFuncIParam.h"
#include "Shell.h"

CGrPictRendererMgr::CGrPictRendererMgr()
{
	memset(m_pictRendererMap, 0x00, sizeof(m_pictRendererMap));
}

CGrPictRendererMgr::~CGrPictRendererMgr()
{
	Clear();
}

void CGrPictRendererMgr::Clear()
{
	auto *p = m_pictRendererMap;
	for(int len=static_cast<int>(PictRenderType::MaxNum); len>0; --len, ++p){
		if(*p){ delete *p; }
		*p = nullptr;
	}
	m_curPictRenderer = nullptr;
	m_filename = _T("");
}

void CGrPictRendererMgr::Initialize()
{
	Clear();
	auto &&param = CGrTxtFunc::Param();
	m_pictRendererMap[static_cast<int>(PictRenderType::Spi)] = new CGrPictSPIRenderer();
	m_pictRendererMap[static_cast<int>(PictRenderType::Ole)] = new CGrPictOleRenderer();
	m_pictRendererMap[static_cast<int>(PictRenderType::Emf)] = new CGrPictEmfRenderer();
	TCHAR dir[_MAX_PATH] = {};
	param.GetText(CGrTxtFuncIParam::TextType::SpiPluginFolder, dir, sizeof(dir)/sizeof(TCHAR));
	m_pictRendererMap[static_cast<int>(PictRenderType::Spi)]->SetParam(_T("PluginDir"), dir);
	auto *p = m_pictRendererMap;
	for(int len= static_cast<int>(PictRenderType::MaxNum); len>0; --len, ++p){
		if(*p){
			(*p)->SetParam(_T("CurDir"), _T("."));
			(*p)->SetParam(_T("DataDir"), CGrTxtFunc::GetDataPath());
		}
	}
}

CGrPictRenderer *CGrPictRendererMgr::getPictRenderer(LPCTSTR lpFileName)
{
	if(m_curPictRenderer && m_filename == lpFileName){
		return m_curPictRenderer;
	}
	if(!CGrShell::GetSearchDir(lpFileName)){
		return nullptr;
	}
	auto *p = m_pictRendererMap;
	for(int len=static_cast<int>(PictRenderType::MaxNum); len>0; --len, ++p){
		if(!*p){ continue; }
		(*p)->SetParam(_T("CurDir"), _T("."));
		if((*p)->IsSupported(lpFileName)){
			return *p;
		}
	}
	return nullptr;
}

bool CGrPictRendererMgr::Draw(CGrBitmap &bmp, int x, int y, int w, int h, LPCTSTR lpFileName)
{
	auto ppr = getPictRenderer(lpFileName);
	if(ppr){
		m_curPictRenderer = ppr;
		m_filename = lpFileName;
		ppr->Draw(bmp, x, y, w, h, lpFileName);
		return true;
	}
	return false;
}

bool CGrPictRendererMgr::Draw(HDC hdc, int x, int y, int w, int h, LPCTSTR lpFileName)
{
	CGrBitmap bmp;
	if(Draw(bmp, 0, 0, w, h, lpFileName)){
		auto hImageDC = CreateCompatibleDC(NULL);
		auto hOldBitmap = SelectBitmap(hImageDC, bmp);
		BitBlt(hdc, x, y, bmp.Width(), bmp.Height(), hImageDC, 0, 0, SRCCOPY);
		SelectBitmap(hImageDC, hOldBitmap);
		DeleteDC(hImageDC);
		return true;
	}
	return false;
}

bool CGrPictRendererMgr::IsSupported(LPCTSTR lpFileName)
{
	auto ppr = getPictRenderer(lpFileName);
	return ppr != nullptr;
}

bool CGrPictRendererMgr::GetSupportedFile(std::tstring &out_filename, LPCTSTR lpFileName)
{
	WIN32_FIND_DATA wfd = {0};
	if(CGrShell::getFileInfo(lpFileName, wfd)){
		if(IsSupported(lpFileName)){
			out_filename = lpFileName;
			return true;
		}
		return false;
	}
	const TCHAR *fileExt[] = {
		_T("png"), _T("jpg"), _T("jpeg"), _T("emf"), _T("bmp"),
	};
	for(int i=0; i<sizeof(fileExt)/sizeof(TCHAR*); ++i){
		TCHAR path[MAX_PATH];
		_stprintf_s(path, _T("%s.%s"), lpFileName, fileExt[i]);
		if(IsSupported(path)){
			out_filename = path;
			return true;
		}
	}
	return false;
}
