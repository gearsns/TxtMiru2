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
	// hdc      : �Q�Ƃ����f�o�C�X�R���e�L�X�g�̃n���h���ł��B
	//            iUsage�p�����[�^��DIB_PAL_COLORS�̂Ƃ��ɁAhdc�Œ�`����Ă���_���p���b�g��
	//            �쐬�����c�h�a�̃J���[�e�[�u���ɃR�s�[����܂��B
	// pbmi     : �r�b�g�}�b�v�̃w�b�_(BITMAPINFOHEADER)����сA�J���[�e�[�u���������|�C���^�ł��B
	//            ���̃w�b�_�̕����ɍ쐬����c�h�a�̕��A�����A�J���[�e�[�u���Ȃǂ������Ă���A
	//            CreateDIBSection�֐����Ăяo���܂��B
	// iUsage   : DIB_PAL_COLORS�̂Ƃ��Ahdc�̘_���p���b�g���Q�Ƃ���A
	//            DIB_RGB_COLORS�̂Ƃ��Apbmi�̃J���[�e�[�u�����Q�Ƃ���܂��B�ʏ��҂�I�����܂��B
	// ppvBits  : �쐬�����c�h�a�̃r�b�g�f�[�^�̐擪���󂯎��o�b�t�@�������܂��B
	//            CreateDIBSection�֐��łc�h�a���쐬����ꍇ�ABITMAPINFOHEADER�\���́A
	//            �J���[�e�[�u���ɁA�r�b�g�f�[�^���A�����Ȃ��̂ŁA
	//            �c�h�a�̃������ɃA�N�Z�X����ꍇ�́A���̒l��ۑ����Ă����K�v������܂��B
	// hSection : �t�@�C���}�b�s���O���g�p����ꍇ�ɂ��̃I�u�W�F�N�g�������܂��B�ʏ��NULL�ł��B
	// dwOffset : �t�@�C���}�b�s���O���g���Ƃ��̐擪����̃I�t�Z�b�g�ł��B�g��Ȃ��ꍇ�͂O�ł��B
	//
	// CreateDIBSection�֐��́A��������ƁA�쐬���ꂽ�r�b�g�}�b�v�I�u�W�F�N�g�̃n���h����Ԃ��܂��B
	// ���̃n���h�����ACreateCompatibleDC�֐��ō쐬�����������c�b�ɑI�����邱�ƂŁA
	// �f�c�h�֐����g����悤�ɂȂ�܂��B

	BITMAPINFOHEADER bminfo = {sizeof(BITMAPINFOHEADER)};

	bminfo.biWidth    = iWidth;
	bminfo.biHeight   = -iHeight;
	bminfo.biPlanes   = 1;
	bminfo.biBitCount = 32;
	// �f�o�C�X�Ɉˑ����Ȃ��r�b�g�}�b�v (DIB)�̍쐬
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
	// hdc      : �Q�Ƃ����f�o�C�X�R���e�L�X�g�̃n���h���ł��B
	//            iUsage�p�����[�^��DIB_PAL_COLORS�̂Ƃ��ɁAhdc�Œ�`����Ă���_���p���b�g��
	//            �쐬�����c�h�a�̃J���[�e�[�u���ɃR�s�[����܂��B
	// pbmi     : �r�b�g�}�b�v�̃w�b�_(BITMAPINFOHEADER)����сA�J���[�e�[�u���������|�C���^�ł��B
	//            ���̃w�b�_�̕����ɍ쐬����c�h�a�̕��A�����A�J���[�e�[�u���Ȃǂ������Ă���A
	//            CreateDIBSection�֐����Ăяo���܂��B
	// iUsage   : DIB_PAL_COLORS�̂Ƃ��Ahdc�̘_���p���b�g���Q�Ƃ���A
	//            DIB_RGB_COLORS�̂Ƃ��Apbmi�̃J���[�e�[�u�����Q�Ƃ���܂��B�ʏ��҂�I�����܂��B
	// ppvBits  : �쐬�����c�h�a�̃r�b�g�f�[�^�̐擪���󂯎��o�b�t�@�������܂��B
	//            CreateDIBSection�֐��łc�h�a���쐬����ꍇ�ABITMAPINFOHEADER�\���́A
	//            �J���[�e�[�u���ɁA�r�b�g�f�[�^���A�����Ȃ��̂ŁA
	//            �c�h�a�̃������ɃA�N�Z�X����ꍇ�́A���̒l��ۑ����Ă����K�v������܂��B
	// hSection : �t�@�C���}�b�s���O���g�p����ꍇ�ɂ��̃I�u�W�F�N�g�������܂��B�ʏ��NULL�ł��B
	// dwOffset : �t�@�C���}�b�s���O���g���Ƃ��̐擪����̃I�t�Z�b�g�ł��B�g��Ȃ��ꍇ�͂O�ł��B
	//
	// CreateDIBSection�֐��́A��������ƁA�쐬���ꂽ�r�b�g�}�b�v�I�u�W�F�N�g�̃n���h����Ԃ��܂��B
	// ���̃n���h�����ACreateCompatibleDC�֐��ō쐬�����������c�b�ɑI�����邱�ƂŁA
	// �f�c�h�֐����g����悤�ɂȂ�܂��B

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
	// �f�o�C�X�Ɉˑ����Ȃ��r�b�g�}�b�v (DIB)�̍쐬
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
