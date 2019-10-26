#pragma warning( disable : 4786 )

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "resource.h"
#include "PropPageOption.h"
#include "TxtConfigDlg.h"
#include "Text.h"
#include "Shell.h"
#include "CurrentDirectory.h"
#include "TxtFuncConfig.h"
#include "TxtFunc.h"
#include "TxtConfigFunc.h"
#include "MessageBox.h"
#include "CSVText.h"

static struct PointSet l_txt_point_set[] = {
	{IDC_EDIT_MAXARCFILESIZE      , CGrTxtFuncIParam::PointsType::ArcMaxFileSize  },
};
static struct PointRange l_txt_point_range[] = {
	{IDC_SPIN_MAXARCFILESIZE      , 0, 1000, 0                         },
	{IDC_SPIN_USEPREPARSERSIZE    , 0, 1000, 0                         },
};

void loadMenuName(std::map<std::tstring, std::tstring> &menu_list)
{
	std::tstring filename = _T("menu_name.lis");
	auto lpDataPath = CGrTxtFunc::GetDataPath();
	if (lpDataPath) {
		CGrShell::ToPrettyFileName(lpDataPath, filename);
		filename += _T("/menu_name.lis");
	}
	CGrCSVText csv;
	if (csv.Open(filename.c_str())) {
		for (const auto &item : csv.GetCSVROW()) {
			if (item.size() < 2) {
				continue;
			}
			if (item[1].size() > 0) {
				menu_list[item[0]] = item[1];
			}
		}
	}
}

void saveMenuName(std::map<std::tstring, std::tstring> &menu_list)
{
	std::tstring filename = _T("menu_name.lis");
	auto lpDataPath = CGrTxtFunc::GetDataPath();
	if (lpDataPath) {
		CGrShell::ToPrettyFileName(lpDataPath, filename);
		filename += _T("/menu_name.lis");
	}
	CGrCSVText csv;
	for (const auto &[fmt, str] : menu_list) {
		if (str.size() > 0) {
			csv.AddFormatTail(L"ss", fmt.c_str(), str.c_str());
		}
	}
	csv.Save(filename.c_str());
}

CGrPropPage* PASCAL CGrPropPageOption::CreateProp(CGrTxtConfigDlg *pDlg)
{
	auto *pPage = new CGrPropPageOption(pDlg);
	pPage->m_hWnd = CreateDialogParam(CGrTxtFunc::GetDllModuleHandle(), MAKEINTRESOURCE(IDD_PROPPAGE_OPTION), pDlg->GetWnd(), reinterpret_cast<DLGPROC>(CGrWinCtrl::WindowMapProc), reinterpret_cast<LPARAM>(pPage));
	pPage->Attach(pPage->m_hWnd);
	return pPage;
}

CGrPropPageOption::CGrPropPageOption(CGrTxtConfigDlg *pDlg) : CGrPropPageSize(pDlg)
{
}

CGrPropPageOption::~CGrPropPageOption()
{
}

// ���C���E�B���h�E
LRESULT CGrPropPageOption::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg){
		HANDLE_MSG(hWnd, WM_INITDIALOG, OnInitDialog );
		HANDLE_MSG(hWnd, WM_NOTIFY    , OnNotify     );
		HANDLE_MSG(hWnd, WM_COMMAND   , OnCommand    );
	case WM_SETPAGEFOCUS:
		::SetFocus(reinterpret_cast<HWND>(wParam));
		break;
	}
	return CGrPropPageSize::WndProc(hWnd, uMsg, wParam, lParam);
}

