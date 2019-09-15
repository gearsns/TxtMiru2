#include <windows.h>
#include "MemDC.h"

CGrMemDC::CGrMemDC(HDC hdc, RECT &rect)
{
	m_rect = rect;
	m_hDC = hdc;
	m_hMemDC = CreateCompatibleDC(hdc);
	m_bitmap.Create(m_rect.right, m_rect.bottom);
	m_oldBitmap = SelectBitmap(m_hMemDC, m_bitmap);
}

CGrMemDC::~CGrMemDC()
{
	if(m_hMemDC){
		BitBlt(m_hDC, m_rect.left, m_rect.top, m_rect.right-m_rect.left, m_rect.bottom-m_rect.top, m_hMemDC, m_rect.left, m_rect.top, SRCCOPY);
		SelectBitmap(m_hMemDC, m_oldBitmap);
		DeleteDC(m_hMemDC);
	}
}
