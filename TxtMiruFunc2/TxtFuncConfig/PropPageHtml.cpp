#pragma warning( disable : 4786 )

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "resource.h"
#include "PropPageHtml.h"
#include "TxtConfigDlg.h"
#include "Text.h"
#include "Shell.h"
#include "CurrentDirectory.h"
#include "TxtFuncConfig.h"
#include "TxtFunc.h"
#include "TxtConfigFunc.h"
#include "MessageBox.h"

static struct PointSet l_html_point_set[] = {
	{IDC_EDIT_SKIPIMAGESIZE_WIDTH , CGrTxtFuncIParam::PointsType::SkipHTMLImgSize },
	{IDC_EDIT_SKIPIMAGESIZE_HEIGHT, CGrTxtFuncIParam::PointsType::SkipHTMLImgSize },
};
static struct PointRange l_html_point_range[] = {
	{IDC_SPIN_SKIPIMAGESIZE_WIDTH , 0, 1000, 0                         },
	{IDC_SPIN_SKIPIMAGESIZE_HEIGHT, 0, 1000, 0                         },
};

CGrPropPage* PASCAL CGrPropPageHtml::CreateProp(CGrTxtConfigDlg *pDlg)
{
	auto *pPage = new CGrPropPageHtml(pDlg);
	pPage->m_hWnd = CreateDialogParam(CGrTxtFunc::GetDllModuleHandle(), MAKEINTRESOURCE(IDD_PROPPAGE_HTML), pDlg->GetWnd(), reinterpret_cast<DLGPROC>(CGrWinCtrl::WindowMapProc), reinterpret_cast<LPARAM>(pPage));
	pPage->Attach(pPage->m_hWnd);
	return pPage;
}

CGrPropPageHtml::CGrPropPageHtml(CGrTxtConfigDlg *pDlg) : CGrPropPageSize(pDlg)
{
}

CGrPropPageHtml::~CGrPropPageHtml()
{
}

// ���C���E�B���h�E
LRESULT CGrPropPageHtml::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg){
		HANDLE_MSG(hWnd, WM_INITDIALOG, OnInitDialog );
		HANDLE_MSG(hWnd, WM_NOTIFY    , OnNotify     );
		HANDLE_MSG(hWnd, WM_COMMAND   , OnCommand    );
	case WM_SETPAGEFOCUS:
		::SetFocus((HWND)wParam);
		break;
	}
	return CGrPropPageSize::WndProc(hWnd, uMsg, wParam, lParam);
}

BOOL CGrPropPageHtml::OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	CGrPropPageSize::OnInitDialog(hwnd, hwndFocus, lParam);
	auto &&param = CGrTxtFunc::Param();
	::SetDlgItemPointSet(hwnd, l_html_point_set, sizeof(l_html_point_set)/sizeof(PointSet));
	::SetDlgItemPointRange(hwnd, l_html_point_range, sizeof(l_html_point_range)/sizeof(PointRange));
	// DDE
	::SetDlgItemTextType(hwnd, param, IDC_EDIT_BROWSER_LIST, CGrTxtFuncIParam::TextType::BrowserAppName);
	// IE�̐ݒ���g�p����
	bool bUseIESetting = param.GetBoolean(CGrTxtFuncIParam::PointsType::UseIESetting);
	::SetCheckDlgItemID(hwnd, IDC_CHECKBOX_IESETTING, bUseIESetting);
	// ���ڐڑ�(Proxy���g�p���Ȃ�)
	bool bUseProxy = param.GetBoolean(CGrTxtFuncIParam::PointsType::UseProxy);
	::SetCheckDlgItemID(hwnd, IDC_RADIO_USEPROXY, bUseProxy );
	::SetCheckDlgItemID(hwnd, IDC_RADIO_NOPROXY , !bUseProxy);
	// �g�p����v���L�V�̃A�h���X
	::SetDlgItemTextType(hwnd, param, IDC_EDIT_PROXY, CGrTxtFuncIParam::TextType::ProxyServer);
	// ��O
	::SetDlgItemTextType(hwnd, param, IDC_EDIT_NOPROXY, CGrTxtFuncIParam::TextType::NoProxyAddress);

	if(bUseIESetting){
		// IE�̐ݒ���g�p����ꍇ�́A�Ǝ��̐ڑ��ݒ�͕ҏW�s��
		::EnableDlgItemID(hwnd, IDC_RADIO_NOPROXY , FALSE);
		::EnableDlgItemID(hwnd, IDC_RADIO_USEPROXY, FALSE);
		::EnableDlgItemID(hwnd, IDC_EDIT_PROXY    , FALSE);
		::EnableDlgItemID(hwnd, IDC_EDIT_NOPROXY  , FALSE);
	} else if(!bUseProxy){
		// �v���L�V���g�p���Ȃ��ꍇ�́A�v���L�V�ݒ�͕ҏW�s��
		::EnableDlgItemID(hwnd, IDC_EDIT_PROXY    , FALSE);
		::EnableDlgItemID(hwnd, IDC_EDIT_NOPROXY  , FALSE);
	}
	int ieOptions[2] = {};
	param.GetPoints(CGrTxtFuncIParam::PointsType::IEOption, ieOptions, sizeof(ieOptions)/sizeof(int));
	::SetCheckDlgItemID(hwnd, IDC_CHECKBOX_DLCTL_SILENT    , (ieOptions[0] == 1)); // �_�C�A���O��\�����Ȃ�
	::SetCheckDlgItemID(hwnd, IDC_CHECKBOX_DLCTL_NO_SCRIPTS, (ieOptions[1] == 1)); // �X�N���v�g�����s���Ȃ�
	int iWebFilter[2] = {};
	param.GetPoints(CGrTxtFuncIParam::PointsType::WebFilter, iWebFilter, sizeof(iWebFilter)/sizeof(int));
	::SetCheckDlgItemID(hwnd, IDC_CHECKBOX_FILTER, (iWebFilter[0] == 1)); // URL�̃u���b�N��L����
	::SetCheckDlgItemID(hwnd, IDC_CHECKBOX_CACHE, (iWebFilter[1] == 1)); // �L���b�V������̓ǂݍ��݂�L����

	//
	return TRUE;
}

