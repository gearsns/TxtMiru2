#include "Bitmap.h"

CGrBitmap::CGrBitmap() : m_hBitmap(NULL), m_lpbuffer(nullptr), m_iWidth(0), m_iHeight(0)
{
}

CGrBitmap::~CGrBitmap()
{
	Destroy();
}

bool CGrBitmap::Create(int iWidth, int iHeight)
{
	if(m_hBitmap && iWidth == m_iWidth && iHeight == m_iHeight){
		return true;
	}
	Destroy();
	if(iWidth <= 0 || iHeight <= 0){
		return false;
	}
	// HBITMAP CreateDIBSection(HDC hdc, CONST BITMAPINFO * pbmi, UINT iUsage,
	//                          VOID ** ppvBits, HANDLE hSection, DWORD dwOffset);
	// hdc      : 参照されるデバイスコンテキストのハンドルです。
	//            iUsageパラメータがDIB_PAL_COLORSのときに、hdcで定義されている論理パレットが
	//            作成されるＤＩＢのカラーテーブルにコピーされます。
	// pbmi     : ビットマップのヘッダ(BITMAPINFOHEADER)および、カラーテーブルを示すポインタです。
	//            このヘッダの部分に作成するＤＩＢの幅、高さ、カラーテーブルなどを代入してから、
	//            CreateDIBSection関数を呼び出します。
	// iUsage   : DIB_PAL_COLORSのとき、hdcの論理パレットが参照され、
	//            DIB_RGB_COLORSのとき、pbmiのカラーテーブルが参照されます。通常後者を選択します。
	// ppvBits  : 作成されるＤＩＢのビットデータの先頭を受け取るバッファを示します。
	//            CreateDIBSection関数でＤＩＢを作成する場合、BITMAPINFOHEADER構造体、
	//            カラーテーブルに、ビットデータが連続しないので、
	//            ＤＩＢのメモリにアクセスする場合は、この値を保存しておく必要があります。
	// hSection : ファイルマッピングを使用する場合にそのオブジェクトを示します。通常はNULLです。
	// dwOffset : ファイルマッピングを使うときの先頭からのオフセットです。使わない場合は０です。
	//
	// CreateDIBSection関数は、成功すると、作成されたビットマップオブジェクトのハンドルを返します。
	// このハンドルを、CreateCompatibleDC関数で作成したメモリＤＣに選択することで、
	// ＧＤＩ関数を使えるようになります。

	BITMAPINFOHEADER bminfo = {sizeof(BITMAPINFOHEADER)};

	bminfo.biWidth    = iWidth;
	bminfo.biHeight   = -iHeight;
	bminfo.biPlanes   = 1;
	bminfo.biBitCount = 32;
	// デバイスに依存しないビットマップ (DIB)の作成
	m_hBitmap = ::CreateDIBSection(NULL, reinterpret_cast<BITMAPINFO *>(&bminfo), DIB_RGB_COLORS, reinterpret_cast<VOID **>(&m_lpbuffer), NULL, 0);
	if(m_hBitmap){
		m_iWidth  = iWidth;
		m_iHeight = iHeight;
		return true;
	}
	return false;
}

void CGrBitmap::Destroy()
{
	if(m_hBitmap){
		::DeleteObject(m_hBitmap);
	}
	m_hBitmap = NULL;
	m_lpbuffer = nullptr;
	m_iWidth = 0;
	m_iHeight = 0;
}

void CGrBitmap::Clear()
{
	if(m_lpbuffer){
		::ZeroMemory(m_lpbuffer, m_iWidth*m_iHeight*sizeof(RGBQUAD));
	}
}


CGrMonoBitmap::CGrMonoBitmap() : m_hBitmap(NULL), m_lpbuffer(nullptr), m_iWidth(0), m_iHeight(0)
{
}

CGrMonoBitmap::~CGrMonoBitmap()
{
	Destroy();
}

bool CGrMonoBitmap::Create(int iWidth, int iHeight)
{
	if(m_hBitmap && iWidth == m_iWidth && iHeight == m_iHeight){
		return true;
	}
	Destroy();
	if(iWidth <= 0 || iHeight <= 0){
		return false;
	}
	// HBITMAP CreateDIBSection(HDC hdc, CONST BITMAPINFO * pbmi, UINT iUsage,
	//                          VOID ** ppvBits, HANDLE hSection, DWORD dwOffset);
	// hdc      : 参照されるデバイスコンテキストのハンドルです。
	//            iUsageパラメータがDIB_PAL_COLORSのときに、hdcで定義されている論理パレットが
	//            作成されるＤＩＢのカラーテーブルにコピーされます。
	// pbmi     : ビットマップのヘッダ(BITMAPINFOHEADER)および、カラーテーブルを示すポインタです。
	//            このヘッダの部分に作成するＤＩＢの幅、高さ、カラーテーブルなどを代入してから、
	//            CreateDIBSection関数を呼び出します。
	// iUsage   : DIB_PAL_COLORSのとき、hdcの論理パレットが参照され、
	//            DIB_RGB_COLORSのとき、pbmiのカラーテーブルが参照されます。通常後者を選択します。
	// ppvBits  : 作成されるＤＩＢのビットデータの先頭を受け取るバッファを示します。
	//            CreateDIBSection関数でＤＩＢを作成する場合、BITMAPINFOHEADER構造体、
	//            カラーテーブルに、ビットデータが連続しないので、
	//            ＤＩＢのメモリにアクセスする場合は、この値を保存しておく必要があります。
	// hSection : ファイルマッピングを使用する場合にそのオブジェクトを示します。通常はNULLです。
	// dwOffset : ファイルマッピングを使うときの先頭からのオフセットです。使わない場合は０です。
	//
	// CreateDIBSection関数は、成功すると、作成されたビットマップオブジェクトのハンドルを返します。
	// このハンドルを、CreateCompatibleDC関数で作成したメモリＤＣに選択することで、
	// ＧＤＩ関数を使えるようになります。

	BYTE bmpinfobuf[(sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 2)/sizeof(BYTE)] = {};
	auto lpbih = reinterpret_cast<LPBITMAPINFOHEADER>(bmpinfobuf);

	lpbih->biSize        = sizeof(BITMAPINFOHEADER);
	lpbih->biWidth       = iWidth;
	lpbih->biHeight      = -iHeight;
	lpbih->biPlanes      = 1;
	lpbih->biBitCount    = 1;
	lpbih->biCompression = BI_RGB;

	auto lpbi = reinterpret_cast<LPBITMAPINFO>(bmpinfobuf);
	RGBQUAD white = {};
	RGBQUAD black = {0xff,0xff,0xff,0x00};
	lpbi->bmiColors[0] = white;
	lpbi->bmiColors[1] = black;
	// デバイスに依存しないビットマップ (DIB)の作成
	m_hBitmap = ::CreateDIBSection(NULL, lpbi, DIB_RGB_COLORS, reinterpret_cast<VOID **>(&m_lpbuffer), NULL, 0);
	if(m_hBitmap){
		m_iWidth  = iWidth;
		m_iHeight = iHeight;
		return true;
	}
	return false;
}

void CGrMonoBitmap::Destroy()
{
	if(m_hBitmap){
		::DeleteObject(m_hBitmap);
	}
	m_hBitmap = NULL;
	m_lpbuffer = nullptr;
	m_iWidth = 0;
	m_iHeight = 0;
}

void CGrMonoBitmap::Clear()
{
	if(m_lpbuffer){
		::ZeroMemory(m_lpbuffer, m_iWidth*m_iHeight*sizeof(RGBQUAD));
	}
}
