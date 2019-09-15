#ifndef __TXTBOOKMARK_H__
#define __TXTBOOKMARK_H__
// �ȉ��� ifdef �u���b�N�� DLL ����̃G�N�X�|�[�g��e�Ղɂ���}�N�����쐬���邽�߂�
// ��ʓI�ȕ��@�ł��B���� DLL ���̂��ׂẴt�@�C���́A�R�}���h ���C���Œ�`���ꂽ TXTFUNCBOOKMARK_EXPORTS
// �V���{���ŃR���p�C������܂��B���̃V���{���́A���� DLL ���g���v���W�F�N�g�Œ�`���邱�Ƃ͂ł��܂���B
// �\�[�X�t�@�C�������̃t�@�C�����܂�ł��鑼�̃v���W�F�N�g�́A
// TXTFUNCBOOKMARK_API �֐��� DLL ����C���|�[�g���ꂽ�ƌ��Ȃ��̂ɑ΂��A���� DLL �́A���̃}�N���Œ�`���ꂽ
// �V���{�����G�N�X�|�[�g���ꂽ�ƌ��Ȃ��܂��B
#ifdef TXTFUNCBOOKMARK_EXPORTS
#define TXTFUNCBOOKMARK_API __declspec(dllexport)
#else
#define TXTFUNCBOOKMARK_API __declspec(dllimport)
#endif

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "stltchar.h"
#include "TxtFuncIParam.h"
#include "TxtFuncIBookmark.h"

extern "C" {
TXTFUNCBOOKMARK_API bool cdecl TxtFuncBookmarkSaveAs(LPCTSTR lpFileName, CGrTxtFuncIBookmark *pBookmark, CGrTxtFuncISubTitle *pSubTitle, LPCTSTR lpDataDir, CGrTxtFuncIParam *pParam);
TXTFUNCBOOKMARK_API bool cdecl TxtFuncBookmarkOpen(LPCTSTR lpFileName, CGrTxtFuncIBookmark *pBookmark, CGrTxtFuncISubTitle *pSubTitle, LPCTSTR lpDataDir, CGrTxtFuncIParam *pParam);
TXTFUNCBOOKMARK_API bool cdecl TxtFuncShowBookList(HWND hWnd, LPCTSTR lpFileName, CGrTxtFuncIBookmark *pBookmark, LPCTSTR lpDataDir, CGrTxtFuncIParam *pParam);
TXTFUNCBOOKMARK_API bool cdecl TxtFuncShowLinkList(HWND hWnd, int mode, CGrTxtFuncIBookmark *pBookmark, CGrTxtFuncISubTitle *pSubTitle, LPCTSTR lpDataDir, CGrTxtFuncIParam *pParam);
TXTFUNCBOOKMARK_API bool cdecl TxtFuncRefreshBookList();
TXTFUNCBOOKMARK_API bool cdecl TxtFuncIsShowBookList();
TXTFUNCBOOKMARK_API bool cdecl TxtFuncIsBookListMember(HWND hWnd);
TXTFUNCBOOKMARK_API bool cdecl TxtFuncRefreshLinkList();
TXTFUNCBOOKMARK_API bool cdecl TxtFuncIsShowLinkList();
TXTFUNCBOOKMARK_API bool cdecl TxtFuncIsLinkListMember(HWND hWnd);
TXTFUNCBOOKMARK_API bool cdecl TxtFuncUpdatePage();
TXTFUNCBOOKMARK_API HWND cdecl TxtFuncGetWnd(int id);
TXTFUNCBOOKMARK_API void cdecl TxtFuncUnInstall();
TXTFUNCBOOKMARK_API bool cdecl TxtFuncSearchFiles(HWND hWnd, LPCTSTR lpDataDir, CGrTxtFuncIParam *pParam);
TXTFUNCBOOKMARK_API bool cdecl TxtFuncShowRubyList(HWND hWnd, void *pDoc, CGrTxtFuncIBookmark *pBookmark, LPCTSTR lpDataDir, CGrTxtFuncIParam *pParam);
TXTFUNCBOOKMARK_API bool cdecl TxtFuncIsShowRubyList();
TXTFUNCBOOKMARK_API bool cdecl TxtFuncRefreshRubyList();
TXTFUNCBOOKMARK_API int cdecl TxtFuncAddFavorite(HWND hWnd, LPCTSTR lpURL);
};
#endif // __TXTBOOKMARK_H__