BOOL CGrPropPageOption::OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	CGrPropPageSize::OnInitDialog(hwnd, hwndFocus, lParam);
	auto &&param = CGrTxtFunc::Param();
	// �E�C���h�E�T�C�Y�̕ۑ�
	::SetCheckDlgItemID(hwnd, IDC_CHECKBOX_SAVESIZE, param.GetBoolean(CGrTxtFuncIParam::PointsType::SaveWindowSize));
	// �^�C�g���o�[�̏���
	::SetDlgItemTextType(hwnd, param, IDC_EDIT_TITLEFORMAT, CGrTxtFuncIParam::TextType::TitleFormat);
	// susie�t�H���_
	::SetDlgItemTextType(hwnd, param, IDC_EDIT_SPIPLUGINFOLDER, CGrTxtFuncIParam::TextType::SpiPluginFolder);
	//
	int iUsePreParser[3] = {};
	param.GetPoints(CGrTxtFuncIParam::PointsType::UsePreParser, iUsePreParser, sizeof(iUsePreParser)/sizeof(int));
	::SetCheckDlgItemID(hwnd, IDC_CHECKBOX_USEPREPARSER      , iUsePreParser[0] == 1);
	::SetCheckDlgItemID(hwnd, IDC_CHECKBOX_SHOWPREPARSERERROR, iUsePreParser[1] == 1);
	::SetDlgItemInt(m_hWnd, IDC_EDIT_USEPREPARSERSIZE, iUsePreParser[2], true);
	//
	::SetDlgItemPointSet(hwnd, l_txt_point_set, sizeof(l_txt_point_set)/sizeof(PointSet));
	::SetDlgItemPointRange(hwnd, l_txt_point_range, sizeof(l_txt_point_range)/sizeof(PointRange));
	int iFileAutoReload[3] = {};
	param.GetPoints(CGrTxtFuncIParam::PointsType::FileAutoReload, iFileAutoReload, sizeof(iFileAutoReload)/sizeof(int));
	// �t�@�C���̍X�V���A�����I�ɍēǂݍ��݂��s��
	::SetCheckDlgItemID(m_hWnd, IDC_CHECKBOX_AUTORELOAD     , iFileAutoReload[0] == 1);
	// �ēǂݍ����A�r�����[�h�ŊJ��
	::SetCheckDlgItemID(m_hWnd, IDC_CHECKBOX_AUTORELOADLOCK , iFileAutoReload[1] == 1);
	// �Ď��Ԋu
	if(iFileAutoReload[2] <= 0){
		iFileAutoReload[2] = 1000;
	}
	::SetDlgItemInt(m_hWnd, IDC_EDIT_AUTORELOAD_TIME, iFileAutoReload[2], true);
	// �����Ԋu
	int iKeyInterval[1] = {};
	param.GetPoints(CGrTxtFuncIParam::PointsType::KeyInterval, iKeyInterval, sizeof(iKeyInterval)/sizeof(int));
	if(iKeyInterval[0] <= 0){
		iKeyInterval[0] = 1000;
	}
	::SetDlgItemInt(m_hWnd, IDC_EDIT_KEYINTERVAL, iKeyInterval[0], true);
	int iSpSelectionMode[2] = {};
	param.GetPoints(CGrTxtFuncIParam::PointsType::SpSelectionMode, iSpSelectionMode, sizeof(iSpSelectionMode)/sizeof(int));
	// ��I�����[�h�̎��A�V�t�g�L�[�����ňꎞ�I�ɑI�����[�h�ɐ؂�ւ���
	::SetCheckDlgItemID(m_hWnd, IDC_CHECKBOX_SPSELECTIONMODE, iSpSelectionMode[0] == 1);
	// ��I�����[�h�̎��A�V�t�g�L�[�����ňꎞ�I�ɑI�����[�h�ɐ؂�ւ���
	::SetCheckDlgItemID(m_hWnd, IDC_CHECKBOX_CLICK_SPSELECTIONMODE, iSpSelectionMode[1] == 1);
	// �����I���I����A�����ŃN���b�v�{�[�h�ɃR�s�[����
	::SetCheckDlgItemID(m_hWnd, IDC_CHECKBOX_AUTOCOPYMODE   , param.GetBoolean(CGrTxtFuncIParam::PointsType::AutoCopyMode   ));
	// �����ŃN���b�v�{�[�h�ɃR�s�[���邩���ɁA�ȉ��̃v���O�������s����
	::SetCheckDlgItemID(m_hWnd, IDC_CHECKBOX_RUNEXECCOPYTEXT, param.GetBoolean(CGrTxtFuncIParam::PointsType::RunExecCopyText));
	// �v���O�������A����
	::SetDlgItemTextType(hwnd, param, IDC_EDIT_COPYTEXTEXE   , CGrTxtFuncIParam::TextType::CopyTextExe     );
	::SetDlgItemTextType(hwnd, param, IDC_EDIT_COPYTEXTPRM   , CGrTxtFuncIParam::TextType::CopyTextPrm     );
	// �t�@�C���^�C�v
	::SetDlgItemTextType(hwnd, param, IDC_EDIT_FILETYPEHTML  , CGrTxtFuncIParam::TextType::FileTypeHtml    );
	::SetDlgItemTextType(hwnd, param, IDC_EDIT_FILETYPEARC7Z , CGrTxtFuncIParam::TextType::FileTypeArc7z   );
	::SetDlgItemTextType(hwnd, param, IDC_EDIT_FILETYPEARCCAB, CGrTxtFuncIParam::TextType::FileTypeArcCab  );
	::SetDlgItemTextType(hwnd, param, IDC_EDIT_FILETYPEARCLZH, CGrTxtFuncIParam::TextType::FileTypeArcLzh  );
	::SetDlgItemTextType(hwnd, param, IDC_EDIT_FILETYPEARCRAR, CGrTxtFuncIParam::TextType::FileTypeArcRar  );
	::SetDlgItemTextType(hwnd, param, IDC_EDIT_FILETYPEARCZIP, CGrTxtFuncIParam::TextType::FileTypeArcZip  );
	// �O���v���O����
	// �v���O�������A����
	::SetDlgItemTextType(hwnd, param, IDC_EDIT_OPENFILEEXE   , CGrTxtFuncIParam::TextType::OpenFileExe     );
	::SetDlgItemTextType(hwnd, param, IDC_EDIT_OPENFILEPRM   , CGrTxtFuncIParam::TextType::OpenFilePrm     );
	::SetDlgItemTextType(hwnd, param, IDC_EDIT_OPENFILEEXE1  , CGrTxtFuncIParam::TextType::OpenFileExe1    );
	::SetDlgItemTextType(hwnd, param, IDC_EDIT_OPENFILEPRM1  , CGrTxtFuncIParam::TextType::OpenFilePrm1    );
	::SetDlgItemTextType(hwnd, param, IDC_EDIT_OPENFILEEXE2  , CGrTxtFuncIParam::TextType::OpenFileExe2    );
	::SetDlgItemTextType(hwnd, param, IDC_EDIT_OPENFILEPRM2  , CGrTxtFuncIParam::TextType::OpenFilePrm2    );
	std::map<std::tstring, std::tstring> menu_list;
	loadMenuName(menu_list);
	SetDlgItemText(hwnd, IDC_EDIT_OPENFILE_MENUNAME1, menu_list[L"ExecOpenFiile"].c_str());
	SetDlgItemText(hwnd, IDC_EDIT_OPENFILE_MENUNAME2, menu_list[L"ExecOpenFiile1"].c_str());
	SetDlgItemText(hwnd, IDC_EDIT_OPENFILE_MENUNAME3, menu_list[L"ExecOpenFiile2"].c_str());
	// �����N����O���v���O����
	// �v���O�������A����
	::SetDlgItemTextType(hwnd, param, IDC_EDIT_LINKEXE       , CGrTxtFuncIParam::TextType::OpenLinkExe     );
	::SetDlgItemTextType(hwnd, param, IDC_EDIT_LINKPRM       , CGrTxtFuncIParam::TextType::OpenLinkPrm     );
	// �����R�[�h����DLL
	::SetDlgItemTextType(hwnd, param, IDC_EDIT_DETECTENC     , CGrTxtFuncIParam::TextType::GuessEncodingDLL);
	// �^�b�`���쎞�A���j���[�̕����Ԋu���L������
	::SetCheckDlgItemID(hwnd, IDC_CHECKBOX_TOUCHMENU, param.GetBoolean(CGrTxtFuncIParam::PointsType::TouchMenu));
	// �L�[���s�[�g
	::SetCheckDlgItemID(hwnd, IDC_CHECKBOX_KEYREPAT , param.GetBoolean(CGrTxtFuncIParam::PointsType::KeyRepeat));
	return TRUE;
}

