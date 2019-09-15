#pragma warning( disable : 4786 )

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "resource.h"
#include "PropPageStyle.h"
#include "TxtConfigDlg.h"
#include "Text.h"
#include "Shell.h"
#include "CurrentDirectory.h"
#include "TxtFuncConfig.h"
#include "TxtFunc.h"
#include "TxtConfigFunc.h"
#include "MessageBox.h"

static struct FontSpacing l_font_spacing[] = {
	{IDC_EDIT_TEXT_FONTSPACING         , IDC_CHECKBOX_TEXT_CENTER         , CGrTxtFuncIParam::Text        },
	{IDC_EDIT_BOLD_FONTSPACING         , IDC_CHECKBOX_BOLD_CENTER         , CGrTxtFuncIParam::Bold        },
	{IDC_EDIT_RUBY_FONTSPACING         , IDC_CHECKBOX_RUBY_CENTER         , CGrTxtFuncIParam::Ruby        },
	{IDC_EDIT_NOTE_FONTSPACING         , IDC_CHECKBOX_NOTE_CENTER         , CGrTxtFuncIParam::Note        },
	{IDC_EDIT_NOMBRE_FONTSPACING       , IDC_CHECKBOX_NOMBRE_CENTER       , CGrTxtFuncIParam::Nombre      },
	{IDC_EDIT_RUNNING_HEADS_FONTSPACING, IDC_CHECKBOX_RUNNING_HEADS_CENTER, CGrTxtFuncIParam::RunningHeads},
};
static struct FontWeightName l_font_weight_name[] = {
	{IDS_FW_REGULAR, FW_REGULAR},
	{IDS_FW_BOLD   , FW_BOLD   },
};
static struct FontWeight l_font_weight[] = {
	{IDC_COMBO_TEXT_WEIGHT         , CGrTxtFuncIParam::Text        },
	{IDC_COMBO_BOLD_WEIGHT         , CGrTxtFuncIParam::Bold        },
	{IDC_COMBO_RUBY_WEIGHT         , CGrTxtFuncIParam::Ruby        },
	{IDC_COMBO_NOTE_WEIGHT         , CGrTxtFuncIParam::Note        },
	{IDC_COMBO_NOMBRE_WEIGHT       , CGrTxtFuncIParam::Nombre      },
	{IDC_COMBO_RUNNING_HEADS_WEIGHT, CGrTxtFuncIParam::RunningHeads},
};
static struct FontName l_font_name[] = {
	{IDC_COMBO_TEXT_FONT         , CGrTxtFuncIParam::Text        },
	{IDC_COMBO_BOLD_FONT         , CGrTxtFuncIParam::Bold        },
	{IDC_COMBO_RUBY_FONT         , CGrTxtFuncIParam::Ruby        },
	{IDC_COMBO_NOTE_FONT         , CGrTxtFuncIParam::Note        },
	{IDC_COMBO_NOMBRE_FONT       , CGrTxtFuncIParam::Nombre      },
	{IDC_COMBO_RUNNING_HEADS_FONT, CGrTxtFuncIParam::RunningHeads},
};
static struct PointSet l_style_point_set[] = {
	{IDC_EDIT_TATECHUNUM          , CGrTxtFuncIParam::TateChuNum      },
	{IDC_EDIT_PAGEFLIP_TIME       , CGrTxtFuncIParam::PageFlipInterval},
	{IDC_EDIT_WHITE_RATE          , CGrTxtFuncIParam::WhiteTransRate  },
};
static struct ValueSet l_style_value_set[] = {
	{IDC_EDIT_FIRST_CHAR  , CGrTxtFuncIParam::LineStartNGCharacters},
	{IDC_EDIT_END_CHAR    , CGrTxtFuncIParam::LineEndNGCharacters  },
	{IDC_EDIT_HANG_CHAR   , CGrTxtFuncIParam::HangingCharacters    },
	{IDC_EDIT_SEP_NG_CHAR , CGrTxtFuncIParam::SeparateNGCharacters },
	{IDC_EDIT_ROTATE_CHAR , CGrTxtFuncIParam::RotateCharacters     },
	{IDC_EDIT_RROTATE_CHAR, CGrTxtFuncIParam::RRotateCharacters    },
};
static struct PointRange l_style_point_range[] = {
	{IDC_SPIN_TATECHUNUM          , 0, 4   , IDS_FONT_SIZE             },
	{IDC_SPIN_PAGEFLIP_TIME       , 0, 100 , IDS_PAGE_FLIP_INTERVAL    },
	{IDC_SPIN_WHITE_RATE          , 0, 100 , IDS_WHITE_TRANS_RATE_RANGE},
	{IDC_SPIN_RUNNINGHEAD_LEVEL   , 1, 6   , IDS_EDIT_RUNNINGHEAD_LEVEL}, // 2.0.27.0
};

