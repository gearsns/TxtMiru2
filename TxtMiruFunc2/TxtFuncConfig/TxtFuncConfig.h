// �ȉ��� ifdef �u���b�N�� DLL ����̃G�N�X�|�[�g��e�Ղɂ���}�N�����쐬���邽�߂�
// ��ʓI�ȕ��@�ł��B���� DLL ���̂��ׂẴt�@�C���́A�R�}���h ���C���Œ�`���ꂽ TXTFUNCCONFIG_EXPORTS
// �V���{���ŃR���p�C������܂��B���̃V���{���́A���� DLL ���g���v���W�F�N�g�Œ�`���邱�Ƃ͂ł��܂���B
// �\�[�X�t�@�C�������̃t�@�C�����܂�ł��鑼�̃v���W�F�N�g�́A
// TXTFUNCCONFIG_API �֐��� DLL ����C���|�[�g���ꂽ�ƌ��Ȃ��̂ɑ΂��A���� DLL �́A���̃}�N���Œ�`���ꂽ
// �V���{�����G�N�X�|�[�g���ꂽ�ƌ��Ȃ��܂��B
#ifdef TXTFUNCCONFIG_EXPORTS
#define TXTFUNCCONFIG_API __declspec(dllexport)
#else
#define TXTFUNCCONFIG_API __declspec(dllimport)
#endif
#include "TxtFuncIParam.h"
extern "C" {
TXTFUNCCONFIG_API bool cdecl TxtFuncConfig(HWND hWnd, LPCTSTR lpkeyBindFileName, LPCTSTR lpDataDir, CGrTxtFuncIParam *pParam);
TXTFUNCCONFIG_API bool cdecl TxtFuncLayoutConfig(HWND hWnd, LPCTSTR lpDataDir, LPCTSTR type, LPCTSTR name, LPCTSTR lpFileName, CGrTxtFuncIParam *pParam);
TXTFUNCCONFIG_API bool cdecl TxtFuncGetBrowserURL(LPTSTR *ppURL, CGrTxtFuncIParam *pParam);
TXTFUNCCONFIG_API bool cdecl TxtFuncGotoPageDlg(HWND hWnd, int maxnum, int cur_page, int *out_page);
TXTFUNCCONFIG_API bool cdecl TxtFuncDocInfoDlg(HWND hWnd, LPCTSTR title, LPCTSTR author, LPCTSTR filename, LPCTSTR text, LPCTSTR writetime, int page, int maxpage);
TXTFUNCCONFIG_API bool cdecl TxtFuncVersionInfoDlg(HWND hWnd, HINSTANCE hInst, CGrTxtFuncIParam *pParam);
};