void CGrPropPageOption::OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	switch(id){
	case IDC_CHECKBOX_USEPREPARSER:
	case IDC_CHECKBOX_SHOWPREPARSERERROR:
		PropSheet_Changed(::GetParent(hwnd), hwnd);
		break;
	case IDC_BUTTON_SPIPLUGINFOLDER: BrowseForFolder(hwnd, IDC_EDIT_SPIPLUGINFOLDER, IDS_SPI_FOLDER      ); break;
	case IDC_CHECKBOX_SAVESIZE        : // �E�C���h�E�T�C�Y�̕ۑ�
	case IDC_CHECKBOX_AUTORELOAD      : // �t�@�C���̍X�V���A�����I�ɍēǂݍ���
	case IDC_CHECKBOX_AUTORELOADLOCK  : // �t�@�C���̍X�V���A�����I�ɍēǂݍ��݂��s��
	case IDC_CHECKBOX_SPSELECTIONMODE : // ��I�����[�h�̎��A�V�t�g�L�[�����ňꎞ�I�ɑI�����[�h�ɐ؂�ւ���
	case IDC_CHECKBOX_CLICK_SPSELECTIONMODE: // ��I�����[�h�̎��A�V�t�g�L�[�����ňꎞ�I�ɑI�����[�h�ɐ؂�ւ���
	case IDC_CHECKBOX_AUTOCOPYMODE    : // �����I���I����A�����ŃN���b�v�{�[�h�ɃR�s�[����
	case IDC_CHECKBOX_RUNEXECCOPYTEXT : // �����ŃN���b�v�{�[�h�ɃR�s�[���邩���ɁA�ȉ��̃v���O�������s����
	case IDC_CHECKBOX_TOUCHMENU       : // �^�b�`���쎞�A���j���[�̕����Ԋu���L������ 
	case IDC_CHECKBOX_KEYREPAT        : // �L�[���s�[�g
		PropSheet_Changed(::GetParent(hwnd), hwnd);
		break;
	case IDC_EDIT_TITLEFORMAT     :
	case IDC_EDIT_SPIPLUGINFOLDER : // SUSIE�v���O�C���t�H���_
	case IDC_EDIT_COPYTEXTEXE     : //
	case IDC_EDIT_COPYTEXTPRM     : //
	case IDC_EDIT_OPENFILEEXE     : //
	case IDC_EDIT_OPENFILEPRM     : //
	case IDC_EDIT_OPENFILEEXE1    :
	case IDC_EDIT_OPENFILEPRM1    :
	case IDC_EDIT_OPENFILEEXE2    :
	case IDC_EDIT_OPENFILEPRM2    :
	case IDC_EDIT_LINKEXE         :
	case IDC_EDIT_LINKPRM         :
	case IDC_EDIT_FILETYPEHTML    : //
	case IDC_EDIT_FILETYPEARC7Z   : //
	case IDC_EDIT_FILETYPEARCCAB  : //
	case IDC_EDIT_FILETYPEARCLZH  : //
	case IDC_EDIT_FILETYPEARCRAR  : //
	case IDC_EDIT_FILETYPEARCZIP  : //
	case IDC_EDIT_DETECTENC       : // �����R�[�h����DLL
	case IDC_EDIT_AUTORELOAD_TIME : // �Ď��Ԋu
	case IDC_EDIT_KEYINTERVAL     : // �����Ԋu
	case IDC_EDIT_USEPREPARSERSIZE: // (n)MB�ȏ�̃t�@�C����Pre-Parser���g�p���Ȃ�
		if(codeNotify == EN_CHANGE){
			PropSheet_Changed(::GetParent(hwnd), hwnd);
		}
		break;
	case IDC_BUTTON_DETECTENC: // �����R�[�h����DLL
		{
			CGrCurrentDirectory curdir;
			std::tstring str;
			TCHAR filter[2048] = {};
			CGrText::LoadString(IDS_DETECTENCFILE, filter, sizeof(filter)/sizeof(TCHAR));
			CGrText::LoadString(IDS_OPENFILE, str);

			OPENFILENAME of = {sizeof(OPENFILENAME)};
			TCHAR dlg_fileName[MAX_PATH] = {};
			TCHAR fileName[MAX_PATH] = {};
			GetDlgItemText(m_hWnd, IDC_EDIT_DETECTENC, dlg_fileName, sizeof(dlg_fileName)/sizeof(TCHAR));

			LPTSTR lpFilePart;
			::GetFullPathName(dlg_fileName, sizeof(fileName)/sizeof(TCHAR), fileName, &lpFilePart);

			of.hwndOwner       = m_hWnd;
			of.lpstrFilter     = filter;
			of.lpstrTitle      = str.c_str();
			of.nMaxCustFilter  = 40;
			of.lpstrFile       = fileName;
			of.nMaxFile        = MAX_PATH - 1;
			of.lpstrInitialDir = _T(".\\");
			of.Flags           = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR;
			if(GetOpenFileName(&of)){
				SetDlgItemText(m_hWnd, IDC_EDIT_DETECTENC, fileName);
			}
		}
		break;
	}
}