LRESULT CGrEditImgFile::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg){
	case WM_DROPFILES:
		{
			TCHAR fileName[256];
			auto hDrop = reinterpret_cast<HDROP>(wParam);
			::DragQueryFile(hDrop, 0, fileName, sizeof(fileName)/sizeof(TCHAR));
			::DragFinish(hDrop);
			CGrCurrentDirectory curDir;
			auto lpFilename = CGrShell::RemovePath(fileName, curDir.GetModuleDir());
			if(lpFilename){
				SetWindowText(hWnd, CGrText::CharNext(lpFilename));
			} else {
				SetWindowText(hWnd, fileName);
			}
		}
		break;
	}
	return CGrComonCtrl::WndProc(hWnd, uMsg, wParam, lParam);
}

////////////////////
CGrPropPage* PASCAL CGrPropPageStyle::CreateProp(CGrTxtConfigDlg *pDlg)
{
	auto *pPage = new CGrPropPageStyle(pDlg);
	pPage->m_hWnd = CreateDialogParam(CGrTxtFunc::GetDllModuleHandle(), MAKEINTRESOURCE(IDD_PROPPAGE_STYLE), pDlg->GetWnd(), reinterpret_cast<DLGPROC>(CGrWinCtrl::WindowMapProc), reinterpret_cast<LPARAM>(pPage));
	pPage->Attach(pPage->m_hWnd);
	return pPage;
}

CGrPropPageStyle::CGrPropPageStyle(CGrTxtConfigDlg *pDlg) : CGrPropPageSize(pDlg)
{
}

CGrPropPageStyle::~CGrPropPageStyle()
{
}

// コンボボックスに文字列を追加します。
//   ctrl_id [in]: コンボボックスのID
//   str_id  [in]: 文字列のID
//   data    [in]: DATA
void CGrPropPageStyle::comboAddString(int ctrl_id, int str_id, int data)
{
	auto cwnd = GetDlgItem(m_hWnd, ctrl_id);
	if(!cwnd){
		return;
	}
	std::tstring str;
	CGrText::LoadString(str_id, str);
	int idx = ComboBox_AddString(cwnd, str.c_str());
	ComboBox_SetItemData(cwnd, idx, data);
}

// メインウィンドウ
LRESULT CGrPropPageStyle::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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

// button attach window handle
//   id    [in]: ID
//   btn   [in]: CGrColorButton
//   color [in]: color
void CGrPropPageStyle::attachBtn(UINT id, CGrColorButton &btn, COLORREF color)
{
	btn.Attach(GetDlgItem(m_hWnd, id));
	btn.SetColor(color);
}