void CGrPropPageHtml::OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	switch(id){
	case IDC_EDIT_SKIPIMAGESIZE_WIDTH :
	case IDC_EDIT_SKIPIMAGESIZE_HEIGHT:
	case IDC_EDIT_BROWSER_LIST        :
	case IDC_EDIT_PROXY               :
	case IDC_EDIT_NOPROXY             :
		if(codeNotify == EN_CHANGE){
			PropSheet_Changed(::GetParent(hwnd), hwnd);
		}
		break;
	case IDC_CHECKBOX_IESETTING:
		if(::GetCheckDlgItemID(m_hWnd, IDC_CHECKBOX_IESETTING) == BST_UNCHECKED){
			::EnableDlgItemID(m_hWnd, IDC_RADIO_NOPROXY , TRUE);
			::EnableDlgItemID(m_hWnd, IDC_RADIO_USEPROXY, TRUE);
			// �v���L�V���g�p���Ȃ��ꍇ�́A�v���L�V�ݒ�͕ҏW�s��
			::EnableCheckDlgItemID(m_hWnd, IDC_RADIO_USEPROXY, IDC_EDIT_PROXY, IDC_EDIT_NOPROXY);
		} else {
			// IE�̐ݒ���g�p����ꍇ�́A�Ǝ��̐ڑ��ݒ�͕ҏW�s��
			::EnableDlgItemID(m_hWnd, IDC_RADIO_NOPROXY , FALSE);
			::EnableDlgItemID(m_hWnd, IDC_RADIO_USEPROXY, FALSE);
			::EnableDlgItemID(m_hWnd, IDC_EDIT_PROXY    , FALSE);
			::EnableDlgItemID(m_hWnd, IDC_EDIT_NOPROXY  , FALSE);
		}
		PropSheet_Changed(::GetParent(hwnd), hwnd);
		break;
	case IDC_CHECKBOX_DLCTL_SILENT    : // �_�C�A���O��\�����Ȃ�
	case IDC_CHECKBOX_DLCTL_NO_SCRIPTS: // �X�N���v�g�����s���Ȃ�
	case IDC_CHECKBOX_FILTER: // URL�̃u���b�N��L����
	case IDC_CHECKBOX_CACHE: // �L���b�V������̓ǂݍ��݂�L����
		PropSheet_Changed(::GetParent(hwnd), hwnd);
		break;
	case IDC_RADIO_NOPROXY :
	case IDC_RADIO_USEPROXY:
		// �v���L�V���g�p���Ȃ��ꍇ�́A�v���L�V�ݒ�͕ҏW�s��
		::EnableCheckDlgItemID(m_hWnd, IDC_RADIO_USEPROXY, IDC_EDIT_PROXY, IDC_EDIT_NOPROXY);
		PropSheet_Changed(::GetParent(hwnd), hwnd);
		break;
	case ID_SHOW_INI_FOLDER:
		{
			CGrShell::Execute(m_hWnd, CGrTxtFunc::GetDataPath());
		}
		break;
	case ID_SHOW_FILTERTEXT: // �u���b�N����URL��ݒ�
		{
			TCHAR block_file[512];
			_stprintf_s(block_file, _T("%s/urlblock.txt"), CGrTxtFunc::GetDataPath());
			{
				WIN32_FIND_DATA wfd;
				if(!CGrShell::getFileInfo(block_file, wfd) || (wfd.nFileSizeHigh == 0 && wfd.nFileSizeLow == 0)){
					auto hFile = ::CreateFile(block_file, GENERIC_WRITE, FILE_SHARE_WRITE, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
					if(hFile != INVALID_HANDLE_VALUE){
						DWORD dwNumberOfBytesWritten;
						TCHAR szBuff[] = _T("\xFEFF#��\r\n\
#https*://kakuyomu\\.jp.*,.*\\.(:?js|svg)\r\n\
#https*://kakuyomu\\.jp.*,kakuyomu\\-logo\\.png\r\n\
#https*://ncode\\.syosetu\\.com,(:?google|microad|tugikuru|accaii|alphapolis)\r\n\
#about:,.*\\.(:?gif|png|jpeg|jpg|js|svg).*\r\n\
#about:,cont_access\\.php\r\n\
#about:,www\\.tugikuru\\.jp\\/colink\r\n");
						WriteFile(hFile, szBuff, sizeof(szBuff), &dwNumberOfBytesWritten, nullptr);
					}
					::CloseHandle(hFile);
				}
			}
			CGrShell::Execute(m_hWnd, block_file);
		}
		break;
	}
}

