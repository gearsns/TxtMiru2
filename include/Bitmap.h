#ifndef __BITMAP_H__
#define __BITMAP_H__

#include <windows.h>
#include <windowsx.h>

class CGrBitmap
{
protected:
	HBITMAP m_hBitmap;
	LPBYTE  m_lpbuffer;
	int     m_iWidth;
	int     m_iHeight;
public:
	CGrBitmap();
	virtual ~CGrBitmap();
	bool Create(int iWidth, int iHeight);
	void Destroy();
	void Clear();

	inline int Width(){ return m_iWidth; }
	inline int Height(){ return m_iHeight; }
	operator HBITMAP() const { return m_hBitmap; }
	LPRGBQUAD GetBits(){ return reinterpret_cast<LPRGBQUAD>(m_lpbuffer); }
};

class CGrMonoBitmap
{
protected:
	HBITMAP m_hBitmap;
	LPBYTE  m_lpbuffer;
	int     m_iWidth;
	int     m_iHeight;
public:
	CGrMonoBitmap();
	virtual ~CGrMonoBitmap();
	bool Create(int iWidth, int iHeight);

	void Destroy();
	void Clear();

	inline int Width(){ return m_iWidth; }
	inline int Height(){ return m_iHeight; }
	operator HBITMAP() const { return m_hBitmap; }
	LPBYTE GetBits(){ return m_lpbuffer; }
};

#endif __BITMAP_H__
