#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <shlobj.h>
#include <filesystem>
#include "resource.h"
#include "ImportDlg.h"
#include "TxtMiruTheme.h"
#include "Text.h"
#include "Shell.h"
#include "BookMarkDB.h"
#include "TxtMiru.h"
#include "MessageBox.h"

#define TXTMIRU_APP_NAME _T("TxtMiru2.0")

static int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	if (uMsg == BFFM_INITIALIZED) {
		::SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
	}
	return 0;
}

bool BrowseForFolder(HWND hwnd, UINT id, UINT mes_id)
{
	TCHAR fileName[MAX_PATH + 1];
	GetDlgItemText(hwnd, id, fileName, sizeof(fileName) / sizeof(TCHAR));

	BROWSEINFO bi = {};
	std::tstring str;
	CGrText::LoadString(mes_id, str);
	bi.hwndOwner = hwnd;
	bi.pszDisplayName = fileName;
	bi.lpszTitle = str.c_str();
	bi.ulFlags = BIF_RETURNONLYFSDIRS;
	bi.lpfn = &BrowseCallbackProc;
	bi.lParam = reinterpret_cast<LPARAM>(fileName);
	auto pidl = ::SHBrowseForFolder(&bi);
	if (pidl) {
		::SHGetPathFromIDList(pidl, fileName);
		::CoTaskMemFree(pidl);
		SetDlgItemText(hwnd, id, fileName);
		return true;
	}
	return false;
}


CGrImportDlg::CGrImportDlg()
{
}


CGrImportDlg::~CGrImportDlg()
{
}


int CGrImportDlg::DoModal(HINSTANCE hinstance, HWND hWnd)
{
	return DialogBoxParam(hinstance, MAKEINTRESOURCE(IDD_IMPORT), hWnd, (DLGPROC)CGrWinCtrl::WindowMapProc, (LPARAM)this);
}

LRESULT CGrImportDlg::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
		HANDLE_MSG(hWnd, WM_INITDIALOG, OnInitDialog);
		HANDLE_MSG(hWnd, WM_COMMAND, OnCommand);
	}
	return CGrWinCtrl::WndProc(hWnd, uMsg, wParam, lParam);
}

BOOL CGrImportDlg::OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	SetWindowPosCenter();

	m_EditURL.SetWnd(GetDlgItem(hwnd, IDC_EDIT_URL));
	m_EditFolder.SetWnd(GetDlgItem(hwnd, IDC_EDIT_FOLDER));
	//
	TxtMiruTheme_SetWindowSubclass(m_hWnd);

	return TRUE;
}