bool CGrPropPageOption::Apply()
{
	auto &&param = CGrTxtFunc::Param();
	::GetDlgItemPointSet(m_hWnd, l_txt_point_set, sizeof(l_txt_point_set)/sizeof(PointSet));
	// �E�C���h�E�T�C�Y�̕ۑ�
	param.SetBoolean(CGrTxtFuncIParam::PointsType::SaveWindowSize, ::GetCheckDlgItemID(m_hWnd, IDC_CHECKBOX_SAVESIZE) == BST_CHECKED);
	// �^�C�g���o�[�̏���
	::GetDlgItemTextType(m_hWnd, param, IDC_EDIT_TITLEFORMAT, CGrTxtFuncIParam::TextType::TitleFormat);
	// susie�t�H���_
	::GetDlgItemTextType(m_hWnd, param, IDC_EDIT_SPIPLUGINFOLDER, CGrTxtFuncIParam::TextType::SpiPluginFolder);
	//
	int iUsePreParserFlag;
	int iUsePreParser[3] = {};
	iUsePreParser[0] = ::GetCheckDlgItemID(m_hWnd, IDC_CHECKBOX_USEPREPARSER      ) == BST_CHECKED ? 1 : 0;
	iUsePreParser[1] = ::GetCheckDlgItemID(m_hWnd, IDC_CHECKBOX_SHOWPREPARSERERROR) == BST_CHECKED ? 1 : 0;
	iUsePreParser[2] = ::GetDlgItemInt(m_hWnd, IDC_EDIT_USEPREPARSERSIZE, &iUsePreParserFlag, true);
	param.SetPoints(CGrTxtFuncIParam::PointsType::UsePreParser, iUsePreParser, sizeof(iUsePreParser)/sizeof(int));
	int iFileAutoReload[3] = {};
	// �t�@�C���̍X�V���A�����I�ɍēǂݍ��݂��s��
	iFileAutoReload[0] = ::GetCheckDlgItemID(m_hWnd, IDC_CHECKBOX_AUTORELOAD    ) == BST_CHECKED ? 1 : 0;
	// �ēǂݍ����A�r�����[�h�ŊJ��
	iFileAutoReload[1] = ::GetCheckDlgItemID(m_hWnd, IDC_CHECKBOX_AUTORELOADLOCK) == BST_CHECKED ? 1 : 0;
	// �Ď��Ԋu
	int iFileAutoReloadFlag;
	iFileAutoReload[2] = ::GetDlgItemInt(m_hWnd, IDC_EDIT_AUTORELOAD_TIME, &iFileAutoReloadFlag, true);
	if(iFileAutoReload[2] <= 0){
		iFileAutoReload[2] = 1000;
	}
	param.SetPoints(CGrTxtFuncIParam::PointsType::FileAutoReload, iFileAutoReload, sizeof(iFileAutoReload)/sizeof(int));
	// �����Ԋu
	int iKeyInterval[1];
	int iKeyIntervalFlag;
	iKeyInterval[0] = ::GetDlgItemInt(m_hWnd, IDC_EDIT_KEYINTERVAL, &iKeyIntervalFlag, true);
	param.SetPoints(CGrTxtFuncIParam::PointsType::KeyInterval, iKeyInterval, sizeof(iKeyInterval)/sizeof(int));
	int iSpSelectionMode[2] = {};
	// ��I�����[�h�̎��A�V�t�g�L�[�����ňꎞ�I�ɑI�����[�h�ɐ؂�ւ���
	iSpSelectionMode[0] = ::GetCheckDlgItemID(m_hWnd, IDC_CHECKBOX_SPSELECTIONMODE      ) == BST_CHECKED ? 1 : 0;
	// ��I�����[�h�̎��A�V�t�g�L�[�����ňꎞ�I�ɑI�����[�h�ɐ؂�ւ���
	iSpSelectionMode[1] = ::GetCheckDlgItemID(m_hWnd, IDC_CHECKBOX_CLICK_SPSELECTIONMODE) == BST_CHECKED ? 1 : 0;
	param.SetPoints(CGrTxtFuncIParam::PointsType::SpSelectionMode, iSpSelectionMode, sizeof(iSpSelectionMode)/sizeof(int));
	// �����I���I����A�����ŃN���b�v�{�[�h�ɃR�s�[����
	param.SetBoolean(CGrTxtFuncIParam::PointsType::AutoCopyMode, ::GetCheckDlgItemID(m_hWnd, IDC_CHECKBOX_AUTOCOPYMODE) == BST_CHECKED);
	// �����ŃN���b�v�{�[�h�ɃR�s�[���邩���ɁA�ȉ��̃v���O�������s����
	param.SetBoolean(CGrTxtFuncIParam::PointsType::RunExecCopyText, ::GetCheckDlgItemID(m_hWnd, IDC_CHECKBOX_RUNEXECCOPYTEXT) == BST_CHECKED);
	// �v���O�������A����
	::GetDlgItemTextType(m_hWnd, param, IDC_EDIT_COPYTEXTEXE   , CGrTxtFuncIParam::TextType::CopyTextExe     );
	::GetDlgItemTextType(m_hWnd, param, IDC_EDIT_COPYTEXTPRM   , CGrTxtFuncIParam::TextType::CopyTextPrm     );
	// �t�@�C���^�C�v
	::GetDlgItemTextType(m_hWnd, param, IDC_EDIT_FILETYPEHTML  , CGrTxtFuncIParam::TextType::FileTypeHtml    );
	::GetDlgItemTextType(m_hWnd, param, IDC_EDIT_FILETYPEARC7Z , CGrTxtFuncIParam::TextType::FileTypeArc7z   );
	::GetDlgItemTextType(m_hWnd, param, IDC_EDIT_FILETYPEARCCAB, CGrTxtFuncIParam::TextType::FileTypeArcCab  );
	::GetDlgItemTextType(m_hWnd, param, IDC_EDIT_FILETYPEARCLZH, CGrTxtFuncIParam::TextType::FileTypeArcLzh  );
	::GetDlgItemTextType(m_hWnd, param, IDC_EDIT_FILETYPEARCRAR, CGrTxtFuncIParam::TextType::FileTypeArcRar  );
	::GetDlgItemTextType(m_hWnd, param, IDC_EDIT_FILETYPEARCZIP, CGrTxtFuncIParam::TextType::FileTypeArcZip  );
	// �O���v���O����
	// �v���O�������A����
	::GetDlgItemTextType(m_hWnd, param, IDC_EDIT_OPENFILEEXE   , CGrTxtFuncIParam::TextType::OpenFileExe     );
	::GetDlgItemTextType(m_hWnd, param, IDC_EDIT_OPENFILEPRM   , CGrTxtFuncIParam::TextType::OpenFilePrm     );
	::GetDlgItemTextType(m_hWnd, param, IDC_EDIT_OPENFILEEXE1  , CGrTxtFuncIParam::TextType::OpenFileExe1    );
	::GetDlgItemTextType(m_hWnd, param, IDC_EDIT_OPENFILEPRM1  , CGrTxtFuncIParam::TextType::OpenFilePrm1    );
	::GetDlgItemTextType(m_hWnd, param, IDC_EDIT_OPENFILEEXE2  , CGrTxtFuncIParam::TextType::OpenFileExe2    );
	::GetDlgItemTextType(m_hWnd, param, IDC_EDIT_OPENFILEPRM2  , CGrTxtFuncIParam::TextType::OpenFilePrm2    );
	std::map<std::tstring, std::tstring> menu_list;
	loadMenuName(menu_list);
	TCHAR buf[2048];
	GetDlgItemText(m_hWnd, IDC_EDIT_OPENFILE_MENUNAME1, buf, _countof(buf)); menu_list[L"ExecOpenFiile"] = buf;
	GetDlgItemText(m_hWnd, IDC_EDIT_OPENFILE_MENUNAME2, buf, _countof(buf)); menu_list[L"ExecOpenFiile1"] = buf;
	GetDlgItemText(m_hWnd, IDC_EDIT_OPENFILE_MENUNAME3, buf, _countof(buf)); menu_list[L"ExecOpenFiile2"] = buf;
	saveMenuName(menu_list);
	// �����N����O���v���O����
	// �v���O�������A����
	::GetDlgItemTextType(m_hWnd, param, IDC_EDIT_LINKEXE       , CGrTxtFuncIParam::TextType::OpenLinkExe     );
	::GetDlgItemTextType(m_hWnd, param, IDC_EDIT_LINKPRM       , CGrTxtFuncIParam::TextType::OpenLinkPrm     );
	// �����R�[�h����DLL
	::GetDlgItemTextType(m_hWnd, param, IDC_EDIT_DETECTENC     , CGrTxtFuncIParam::TextType::GuessEncodingDLL);
	// �^�b�`���쎞�A���j���[�̕����Ԋu���L������
	param.SetBoolean(CGrTxtFuncIParam::PointsType::TouchMenu, ::GetCheckDlgItemID(m_hWnd, IDC_CHECKBOX_TOUCHMENU) == BST_CHECKED);
	// �L�[���s�[�g
	param.SetBoolean(CGrTxtFuncIParam::PointsType::KeyRepeat, ::GetCheckDlgItemID(m_hWnd, IDC_CHECKBOX_KEYREPAT ) == BST_CHECKED);
	//
	param.UpdateConfig(GetParent(GetParent(m_hWnd)));
	//
	return true;
}

