#ifndef __TXTFUNCIPARAM_H__
#define __TXTFUNCIPARAM_H__

#define WM_TXTMIRU_FUNC_CALL _T("TxtMiru_FuncCall")
class CGrTxtFuncIParam
{
public:
	enum FunCallType {
		FncCT_UpdateCheck          ,
		FncCT_GetCurrentPage       ,
		FncCT_SetActiveModelessDlg ,
		FunCT_InstallHook          ,
		FunCT_UninstallHook        ,
		FunCT_AddBookmark          ,
		FunCT_DeleteBookmark       ,
		FncCT_GetCurrentDisplayPage,
		FncCT_GetSinglePage        ,
		FncCT_MaxNum              ,
	};
	enum ValueType {
		HangingCharacters      , // �Ԃ牺������
		LineStartNGCharacters  , // �s���֎~
		LineEndNGCharacters    , // �s���֎~����
		LineStartSkipCharacters, // �s���X�L�b�v����
		SeparateNGCharacters   , // �����֎~����
		RotateCharacters       , // ��]�����ĕ\�����镶��
		RRotateCharacters      , // �t��]�����ĕ\�����镶��
		VT_MaxNum              ,
	};
	enum CharType {
		Text        ,
		Ruby        ,
		Note        ,
		Nombre      ,
		RunningHeads,
		Bold        ,
		CT_MaxNum   ,
	};
	enum TextType {
		LastFolder        ,
		LastFile          ,
		HistFile1         ,
		HistFile2         ,
		HistFile3         ,
		HistFile4         ,
		HistFile5         ,
		HistFile6         ,
		HistFile7         ,
		HistFile8         ,
		HistFile9         ,
		HistFileTitle1    ,
		HistFileTitle2    ,
		HistFileTitle3    ,
		HistFileTitle4    ,
		HistFileTitle5    ,
		HistFileTitle6    ,
		HistFileTitle7    ,
		HistFileTitle8    ,
		HistFileTitle9    ,
		HistSearchWord1   ,
		HistSearchWord2   ,
		HistSearchWord3   ,
		HistSearchWord4   ,
		HistSearchWord5   ,
		HistSearchWord6   ,
		HistSearchWord7   ,
		HistSearchWord8   ,
		HistSearchWord9   ,
		LayoutFile        , // ���C�A�E�g�t�@�C����
		LayoutType        , // ���C�A�E�g�^�C�v
		BookMarkFolder    , // �x�t�H���_
		NoteFormat        , // ���ߏ���
		RunningHeadsFormat, // ������
		BackgroundImage   , // �w�i�摜
		SpiPluginFolder   , // SPI�v���O�C���t�H���_
		ProxyServer       , // �v���L�V�T�[�o�[
		NoProxyAddress    , // �v���L�V���珜�O
		PreParserFolder   , // PreParser�t�@�C���݂̍�t�H���_
		BrowserAppName    , // �u���E�U��������擾����ۂ̃A�v���P�[�V�������ꗗ
		FileTypeHtml      , // HTM, HTML, MHT, MHTML, XHTML
		FileTypeArc7z     , // 7Z
		FileTypeArcCab    , // CAB
		FileTypeArcLzh    , // LZH
		FileTypeArcRar    , // RAR, R01
		FileTypeArcZip    , // ZIP
		CopyTextExe       , // �R�s�[�e�L�X�g
		CopyTextPrm       , // �v���O�������A����
		OpenFileExe       , // �O���v���O����
		OpenFilePrm       , // �v���O�������A����
		OpenFileExe1      , // �O���v���O����
		OpenFilePrm1      , // �v���O�������A����
		OpenFileExe2      , // �O���v���O����
		OpenFilePrm2      , // �v���O�������A����
		GuessEncodingDLL  , // �����R�[�h����DLL
		TitleFormat       , // �^�C�g������
		OpenLinkExe       , // �����N�N���b�N���ɋN������ �O���v���O����
		OpenLinkPrm       , // �����N�N���b�N���ɋN������v���O�������A����
		RubyListIgnore  , // ���r�ꗗ�ւ̏��O�ݒ�
		TT_MaxNum         ,
	};
	enum FileType {
		FT_Html  , // HTM, HTML, MHT, MHTML, XHTML
		FT_Text  , // TXT
		FT_Link  , // LNK
		FT_Siori , // SIORI
		FT_Arc7z , // 7Z
		FT_ArcCab, // CAB
		FT_ArcLzh, // LZH
		FT_ArcRar, // RAR, R01
		FT_ArcZip, // ZIP
		FT_MaxNum, //
	};
	enum AntiAliasType {
		AA_None   ,
		AA_Normal ,
		AA_LCD    ,
		AA_MaxNum
	};
	enum PointsType {
		PageColor       ,
		WindowSize      , // �E�C���h�E�T�C�Y
		TateChuNum      , // �c�����̍ő厚��
		ShowHScroll     , // �����X�N���[���o�[�\��
		ShowVScroll     , // �����X�N���[���o�[�\��
		SearchLoop      , // �߂��Č���
		BookMarkAutoSave, // �x�����ۑ�
		BookMarkToFolder, // �x�t�@�C���͞x�t�H���_�ɕۑ�(<=>�e�L�X�g�t�@�C���Ɠ����t�H���_�ɍ쐬)
		BookMarkNum     , // �����ۑ��ŕۑ������ő�x��
		SaveWindowSize  , // �E�C���h�E�T�C�Y�̕ۑ�
		BookmarkPos     , // �x�E�C���h�E�̈ʒu
		SubtitlePos     , // ���o���ꗗ�E�C���h�E�̈ʒu
		FullScreen      , // �t���X�N���[��
		WzMemoMode      , // �K�w�t���e�L�X�g�`��:http://ja.wikipedia.org/wiki/�A�E�g���C���v���Z�b�T
		PageFlip        , // �y�[�W�߂���A�j���[�V����
		PageFlipInterval, // �y�[�W�߂���A�j���[�V�����Ԋu
		AntiAlias       , // �A���`�A���A�X���s��
		SelectionMode   , // �I�����[�h
		SkipHTMLImgSize , // HTML�\�����A��������摜�̃T�C�Y(�w�肵���T�C�Y��菬�����摜�͕\�����Ȃ��F�i�ށE�߂�Ƃ��A�C�R���𔼃y�[�W�g�p�����Ȃ��悤�ɂ��邽��)
		UseIESetting    , // IE�̐ݒ���g�p���Đڑ�
		UseProxy        , // �v���L�V���g�p
		LinkTextColor   , // �����N�e�L�X�g�J���[
		WhiteTrans      , // �摜�̔��F�����𓧉�
		WhiteTransRate  , // �摜�̔��F�����𓧉߂���x����(�S�̓I�ɒW���摜���ƂقƂ�ǂ������ɂȂ�̂�)
		UsePreParser    , // Parse�����̑O�ɁA�p�[�X�������s�����ǂ��� [0:Pre-Parser���g�p����, 1:Pre-Parser�̃G���[��\�����Ȃ�, 2:(n)MB�ȏ�̃t�@�C����Pre-Parser���g�p���Ȃ�]
		ArcMaxFileSize  , // ���Ƀt�@�C���̓W�J�ő�T�C�Y
		LupePos         , // ���[�y
		FileAutoReload  , // �t�@�C���̍X�V���A���������[�h
		UseOverlapChar  , // �K,�J�� �������� �������邩�ǂ���
		PictPaddingNone , // �摜�\�����̗]�����Ȃ���(�y�[�W��t�ɕ\��)
		SpSelectionMode , // �I�����[�h�ȊO�ł��ꎞ�I�ɑI���ł���悤��
		AutoCopyMode    , // �I���������A�����őI�𕶎����N���b�v�{�[�h�ɃR�s�[
		AutoHideMenu    , // ���j���[�E�X�N���[���o�[�������I�ɉB��
		LinkPos         , // ���o���ꗗ&�x�E�C���h�E�̈ʒu LinkDlg(Showed),StayOn,left,top,width,height
		RunExecCopyText , // �I�������e�L�X�g���v���O�����ɓn��
		CopyRuby        , // �R�s�[���A���r��D��
		ImageNextLayout , // �}�G�����ꂽ���_�Ŏ��̃��C�A�E�g��
		IEOption        , // IE�̐ݒ�, DLCTL_SILENT,DLCTL_NO_SCRIPTS
		UnicodeIni      , // INI�t�@�C�������j�R�[�h�ŕۑ�����Ă��邩
		RubyPosition    , // ���r���t�A���t
		PageMode        , // �y�[�W���[�h
		FavoritePos     , // ���C�ɓ���E�C���h�E (Showed)
		TouchMenu       , // �^�b�`���쎞�A���j���[�̕����Ԋu���L��
		TopMost         , // �t���X�N���[������TOPMOST�ɂ��邩
		UseRegExp       , // ���K�\��
		KeyInterval     , // ���������莞��
		WebFilter       , // URL Block
		RunningHeadLevel, // ���̐ݒ� [1:�ŏ��Ɍ����������o���𒌂ɂ���],[n:n���x��n�܂ł̌��o���𒌂̑Ώۂɂ���]
		KeyRepeat       , // �L�[���s�[�g�̎g�p�L��
		AozoraSetting   , // �󕶌ɐݒ�
		UseFont         , // �t�H���g�t�@�C���̃��[�h
		PT_MaxNum       ,
	};
public:
	CGrTxtFuncIParam(){};
	virtual ~CGrTxtFuncIParam(){};