void CGrImportDlg::OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	switch (id) {
	case IDC_BUTTON_FOLDER:
	{
		if (BrowseForFolder(hwnd, IDC_EDIT_FOLDER, IDS_IMPORT_FOLDER)) {
		}
	}
		break;
	case IDOK:
	{
		m_EditFolder.GetText(m_folder);
		if (m_folder.empty()) {
			SetFocus(m_EditFolder);
			// Error
			CGrMessageBox::Show(m_hWnd, IDS_ERROR_IMPORT_FOLDER, TXTMIRU_APP_NAME);
			break;
		}
		m_EditURL.GetText(m_url);
		if (m_url.empty() && !CGrShell::IsURI(m_url.c_str())) {
			SetFocus(m_EditURL);
			// Error
			CGrMessageBox::Show(m_hWnd, IDS_ERROR_IMPORT_URL, TXTMIRU_APP_NAME);
			break;
		}
		CGrShell::ToPrettyFileName(m_url);
		if (!CGrShell::EndBackSlash(const_cast<TCHAR*>(m_url.c_str()))) {
			m_url += _T("/");
		}
		CGrShell::ToPrettyFileName(m_folder);
		if (!CGrShell::EndBackSlash(const_cast<TCHAR*>(m_folder.c_str()))) {
			m_folder += _T("/");
		}
		//
		bool bCopy = true;
		std::tstring cache_dir(CGrTxtMiru::GetDataPath());
		if (!CGrShell::EndBackSlash(const_cast<TCHAR*>(cache_dir.c_str()))) {
			cache_dir += _T("/");
		}
		cache_dir += _T("Cache/");
		CGrShell::ToPrettyFileName(cache_dir);
		if (0 == _tcsnicmp(cache_dir.c_str(), m_folder.c_str(), cache_dir.size())) {
			bCopy = false;
		}
		std::tstring folder;
		if (bCopy) {
			std::tstring sysdate;
			time_t now;

			time(&now);
			struct tm ltm = {};
			if (0 == localtime_s(&ltm, &now)) {
				TCHAR date_str[512] = {};
				_tcsftime(date_str, sizeof(date_str) / sizeof(TCHAR), _T("%Y%m%d%H%M%S"), &ltm);
				sysdate = date_str;
			}
			else {
				// Error
				CGrMessageBox::Show(m_hWnd, IDS_ERROR_IMPORT_DATE, TXTMIRU_APP_NAME);
				break;
			}
			folder = cache_dir + _T("Import/") + sysdate + _T("/");
			try {
				std::filesystem::path from(m_folder);
				std::filesystem::path to(folder);
				from.make_preferred();
				to.make_preferred();
				std::filesystem::create_directories(to);
				std::filesystem::copy(from, to, std::filesystem::copy_options::recursive);
			}
			catch (std::filesystem::filesystem_error& err) {
				CGrMessageBox::Show(m_hWnd, IDS_ERROR_IMPORT_COPY, TXTMIRU_APP_NAME);
				break;
			}
			catch (std::error_code& err) {
				CGrMessageBox::Show(m_hWnd, IDS_ERROR_IMPORT_COPY, TXTMIRU_APP_NAME);
				break;
			}
			catch (...) {
				CGrMessageBox::Show(m_hWnd, IDS_ERROR_IMPORT_COPY, TXTMIRU_APP_NAME);
				break;
			}
		}
		else {
			folder = m_folder;
		}
		//
		std::vector<std::tstring> ftarget_list;
		try {
			for (const auto f : std::filesystem::recursive_directory_iterator(folder.c_str())) {
				auto filename = f.path().wstring();
				CGrShell::ToPrettyFileName(filename);
				ftarget_list.push_back(filename);
			}
		} catch(...) {
			CGrMessageBox::Show(m_hWnd, IDS_ERROR_IMPORT_FILELIST, TXTMIRU_APP_NAME);
			break;
		}
		if (ftarget_list.empty()) {
			// Error
			CGrMessageBox::Show(m_hWnd, IDS_ERROR_IMPORT_FILELIST, TXTMIRU_APP_NAME);
			break;
		}
		CGrDBCache db;
		if (db.Open()) {
			CacheItem item;
			db.BeginSession();
			std::tstring datapath = CGrTxtMiru::GetDataPath();
			CGrShell::ToPrettyFileName(datapath);
			{
				std::tstring sysdate;
				CGrDBFunc::GetSysDate(sysdate);
				sysdate = _T("Import ") + sysdate;
				std::tstring filename(folder);
				if (0 == CGrText::niCmp(datapath.c_str(), filename.c_str(), datapath.size())) {
					filename.replace(0, datapath.size(), L"%DATADIR%");
				}
				db.Put(m_url.c_str(), filename.c_str(), sysdate.c_str(), L"");
				db.Get(item, m_url.c_str());
			}
			for (const auto& fullpath : ftarget_list) {
				std::tstring url(m_url);
				url += CGrShell::RemovePath(fullpath.c_str(), folder.c_str());
				CGrShell::ToPrettyFileName(url);
				std::tstring filename(fullpath);
				if (0 == CGrText::niCmp(datapath.c_str(), filename.c_str(), datapath.size())) {
					filename.replace(0, datapath.size(), L"%DATADIR%");
				}
				db.Put(item, url.c_str(), filename.c_str());
			}
			db.Commit();
			CGrMessageBox::Show(m_hWnd, IDS_SUCCESS_IMPORT, TXTMIRU_APP_NAME);
		}
		else {
			CGrMessageBox::Show(m_hWnd, IDS_ERROR_IMPORT_OPENDB, TXTMIRU_APP_NAME);
			break;
		}
	}
	break;
	case IDCANCEL:
	{
		::EndDialog(m_hWnd, IDCANCEL);
	}
	break;
	}
}
