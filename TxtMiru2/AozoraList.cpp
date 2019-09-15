#pragma warning( disable : 4786 )

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "resource.h"
#include "AozoraList.h"

#define TXTMIRU_AOZORA_EXE       _T("TxtMiruDBAozora")
#define TXTMIRU_AOZORA_PROP_NAME _T("TxtMiru2.0-Aozora")

struct TxtMiruData
{
	HWND hWnd;
};
static BOOL CALLBACK EnumWindowsProc(
	HWND hwnd,      // �e�E�B���h�E�̃n���h��
	LPARAM lParam   // �A�v���P�[�V������`�̒l
	)
{
	auto handle = GetProp(hwnd, TXTMIRU_AOZORA_PROP_NAME);
	if(!handle){
		return TRUE; // �p��
	}
	auto *lpData = reinterpret_cast<TxtMiruData*>(lParam);
	lpData->hWnd = hwnd;
	return FALSE;
}

void CloseAozoraList(HWND hWnd)
{
	TxtMiruData data = {};
	EnumWindows(EnumWindowsProc, LPARAM(&data));
	if(data.hWnd){
		PostMessage(data.hWnd, WM_CLOSE, 0, 0);
	}
}
void showAozoraList(HWND hWnd, LPCTSTR lpsCmdLine)
{
	TxtMiruData data = {};
	EnumWindows(EnumWindowsProc, LPARAM(&data));
	if(data.hWnd){
		// ForceForegroundWindow
		auto dwForceThread   = GetWindowThreadProcessId(data.hWnd, nullptr);
		auto dwCurrentThread = GetCurrentThreadId();
		if(dwForceThread == dwCurrentThread){
			BringWindowToTop(data.hWnd);
		} else {
			AttachThreadInput(dwForceThread, dwCurrentThread, true);
			BringWindowToTop(data.hWnd);
			AttachThreadInput(dwForceThread, dwCurrentThread, false);
		}
	} else {
		PROCESS_INFORMATION pi = {};
		STARTUPINFO si = {sizeof(STARTUPINFO)};

		auto len = _tcslen(TXTMIRU_AOZORA_EXE) + 1 + _tcslen(lpsCmdLine) + 1;
		auto lpTxtMiruCmdLine = static_cast<LPTSTR>(_malloca((len+1) * sizeof(TCHAR)));
		if (lpTxtMiruCmdLine) {
			_stprintf_s(lpTxtMiruCmdLine, len/* * sizeof(TCHAR)*/, _T("%s %s"), TXTMIRU_AOZORA_EXE, lpsCmdLine);
			CreateProcess(
				nullptr, // LPCTSTR lpApplicationName                , // ���s�\���W���[���̖��O
				lpTxtMiruCmdLine, // LPTSTR lpCommandLine                     , // �R�}���h���C���̕�����
				nullptr, // LPSECURITY_ATTRIBUTES lpProcessAttributes, // �Z�L�����e�B�L�q�q
				nullptr, // LPSECURITY_ATTRIBUTES lpThreadAttributes , // �Z�L�����e�B�L�q�q
				FALSE, // BOOL bInheritHandles                     , // �n���h���̌p���I�v�V����
				0, // DWORD dwCreationFlags                    , // �쐬�̃t���O
				nullptr, // LPVOID lpEnvironment                     , // �V�������u���b�N
				nullptr, // LPCTSTR lpCurrentDirectory               , // �J�����g�f�B���N�g���̖��O
				&si, // LPSTARTUPINFO lpStartupInfo              , // �X�^�[�g�A�b�v���
				&pi               // LPPROCESS_INFORMATION lpProcessInformation // �v���Z�X���
			);
			if (pi.hThread) {
				CloseHandle(pi.hThread);
			}
			if (pi.hProcess) {
				CloseHandle(pi.hProcess);
			}
		}
	}
}

