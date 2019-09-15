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
		HangingCharacters      , // ぶら下げ文字
		LineStartNGCharacters  , // 行頭禁止
		LineEndNGCharacters    , // 行末禁止文字
		LineStartSkipCharacters, // 行頭スキップ文字
		SeparateNGCharacters   , // 分割禁止文字
		RotateCharacters       , // 回転させて表示する文字
		RRotateCharacters      , // 逆回転させて表示する文字
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
		LayoutFile        , // レイアウトファイル名
		LayoutType        , // レイアウトタイプ
		BookMarkFolder    , // 栞フォルダ
		NoteFormat        , // 注釈書式
		RunningHeadsFormat, // 柱書式
		BackgroundImage   , // 背景画像
		SpiPluginFolder   , // SPIプラグインフォルダ
		ProxyServer       , // プロキシサーバー
		NoProxyAddress    , // プロキシから除外
		PreParserFolder   , // PreParserファイルの在るフォルダ
		BrowserAppName    , // ブラウザから情報を取得する際のアプリケーション名一覧
		FileTypeHtml      , // HTM, HTML, MHT, MHTML, XHTML
		FileTypeArc7z     , // 7Z
		FileTypeArcCab    , // CAB
		FileTypeArcLzh    , // LZH
		FileTypeArcRar    , // RAR, R01
		FileTypeArcZip    , // ZIP
		CopyTextExe       , // コピーテキスト
		CopyTextPrm       , // プログラム名、引数
		OpenFileExe       , // 外部プログラム
		OpenFilePrm       , // プログラム名、引数
		OpenFileExe1      , // 外部プログラム
		OpenFilePrm1      , // プログラム名、引数
		OpenFileExe2      , // 外部プログラム
		OpenFilePrm2      , // プログラム名、引数
		GuessEncodingDLL  , // 文字コード判定DLL
		TitleFormat       , // タイトル書式
		OpenLinkExe       , // リンククリック時に起動する 外部プログラム
		OpenLinkPrm       , // リンククリック時に起動するプログラム名、引数
		RubyListIgnore  , // ルビ一覧への除外設定
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
		WindowSize      , // ウインドウサイズ
		TateChuNum      , // 縦中横の最大字数
		ShowHScroll     , // 水平スクロールバー表示
		ShowVScroll     , // 垂直スクロールバー表示
		SearchLoop      , // 戻って検索
		BookMarkAutoSave, // 栞自動保存
		BookMarkToFolder, // 栞ファイルは栞フォルダに保存(<=>テキストファイルと同じフォルダに作成)
		BookMarkNum     , // 自動保存で保存される最大栞数
		SaveWindowSize  , // ウインドウサイズの保存
		BookmarkPos     , // 栞ウインドウの位置
		SubtitlePos     , // 見出し一覧ウインドウの位置
		FullScreen      , // フルスクリーン
		WzMemoMode      , // 階層付きテキスト形式:http://ja.wikipedia.org/wiki/アウトラインプロセッサ
		PageFlip        , // ページめくりアニメーション
		PageFlipInterval, // ページめくりアニメーション間隔
		AntiAlias       , // アンチアリアスを行う
		SelectionMode   , // 選択モード
		SkipHTMLImgSize , // HTML表示時、無視する画像のサイズ(指定したサイズより小さい画像は表示しない：進む・戻るとかアイコンを半ページ使用させないようにするため)
		UseIESetting    , // IEの設定を使用して接続
		UseProxy        , // プロキシを使用
		LinkTextColor   , // リンクテキストカラー
		WhiteTrans      , // 画像の白色部分を透過
		WhiteTransRate  , // 画像の白色部分を透過する度合い(全体的に淡い画像だとほとんどが透明になるので)
		UsePreParser    , // Parse処理の前に、パース処理を行うかどうか [0:Pre-Parserを使用する, 1:Pre-Parserのエラーを表示しない, 2:(n)MB以上のファイルはPre-Parserを使用しない]
		ArcMaxFileSize  , // 書庫ファイルの展開最大サイズ
		LupePos         , // ルーペ
		FileAutoReload  , // ファイルの更新時、自動リロード
		UseOverlapChar  , // ゜,゛を 文字合成 処理するかどうか
		PictPaddingNone , // 画像表示時の余白をなくす(ページ一杯に表示)
		SpSelectionMode , // 選択モード以外でも一時的に選択できるように
		AutoCopyMode    , // 選択完了時、自動で選択文字をクリップボードにコピー
		AutoHideMenu    , // メニュー・スクロールバーを自動的に隠す
		LinkPos         , // 見出し一覧&栞ウインドウの位置 LinkDlg(Showed),StayOn,left,top,width,height
		RunExecCopyText , // 選択したテキストをプログラムに渡す
		CopyRuby        , // コピー時、ルビを優先
		ImageNextLayout , // 挿絵が現れた時点で次のレイアウトに
		IEOption        , // IEの設定, DLCTL_SILENT,DLCTL_NO_SCRIPTS
		UnicodeIni      , // INIファイルがユニコードで保存されているか
		RubyPosition    , // ルビ肩付、中付
		PageMode        , // ページモード
		FavoritePos     , // お気に入りウインドウ (Showed)
		TouchMenu       , // タッチ操作時、メニューの文字間隔も広く
		TopMost         , // フルスクリーン時にTOPMOSTにするか
		UseRegExp       , // 正規表現
		KeyInterval     , // 長押し判定時間
		WebFilter       , // URL Block
		RunningHeadLevel, // 柱の設定 [1:最初に見つかった見出しを柱にする],[n:nレベルnまでの見出しを柱の対象にする]
		KeyRepeat       , // キーリピートの使用有無
		AozoraSetting   , // 青空文庫設定
		UseFont         , // フォントファイルのロード
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
