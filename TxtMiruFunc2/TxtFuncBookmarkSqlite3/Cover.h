#ifndef __COVER_H__
#define __COVER_H__

#include "WindowCtrl.h"
#include "Bitmap.h"
#include "TxtFunc.h"
#include "TxtFuncBookmarkDef.h"
#include "PictRendererMgr.h"

class CGrCover : public CGrWinCtrl
{
public:
	CGrCover();
	virtual ~CGrCover();
	HWND Create(HWND hWnd);
	bool Load(int id);
	bool Load(LPCTSTR lpFileName);
	const LPCTSTR GetFilename();
	void Attach(HWND hWnd);
protected:
	virtual LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	bool load(LPCTSTR lpFileName);
private:
	std::tstring m_filename;
	CGrPictRendererMgr m_pictMgr;
	CGrBitmap m_image;
	bool m_bLoad = false;
};

#endif // __COVER_H__