BOOL CGrPropPageStyle::OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	CGrPropPageSize::OnInitDialog(hwnd, hwndFocus, lParam);
	auto &&param = CGrTxtFunc::Param();
	COLORREF page_color[3] = {};
	COLORREF link_color = {};
	param.GetPoints(CGrTxtFuncIParam::PageColor    , reinterpret_cast<int*>(page_color ), sizeof(page_color)/sizeof(COLORREF));
	param.GetPoints(CGrTxtFuncIParam::LinkTextColor, reinterpret_cast<int*>(&link_color), 1                                  );
	attachBtn(IDC_BUTTON_TEXT_COLOR          , m_colBtnText              , param.GetColor(CGrTxtFuncIParam::Text        ));
	attachBtn(IDC_BUTTON_BOLD_COLOR          , m_colBtnBold              , param.GetColor(CGrTxtFuncIParam::Bold        ));
	attachBtn(IDC_BUTTON_RUBY_COLOR          , m_colBtnRuby              , param.GetColor(CGrTxtFuncIParam::Ruby        ));
	attachBtn(IDC_BUTTON_NOTE_COLOR          , m_colBtnNote              , param.GetColor(CGrTxtFuncIParam::Note        ));
	attachBtn(IDC_BUTTON_NOMBRE_COLOR        , m_colBtnNombre            , param.GetColor(CGrTxtFuncIParam::Nombre      ));
	attachBtn(IDC_BUTTON_RUNNING_HEADS_COLOR , m_colBtnRunningHeads      , param.GetColor(CGrTxtFuncIParam::RunningHeads));
	attachBtn(IDC_BUTTON_PAPER_BKCOLOR       , m_colBtnPaperBkColor      , page_color[0]                                 );
	attachBtn(IDC_BUTTON_BKCOLOR             , m_colBtnBkColor           , page_color[2]                                 );
	attachBtn(IDC_BUTTON_LINKCOLOR           , m_colBtnLinkColor         , link_color                                    );
	// ページ設定 NxN
	::SetDlgItemPointSet(m_hWnd, l_style_point_set, sizeof(l_style_point_set)/sizeof(PointSet));
	::SetDlgItemPointRange(m_hWnd, l_style_point_range, sizeof(l_style_point_range)/sizeof(PointRange));
	// フォント
	LOGFONT lfFont = {};
	lfFont.lfCharSet = DEFAULT_CHARSET;
	lfFont.lfFaceName[0] = 0;
	lfFont.lfPitchAndFamily = 0;
	auto hdc = ::GetDC(m_hWnd);
	::EnumFontFamiliesEx(hdc, &lfFont,
						 [](CONST LOGFONTW *lpelfe, CONST TEXTMETRICW *lpntme, DWORD FontType, LPARAM lParam) {
							 auto *lp_font_name_list = reinterpret_cast<FontNameList*>(lParam);
							 lp_font_name_list->insert(lpelfe->lfFaceName);
							 return static_cast<int>(TRUE);
						 },
						 reinterpret_cast<LPARAM>(&m_fontNameList), 0);
	::ReleaseDC(m_hWnd, hdc);
	{
		auto hwnd_text         = GetDlgItem(m_hWnd, IDC_COMBO_TEXT_FONT         );
		auto hwnd_bold         = GetDlgItem(m_hWnd, IDC_COMBO_BOLD_FONT         );
		auto hwnd_ruby         = GetDlgItem(m_hWnd, IDC_COMBO_RUBY_FONT         );
		auto hwnd_note         = GetDlgItem(m_hWnd, IDC_COMBO_NOTE_FONT         );
		auto hwnd_nombre       = GetDlgItem(m_hWnd, IDC_COMBO_NOMBRE_FONT       );
		auto hwnd_runningheads = GetDlgItem(m_hWnd, IDC_COMBO_RUNNING_HEADS_FONT);
		for(const auto &item : m_fontNameList){
			ComboBox_AddString(hwnd_text        , item.c_str());
			ComboBox_AddString(hwnd_bold        , item.c_str());
			ComboBox_AddString(hwnd_ruby        , item.c_str());
			ComboBox_AddString(hwnd_note        , item.c_str());
			ComboBox_AddString(hwnd_nombre      , item.c_str());
			ComboBox_AddString(hwnd_runningheads, item.c_str());
		}
		SetWindowText(hwnd_text        , param.GetFontName(CGrTxtFuncIParam::Text        ));
		SetWindowText(hwnd_bold        , param.GetFontName(CGrTxtFuncIParam::Bold        ));
		SetWindowText(hwnd_ruby        , param.GetFontName(CGrTxtFuncIParam::Ruby        ));
		SetWindowText(hwnd_note        , param.GetFontName(CGrTxtFuncIParam::Note        ));
		SetWindowText(hwnd_nombre      , param.GetFontName(CGrTxtFuncIParam::Nombre      ));
		SetWindowText(hwnd_runningheads, param.GetFontName(CGrTxtFuncIParam::RunningHeads));
	}
	{
		TCHAR buf[1024];
		param.GetText(CGrTxtFuncIParam::LayoutFile, buf, sizeof(buf)/sizeof(TCHAR));
		std::tstring filename(buf);

		CGrTxtLayoutMgr mgr;
		m_lil.clear();
		mgr.GetLayoutList(m_lil);
		auto hwnd_layout = GetDlgItem(m_hWnd, IDC_COMBO_LAYOUT);
		int iselect = 0;
		int len = m_lil.size();
		if(len > 0){
			const auto *it = &m_lil[0];
			for(int idx=0; len>0; --len, ++it, ++idx){
				const auto &li = *it;
				int iItem = ComboBox_AddString(hwnd_layout, li.name.c_str());
				ComboBox_SetItemData(hwnd_layout, iItem, idx);
				if(li.filename == filename){
					iselect = idx;
				}
			}
		}
		ComboBox_SetCurSel(hwnd_layout, iselect);
	}
	// 文字間隔
	// 中央揃え
	int idx = 0;
	for(idx=0; idx<sizeof(l_font_spacing)/sizeof(FontSpacing); ++idx){
		const auto &fs = l_font_spacing[idx];
		TCHAR buf[512] = {};
		_stprintf_s(buf, _T("%g"), param.GetFontSpacing(fs.type));
		SetDlgItemText(m_hWnd, fs.id_spacing, buf);
		::SetCheckDlgItemID(hwnd, fs.id_center, param.GetFontCentering(fs.type));
	}
	// スタイル
	for(idx=0; idx<sizeof(l_font_weight)/sizeof(FontWeight); ++idx){
		const auto &fw = l_font_weight[idx];
		auto hwnd_cmb = GetDlgItem(m_hWnd, fw.id);
		long lfWeight = param.GetFontWeight(fw.type);
		int iselect = 0;
		for(int cmb_idx=0; cmb_idx<sizeof(l_font_weight_name)/sizeof(FontWeightName); ++cmb_idx){
			const auto &fwn = l_font_weight_name[cmb_idx];
			std::tstring str;
			CGrText::LoadString(fwn.str_id, str);
			ComboBox_AddString(hwnd_cmb, str.c_str());
			ComboBox_SetItemData(hwnd_cmb, cmb_idx, fwn.lfWeight);
			if(fwn.lfWeight == lfWeight){
				iselect = cmb_idx;
			}
		}
		ComboBox_SetCurSel(hwnd_cmb, iselect);
	}
	// 背景画像
	m_imgEditBox.Attach(GetDlgItem(m_hWnd, IDC_EDIT_BACK_IMG));
	::SetDlgItemTextType(m_hWnd, param, IDC_EDIT_BACK_IMG, CGrTxtFuncIParam::BackgroundImage);
	//
	::CheckDlgButton(m_hWnd, param, IDC_CHECKBOX_WZMEMO           , CGrTxtFuncIParam::WzMemoMode     );
	::CheckDlgButton(m_hWnd, param, IDC_CHECKBOX_OVERLAPCHAR      , CGrTxtFuncIParam::UseOverlapChar );
	::CheckDlgButton(m_hWnd, param, IDC_CHECKBOX_IMAGE_NEXT_LAYOUT, CGrTxtFuncIParam::ImageNextLayout);
	//
	for(idx=0; idx<sizeof(l_style_value_set)/sizeof(ValueSet); ++idx){
		TCHAR buf[1024] = {};
		param.GetTextList(l_style_value_set[idx].type, buf, sizeof(buf)/sizeof(TCHAR));
		SetDlgItemText(m_hWnd, l_style_value_set[idx].id , buf);
	}
	//
	::CheckDlgButton(m_hWnd, param, IDC_CHECKBOX_PAGEFLIP, CGrTxtFuncIParam::PageFlip);
	::EnableCheckDlgItemID(m_hWnd, IDC_CHECKBOX_PAGEFLIP, IDC_SPIN_PAGEFLIP_TIME, IDC_EDIT_PAGEFLIP_TIME);
	// アンチエイリアス
	int iAntiAlias = 0;
	param.GetPoints(CGrTxtFuncIParam::AntiAlias, &iAntiAlias, 1);
	CheckDlgButton(m_hWnd, IDC_ANTIALIAS_NORMAL, iAntiAlias == 1);
	CheckDlgButton(m_hWnd, IDC_ANTIALIAS_LCD   , iAntiAlias == 2);
	// ルビ
	int iRubyPosition = 0;
	param.GetPoints(CGrTxtFuncIParam::RubyPosition, &iRubyPosition, 1);
	CheckDlgButton(m_hWnd, IDC_RUBYPOSITION_TOP   , iRubyPosition == 0);
	CheckDlgButton(m_hWnd, IDC_RUBYPOSITION_CENTER, iRubyPosition == 1);
	// 白色画像透過
	::CheckDlgButton(m_hWnd, param, IDC_CHECKBOX_WHITETRANS, CGrTxtFuncIParam::WhiteTrans);
	::EnableCheckDlgItemID(m_hWnd, IDC_CHECKBOX_WHITETRANS, IDC_SPIN_WHITE_RATE, IDC_EDIT_WHITE_RATE);
	// 画像をページ一杯に表示
	::CheckDlgButton(m_hWnd, param, IDC_CHECKBOX_PICTPADDINGNONE, CGrTxtFuncIParam::PictPaddingNone);
	// ページ内で最初に見つかった見出しを柱にする
	int iRunningHeadLevel[2] = {1,1};
	param.GetPoints(CGrTxtFuncIParam::RunningHeadLevel, iRunningHeadLevel, _countof(iRunningHeadLevel));
	::CheckDlgButton(m_hWnd, IDC_CHECKBOX_RUNNINGHEAD_1ST, iRunningHeadLevel[0] == 1);
	// レベルnまでの見出しを柱の対象にする
	::SetDlgItemInt(hwnd, IDC_EDIT_RUNNINGHEAD_LEVEL, iRunningHeadLevel[1], true);
	// Dark mode
	std::tstring ini_filename(CGrTxtFunc::GetDataPath());
	ini_filename += _T("/TxtMiru.ini");
	TCHAR val[2048];
	::GetPrivateProfileString(_T("points"), _T("EnableDarkTheme"), _T(""), val, sizeof(val) / sizeof(TCHAR), ini_filename.c_str());
	::SetCheckDlgItemID(hwnd, IDC_CHECKBOX_DARKMODE, lstrcmpi(val, _T("1")) == 0);
	// 青空文庫設定
	auto hAozoraBottom = GetDlgItem(hwnd, IDC_COMBOBOX_AOZORA_BOTTOM);
	int iAozoraSetting[1] = {};
	param.GetPoints(CGrTxtFuncIParam::AozoraSetting, iAozoraSetting, sizeof(iAozoraSetting) / sizeof(int));
	for (auto id : { IDS_AOZORA_BOTTOM_NEXT, IDS_AOZORA_BOTTOM_LIMIT, IDS_AOZORA_BOTTOM_NONE }) {
		std::tstring str;
		CGrText::LoadString(id, str);
		SendMessage(hAozoraBottom, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(str.c_str()));
	}
	SendMessage(hAozoraBottom, CB_SETCURSEL, iAozoraSetting[0], 0);
	// 起動時に、Fontフォルダ以下のフォントをロードする
	::CheckDlgButton(m_hWnd, param, IDC_CHECKBOX_USEFONT, CGrTxtFuncIParam::UseFont);

	return TRUE;
}