	virtual void SetFontName(CharType type, LPCTSTR fontName) = 0;
	virtual void SetColor(CharType type, COLORREF color) = 0;
	virtual void SetFontSpacing(CharType type, double fontSpacing) = 0;
	virtual void SetFontCentering(CharType type, bool fontCentering) = 0;
	virtual void SetFontWeight(CharType type, long weight) = 0;
	virtual void SetBoolean(PointsType type, bool val) = 0;
	virtual void SetPoints(PointsType type, const int points[], int size) = 0;
	virtual void SetTextList(ValueType type, LPCTSTR lpStr) = 0;
	virtual void SetText(TextType type, LPCTSTR lpStr) = 0;

	virtual LPCTSTR GetFontName(CharType type) = 0;
	virtual COLORREF GetColor(CharType type) = 0;
	virtual double GetFontSpacing(CharType type) = 0;
	virtual bool GetFontCentering(CharType type) = 0;
	virtual long GetFontWeight(CharType type) = 0;
	virtual bool GetBoolean(PointsType type) const = 0;
	virtual int GetPoints(PointsType type, int points[], int size) const = 0;
	virtual void GetTextList(ValueType type, LPTSTR outstr, int numberOfElements) = 0;
	virtual void GetText(TextType type, LPTSTR outstr, int numberOfElements) const = 0;

	virtual POINT GetCharOffset(LPCTSTR str) const = 0;
	virtual WORD GetCharOffsetType(LPCTSTR str) const = 0;

	virtual void UpdateKeybord(HWND hWnd) = 0;
	virtual void UpdateConfig(HWND hWnd) = 0;
	virtual void UpdateLayout(HWND hWNd) = 0;
	virtual void UpdateStyleList(HWND hWnd) = 0;
	virtual bool SaveStyle(LPCTSTR lpFileName, LPCTSTR lpStyleName) = 0;
};
#endif // __TXTFUNCIPARAM_H__
