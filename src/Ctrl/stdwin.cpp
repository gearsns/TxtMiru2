// stdwin.c
//
// ���C���E�C���h�E�쐬�o�^
//
#define STRICT // �^��錾����юg�p���ɁA��茵���Ȍ^�`�F�b�N���s���܂��B

#include <windows.h>
#include "stdwin.h"

CGrStdWin::CGrStdWin(HINSTANCE hInst)
{
	m_hInst = hInst;
	ole_initialized = OleInitialize(NULL);
}

CGrStdWin::~CGrStdWin()
{
	if(ole_initialized){
		OleUninitialize();
	}
}

HINSTANCE CGrStdWin::m_hInst = NULL;
HWND      CGrStdWin::m_hWnd  = NULL;

// �E�B���h�E�E�N���X�̓o�^
BOOL CGrStdWin::InitApp(WNDPROC fnWndProc, LPCTSTR szClassNm, UINT idi_app, UINT idr_menu)
{
	WNDCLASSEX wc = {sizeof(WNDCLASSEX)};

	wc.hCursor          = LoadCursor(NULL,IDI_APPLICATION);
	wc.hIcon            = static_cast<HICON>(LoadIcon(GetModuleHandle(NULL),MAKEINTRESOURCE(idi_app)));
	wc.hIconSm          = static_cast<HICON>(LoadImage(m_hInst,MAKEINTRESOURCE(idi_app),IMAGE_ICON,GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),LR_DEFAULTCOLOR));
	wc.lpszMenuName     = MAKEINTRESOURCE(idr_menu);
	wc.lpszClassName    = szClassNm;
	wc.hbrBackground    = reinterpret_cast<HBRUSH>(COLOR_WINDOW+1);
	wc.hInstance        = m_hInst;
	wc.style            = CS_HREDRAW | CS_VREDRAW;// | CS_DBLCLKS;
	wc.lpfnWndProc      = fnWndProc;

	return RegisterClassEx(&wc);
}

// �E�B���h�E�̐���
HWND CGrStdWin::InitInstance(int nCmdShow, LPCTSTR szClassName, LPCTSTR szAppName, LPWINDOWPLACEMENT lpwndp)
{
	int x      = 400;
	int y      = 100;
	UINT width  = CW_USEDEFAULT;
	UINT height = CW_USEDEFAULT;
	HWND hWnd;

	if(lpwndp){
		x      = lpwndp->rcNormalPosition.left;
		y      = lpwndp->rcNormalPosition.top;
		width  = lpwndp->rcNormalPosition.right  - lpwndp->rcNormalPosition.left;
		height = lpwndp->rcNormalPosition.bottom - lpwndp->rcNormalPosition.top;
	}
	hWnd = CreateWindow(szClassName,
						szAppName,                             // �^�C�g���o�[�ɂ��̖��O���\������܂�
						WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, // �E�B���h�E�̎��
						x,                                     // �w���W
						y,                                     // �x���W
						width,                                 // ��
						height,                                // ����
						NULL,                                  // �e�E�B���h�E�̃n���h���A�e�����Ƃ���NULL
						NULL,                                  // ���j���[�n���h���A�N���X���j���[���g���Ƃ���NULL
						m_hInst,                               // �C���X�^���X�n���h��
						NULL);
	if(!hWnd){
		return hWnd;
	}

	if(lpwndp){
		SetWindowPlacement(hWnd, lpwndp);
		nCmdShow = lpwndp->showCmd;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	m_hWnd = hWnd;
	return hWnd;
}