void CGrPropPageStyle::OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	switch(id){
	case IDC_EDIT_FIRST_CHAR               :
	case IDC_EDIT_END_CHAR                 :
	case IDC_EDIT_TATECHUNUM               :
	case IDC_EDIT_BACK_IMG                 :
	case IDC_EDIT_PAGEFLIP_TIME            :
	case IDC_EDIT_WHITE_RATE               :
	case IDC_EDIT_HANG_CHAR                :
	case IDC_EDIT_SEP_NG_CHAR              :
	case IDC_EDIT_ROTATE_CHAR              :
	case IDC_EDIT_RROTATE_CHAR             :
	case IDC_EDIT_TEXT_FONTSPACING         :
	case IDC_EDIT_BOLD_FONTSPACING         :
	case IDC_EDIT_RUBY_FONTSPACING         :
	case IDC_EDIT_NOTE_FONTSPACING         :
	case IDC_EDIT_NOMBRE_FONTSPACING       :
	case IDC_EDIT_RUNNING_HEADS_FONTSPACING:
	case IDC_EDIT_RUNNINGHEAD_LEVEL        :
		if(codeNotify == EN_CHANGE){
			PropSheet_Changed(::GetParent(hwnd), hwnd);
		}
		break;
	case IDC_COMBO_TEXT_FONT           :
	case IDC_COMBO_BOLD_FONT           :
	case IDC_COMBO_RUBY_FONT           :
	case IDC_COMBO_NOTE_FONT           :
	case IDC_COMBO_NOMBRE_FONT         :
	case IDC_COMBO_RUNNING_HEADS_FONT  :
	case IDC_COMBO_TEXT_WEIGHT         :
	case IDC_COMBO_BOLD_WEIGHT         :
	case IDC_COMBO_RUBY_WEIGHT         :
	case IDC_COMBO_NOTE_WEIGHT         :
	case IDC_COMBO_NOMBRE_WEIGHT       :
	case IDC_COMBO_RUNNING_HEADS_WEIGHT:
	case IDC_COMBO_LAYOUT              :
	case IDC_COMBOBOX_AOZORA_BOTTOM: // 地付き、地寄せ
		if(codeNotify == CBN_EDITCHANGE || codeNotify == CBN_SELCHANGE){
			PropSheet_Changed(::GetParent(hwnd), hwnd);
		}
		break;
	case IDC_BUTTON_BACK_IMG:
		{
			// プログラム本体から見て相対パスで指定している場合、カレントが変わっていると上手く動作しないので
			// 一旦、カレントフォルダをプログラム本体のパス変更してから GetFullPathName で位置を決定する
			CGrCurrentDirectory curdir;
			std::tstring str;
			TCHAR filter[2048] = {};
			CGrText::LoadString(IDS_IMAGEFILE, filter, sizeof(filter)/sizeof(TCHAR));
			CGrText::LoadString(IDS_OPENFILE, str);

			OPENFILENAME of = {sizeof(OPENFILENAME)};
			TCHAR dlg_fileName[MAX_PATH] = {};
			TCHAR fileName[MAX_PATH] = {};
			GetDlgItemText(m_hWnd, IDC_EDIT_BACK_IMG, dlg_fileName, sizeof(dlg_fileName)/sizeof(TCHAR));

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
				CGrCurrentDirectory curDir;
				auto lpFilename = CGrShell::RemovePath(fileName, curDir.GetModuleDir());
				if(lpFilename){
					SetDlgItemText(m_hWnd, IDC_EDIT_BACK_IMG, CGrText::CharNext(lpFilename));
				} else {
					SetDlgItemText(m_hWnd, IDC_EDIT_BACK_IMG, fileName);
				}
			}
		}
		break;
	case IDC_BUTTON_TEXT_COLOR            :
	case IDC_BUTTON_BOLD_COLOR            :
	case IDC_BUTTON_RUBY_COLOR            :
	case IDC_BUTTON_NOTE_COLOR            :
	case IDC_BUTTON_NOMBRE_COLOR          :
	case IDC_BUTTON_RUNNING_HEADS_COLOR   :
	case IDC_BUTTON_PAPER_BKCOLOR         :
	case IDC_BUTTON_BKCOLOR               :
	case IDC_BUTTON_LINKCOLOR             :
	case IDC_CHECKBOX_WZMEMO              :
	case IDC_CHECKBOX_IMAGE_NEXT_LAYOUT   :
	case IDC_CHECKBOX_OVERLAPCHAR         :
	case IDC_ANTIALIAS_NORMAL             :
	case IDC_ANTIALIAS_LCD                :
	case IDC_RUBYPOSITION_TOP             :
	case IDC_RUBYPOSITION_CENTER          :
	case IDC_CHECKBOX_TEXT_CENTER         :
	case IDC_CHECKBOX_BOLD_CENTER         :
	case IDC_CHECKBOX_RUBY_CENTER         :
	case IDC_CHECKBOX_NOTE_CENTER         :
	case IDC_CHECKBOX_NOMBRE_CENTER       :
	case IDC_CHECKBOX_RUNNING_HEADS_CENTER:
	case IDC_CHECKBOX_PICTPADDINGNONE     :
	case IDC_CHECKBOX_RUNNINGHEAD_1ST     :
	case IDC_CHECKBOX_DARKMODE            :
	case IDC_CHECKBOX_USEFONT             :	// 起動時に、Fontフォルダ以下のフォントをロードする
		PropSheet_Changed(::GetParent(hwnd), hwnd);
		break;
	case IDC_CHECKBOX_PAGEFLIP            :
		::EnableCheckDlgItemID(m_hWnd, IDC_CHECKBOX_PAGEFLIP, IDC_SPIN_PAGEFLIP_TIME, IDC_EDIT_PAGEFLIP_TIME);
		PropSheet_Changed(::GetParent(hwnd), hwnd);
		break;
	case IDC_CHECKBOX_WHITETRANS          :
		::EnableCheckDlgItemID(m_hWnd, IDC_CHECKBOX_WHITETRANS, IDC_SPIN_WHITE_RATE, IDC_EDIT_WHITE_RATE);
		PropSheet_Changed(::GetParent(hwnd), hwnd);
		break;
	case ID_SAVE_STYLE:
		{
			TCHAR styleName[MAX_PATH+1];
			GetDlgItemText(m_hWnd, IDC_EDIT_SAVE_STYLE_NAME, styleName, sizeof(styleName)/sizeof(TCHAR));
			if(styleName[0] == '\0'){
				CGrMessageBox::Show(CGrTxtFunc::GetDllModuleHandle(), m_hWnd, IDS_STYLE_NAME_EMPTY);
				setFocus(IDS_STYLE_NAME_EMPTY);
				break;
			}
			if(!IsApply()){
				if(IDYES != CGrMessageBox::Show(CGrTxtFunc::GetDllModuleHandle(), m_hWnd, IDS_NO_APPLY_MES, IDS_MES_CONF, MB_YESNO)){
					break;
				}
			}
			TCHAR styleFolderName[MAX_PATH+1];
			_stprintf_s(styleFolderName, _T("%s/Style/"), CGrTxtFunc::GetDataPath());
			CGrShell::CreateFolder(styleFolderName);
			TCHAR iniFileName[MAX_PATH+1];
			_stprintf_s(iniFileName, _T("%s/Style/%s.ini"), CGrTxtFunc::GetDataPath(), styleName);
			WIN32_FIND_DATA wfd;
			if(CGrShell::getFileInfo(iniFileName, wfd)){
				if(IDYES != CGrMessageBox::Show(CGrTxtFunc::GetDllModuleHandle(), m_hWnd, IDS_STYLE_OVERWRITE, IDS_MES_CONF, MB_YESNO)){
					break;
				}
			}
			auto &&param = CGrTxtFunc::Param();
			if(param.SaveStyle(iniFileName, styleName)){
				param.UpdateStyleList(GetParent(GetParent(m_hWnd)));
				CGrMessageBox::Show(CGrTxtFunc::GetDllModuleHandle(), m_hWnd, IDS_STYLE_SAVE_OK, IDS_MES_CONF);
			} else {
				setFocus(IDC_EDIT_SAVE_STYLE_NAME);
				CGrMessageBox::Show(CGrTxtFunc::GetDllModuleHandle(), m_hWnd, IDS_STYLE_WRITEERROR);
			}
		}
		break;
	case ID_SHOW_STYLE_FOLDER:
		{
			TCHAR styleFolderName[MAX_PATH+1];
			_stprintf_s(styleFolderName, _T("%s/Style/"), CGrTxtFunc::GetDataPath());
			CGrShell::CreateFolder(styleFolderName);
			CGrShell::Execute(m_hWnd, styleFolderName);
		}
		break;
	}
}