bool CGrPropPageHtml::Apply()
{
	auto &&param = CGrTxtFunc::Param();
	// �y�[�W�ݒ� NxN
	::GetDlgItemPointSet(m_hWnd, l_html_point_set, sizeof(l_html_point_set)/sizeof(PointSet));
	// DDE
	::GetDlgItemTextType(m_hWnd, param, IDC_EDIT_BROWSER_LIST, CGrTxtFuncIParam::TextType::BrowserAppName);
	// IE�̐ݒ���g�p����
	param.SetBoolean(CGrTxtFuncIParam::PointsType::UseIESetting, ::GetCheckDlgItemID(m_hWnd, IDC_CHECKBOX_IESETTING) == BST_CHECKED);
	// ���ڐڑ�(Proxy���g�p���Ȃ�)
	param.SetBoolean(CGrTxtFuncIParam::PointsType::UseProxy, ::GetCheckDlgItemID(m_hWnd, IDC_RADIO_USEPROXY) == BST_CHECKED);
	// �g�p����v���L�V�̃A�h���X
	::GetDlgItemTextType(m_hWnd, param, IDC_EDIT_PROXY, CGrTxtFuncIParam::TextType::ProxyServer);
	// ��O
	::GetDlgItemTextType(m_hWnd, param, IDC_EDIT_NOPROXY, CGrTxtFuncIParam::TextType::NoProxyAddress);
	int ieOptions[2] = {};
	ieOptions[0] = (::GetCheckDlgItemID(m_hWnd, IDC_CHECKBOX_DLCTL_SILENT    ) == BST_CHECKED) ? 1 : 0; // �_�C�A���O��\�����Ȃ�
	ieOptions[1] = (::GetCheckDlgItemID(m_hWnd, IDC_CHECKBOX_DLCTL_NO_SCRIPTS) == BST_CHECKED) ? 1 : 0; // �X�N���v�g�����s���Ȃ�
	param.SetPoints(CGrTxtFuncIParam::PointsType::IEOption, ieOptions, sizeof(ieOptions)/sizeof(int));
	int iWebFilter[2] = {};
	iWebFilter[0] = (::GetCheckDlgItemID(m_hWnd, IDC_CHECKBOX_FILTER) == BST_CHECKED) ? 1 : 0; // URL�̃u���b�N��L����
	iWebFilter[1] = (::GetCheckDlgItemID(m_hWnd, IDC_CHECKBOX_CACHE) == BST_CHECKED) ? 1 : 0; // �L���b�V������̓ǂݍ��݂�L����
	param.SetPoints(CGrTxtFuncIParam::PointsType::WebFilter, iWebFilter, sizeof(iWebFilter)/sizeof(int));
	//
	param.UpdateConfig(GetParent(GetParent(m_hWnd)));
	//
	return true;
}