bool CGrPropPageStyle::Apply()
{
	auto &&param = CGrTxtFunc::Param();

	TCHAR buf[512] = {};
	int idx = 0;
	// Font Check
	{
		TCHAR fontName[MemberSizeOf(LOGFONT, lfFaceName)];
		auto e=m_fontNameList.end();
		for(idx=0; idx<sizeof(l_font_name)/sizeof(FontName); ++idx){
			auto hwnd_font = GetDlgItem(m_hWnd, l_font_name[idx].id);
			::GetWindowText(hwnd_font, fontName, sizeof(fontName)/sizeof(TCHAR));
			if(e == m_fontNameList.find(fontName)){
				CGrMessageBox::Show(CGrTxtFunc::GetDllModuleHandle(), m_hWnd, IDS_FONT_NOTFOUND);
				setFocus(hwnd_font);
				return false;
			}
		}
	}
	//
	{
		auto hwnd_layout = GetDlgItem(m_hWnd, IDC_COMBO_LAYOUT);
		idx = ComboBox_GetCurSel(hwnd_layout);
		idx = ComboBox_GetItemData(hwnd_layout, idx);
		const auto &li = m_lil[idx];
		param.SetText(CGrTxtFuncIParam::LayoutFile, li.filename.c_str());

		CGrTxtLayoutMgr mgr;
		auto *pLayout = mgr.GetTxtLayout(m_lil[idx]);
		if(pLayout){
			pLayout->Save(li.filename.c_str());
			delete pLayout;
		}
	}
	//
	{
		for(idx = 0; idx<sizeof(l_style_point_range)/sizeof(PointRange); ++idx){
			if(!::IsWindowEnabled(GetDlgItem(m_hWnd, l_style_point_range[idx].id))){
				continue;
			}
			int num = SendMessage(GetDlgItem(m_hWnd, l_style_point_range[idx].id), UDM_GETPOS, 0, 0);
			if(num < l_style_point_range[idx].min || (num > l_style_point_range[idx].max && l_style_point_range[idx].max > 0)){
				GetDlgItemText(m_hWnd, l_style_point_range[idx].id , buf, sizeof(buf)/sizeof(TCHAR));
				CGrMessageBox::Show(CGrTxtFunc::GetDllModuleHandle(), m_hWnd, l_style_point_range[idx].err_id);
				setFocus(l_style_point_range[idx].id);
				return false;
			}
		}
	}
	// 文字間隔
	// 中央揃え
	for(idx=0; idx<sizeof(l_font_spacing)/sizeof(FontSpacing); ++idx){
		const auto &fs = l_font_spacing[idx];
		GetDlgItemText(m_hWnd, fs.id_spacing , buf, sizeof(buf)/sizeof(TCHAR));
		double val = _tstof(buf);
		if(val < 0.0){
			val = 0.0;
		}
		param.SetFontSpacing(fs.type, val);
		param.SetFontCentering(fs.type, ::GetCheckDlgItemID(m_hWnd, fs.id_center) == BST_CHECKED);
	}
	// スタイル
	for(idx=0; idx<sizeof(l_font_weight)/sizeof(FontWeight); ++idx){
		const auto &fw = l_font_weight[idx];
		auto hwnd_cmb = GetDlgItem(m_hWnd, fw.id);
		long lfWeight = ComboBox_GetItemData(hwnd_cmb, ComboBox_GetCurSel(hwnd_cmb));
		param.SetFontWeight(fw.type, lfWeight);
	}
	//
	COLORREF page_color[3] = {};
	COLORREF link_color = {};
	param.SetColor(CGrTxtFuncIParam::Text        , m_colBtnText        .GetColor());
	param.SetColor(CGrTxtFuncIParam::Bold        , m_colBtnBold        .GetColor());
	param.SetColor(CGrTxtFuncIParam::Ruby        , m_colBtnRuby        .GetColor());
	param.SetColor(CGrTxtFuncIParam::Note        , m_colBtnNote        .GetColor());
	param.SetColor(CGrTxtFuncIParam::Nombre      , m_colBtnNombre      .GetColor());
	param.SetColor(CGrTxtFuncIParam::RunningHeads, m_colBtnRunningHeads.GetColor());
	page_color[0] = m_colBtnPaperBkColor      .GetColor();
	page_color[1] = m_colBtnPaperShadowBkColor.GetColor();
	page_color[2] = m_colBtnBkColor           .GetColor();
	link_color    = m_colBtnLinkColor         .GetColor();
	param.SetPoints(CGrTxtFuncIParam::PageColor    , reinterpret_cast<int*>(page_color) , sizeof(page_color)/sizeof(COLORREF));
	param.SetPoints(CGrTxtFuncIParam::LinkTextColor, reinterpret_cast<int*>(&link_color), 1                                  );
	// ページ設定 NxN
	::GetDlgItemPointSet(m_hWnd, l_style_point_set, sizeof(l_style_point_set)/sizeof(PointSet));
	//
	TCHAR fontName[MemberSizeOf(LOGFONT, lfFaceName)];
	for(idx=0; idx<sizeof(l_font_name)/sizeof(FontName); ++idx){
		GetDlgItemText(m_hWnd, l_font_name[idx].id, fontName, sizeof(fontName)/sizeof(TCHAR));
		param.SetFontName(l_font_name[idx].type, fontName);
	}
	::GetDlgItemTextType(m_hWnd, param, IDC_EDIT_BACK_IMG, CGrTxtFuncIParam::BackgroundImage);
	param.SetBoolean(CGrTxtFuncIParam::WzMemoMode     , IsDlgButtonChecked(m_hWnd, IDC_CHECKBOX_WZMEMO           ) == TRUE);
	param.SetBoolean(CGrTxtFuncIParam::UseOverlapChar , IsDlgButtonChecked(m_hWnd, IDC_CHECKBOX_OVERLAPCHAR      ) == TRUE);
	param.SetBoolean(CGrTxtFuncIParam::ImageNextLayout, IsDlgButtonChecked(m_hWnd, IDC_CHECKBOX_IMAGE_NEXT_LAYOUT) == TRUE);
	//
	for(idx=0; idx<sizeof(l_style_value_set)/sizeof(ValueSet); ++idx){
		GetDlgItemText(m_hWnd, l_style_value_set[idx].id , buf, sizeof(buf)/sizeof(TCHAR));
		param.SetTextList(l_style_value_set[idx].type, buf);
	}
	//
	param.SetBoolean(CGrTxtFuncIParam::PageFlip, IsDlgButtonChecked(m_hWnd, IDC_CHECKBOX_PAGEFLIP) == TRUE);
	// アンチエイリアス
	int iAntiAlias = 1;
	if(IsDlgButtonChecked(m_hWnd, IDC_ANTIALIAS_LCD)){
		iAntiAlias = 2;
	}
	param.SetPoints(CGrTxtFuncIParam::AntiAlias, &iAntiAlias, 1);
	// ルビ
	int iRubyPosition = 0;
	if(IsDlgButtonChecked(m_hWnd, IDC_RUBYPOSITION_CENTER)){
		iRubyPosition = 1;
	}
	param.SetPoints(CGrTxtFuncIParam::RubyPosition, &iRubyPosition, 1);
	// 白色画像透過
	param.SetBoolean(CGrTxtFuncIParam::WhiteTrans, IsDlgButtonChecked(m_hWnd, IDC_CHECKBOX_WHITETRANS) == TRUE);
	// 画像をページ一杯に表示
	param.SetBoolean(CGrTxtFuncIParam::PictPaddingNone, IsDlgButtonChecked(m_hWnd, IDC_CHECKBOX_PICTPADDINGNONE) == TRUE);
	// ページ内で最初に見つかった見出しを柱にする
	int iRunningHeadLevel[2] = {1,1};
	if(IsDlgButtonChecked(m_hWnd, IDC_CHECKBOX_RUNNINGHEAD_1ST)){
		iRunningHeadLevel[0] = 1;
	} else {
		iRunningHeadLevel[0] = 0;
	}
	// レベルnまでの見出しを柱の対象にする
	int iRunningHeadLevelFlag;
	iRunningHeadLevel[1] = ::GetDlgItemInt(m_hWnd, IDC_EDIT_RUNNINGHEAD_LEVEL, &iRunningHeadLevelFlag, true);
	param.SetPoints(CGrTxtFuncIParam::RunningHeadLevel, iRunningHeadLevel, _countof(iRunningHeadLevel));
	// Dark mode
	const TCHAR *pVal = nullptr;
	if (::GetCheckDlgItemID(m_hWnd, IDC_CHECKBOX_DARKMODE) == BST_CHECKED) {
		pVal = _T("1");
	}
	else {
		pVal = _T("0");
	}
	std::tstring ini_filename(CGrTxtFunc::GetDataPath());
	if (!CGrShell::GetSearchDir(ini_filename.c_str())) {
		if (!CGrShell::CreateFolder(ini_filename.c_str())) {
			return false;
		}
	}
	ini_filename += _T("/TxtMiru.ini");
	::WritePrivateProfileString(_T("points"), _T("EnableDarkTheme"), pVal, ini_filename.c_str());
	//
	// 青空文庫設定
	int iAozoraSetting[1] = {};
	iAozoraSetting[0] = SendMessage(GetDlgItem(m_hWnd, IDC_COMBOBOX_AOZORA_BOTTOM), CB_GETCURSEL, 0, 0);
	param.SetPoints(CGrTxtFuncIParam::AozoraSetting, iAozoraSetting, sizeof(iAozoraSetting) / sizeof(int));
	// 起動時に、Fontフォルダ以下のフォントをロードする
	param.SetBoolean(CGrTxtFuncIParam::UseFont, IsDlgButtonChecked(m_hWnd, IDC_CHECKBOX_USEFONT) == TRUE);
	//
	param.UpdateConfig(GetParent(GetParent(m_hWnd)));
	return true;
}
